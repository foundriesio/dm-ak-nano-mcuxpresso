/*
 * FreeRTOS V202107.00
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file ota_demo_core_mqtt.c
 * @brief OTA update example using coreMQTT.
 */

/* Standard includes. */

#define LIBRARY_LOG_LEVEL LOG_INFO
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "fsl_flexspi.h"

#include "aknano_priv.h"

#include "aws_demo_config.h"
#include "aws_dev_mode_key_provisioning.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include "core_json.h"

#include "iot_network.h"

/* Retry utilities include. */
#include "backoff_algorithm.h"

/* Include PKCS11 helper for random number generation. */
#include "pkcs11_helpers.h"

/*Include backoff algorithm header for retry logic.*/
#include "backoff_algorithm.h"

/* Transport interface include. */
#include "transport_interface.h"

/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

/* Include header for connection configurations. */
#include "aws_clientcredential.h"

/* Include header for client credentials. */
#include "aws_clientcredential_keys.h"

/* Include header for root CA certificates. */
#include "iot_default_root_certificates.h"

/* OTA Library include. */
#include "ota.h"

/* OTA library and demo configuration macros. */
#include "ota_config.h"
#include "ota_demo_config.h"

/* OTA Library Interface include. */
#include "ota_os_freertos.h"
#include "ota_mqtt_interface.h"

/* PAL abstraction layer APIs. */
#include "ota_pal.h"

/* Includes the OTA Application version number. */
#include "ota_appversion32.h"


#include "core_http_client.h"

/*------------- Demo configurations -------------------------*/

/** Note: The device client certificate and private key credentials are
 * obtained by the transport interface implementation (with Secure Sockets)
 * from the demos/include/aws_clientcredential_keys.h file.
 *
 * The following macros SHOULD be defined for this demo which uses both server
 * and client authentications for TLS session:
 *   - keyCLIENT_CERTIFICATE_PEM for client certificate.
 *   - keyCLIENT_PRIVATE_KEY_PEM for client private key.
 */


// #define AKNANO_DRY_RUN

/**
 * @brief Number of milliseconds in a second.
 */
#define NUM_MILLISECONDS_IN_SECOND                  ( 1000U )

/**
 * @brief Milliseconds per second.
 */
#define MILLISECONDS_PER_SECOND                     ( 1000U )

/**
 * @brief Milliseconds per FreeRTOS tick.
 */
#define MILLISECONDS_PER_TICK                       ( MILLISECONDS_PER_SECOND / configTICK_RATE_HZ )

/**
 * @brief Each compilation unit that consumes the NetworkContext must define it.
 * It should contain a single pointer to the type of your desired transport.
 * When using multiple transports in the same compilation unit, define this
 * pointer as void *.
 *
 * @note Transport stacks are defined in amazon-freertos/libraries/abstractions/transport/secure_sockets/transport_secure_sockets.h.
 */
struct NetworkContext
{
    SecureSocketsTransportParams_t * pParams;
};

/**
 * @brief Global entry time into the application to use as a reference timestamp
 * in the #prvGetTimeMs function. #prvGetTimeMs will always return the difference
 * between the current time and the global entry time. This will reduce the
 * chances of overflow for the 32 bit unsigned integer used for holding the
 * timestamp.
 */
static uint32_t ulGlobalEntryTimeMs;

static struct aknano_settings xaknano_settings;
static struct aknano_context xaknano_context;


/*-----------------------------------------------------------*/

static uint32_t prvGetTimeMs( void )
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = ( uint32_t ) xTickCount * MILLISECONDS_PER_TICK;

    /* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
     * elapsed time in the application. */
    ulTimeMs = ( uint32_t ) ( ulTimeMs - ulGlobalEntryTimeMs );

    return ulTimeMs;
}
/*-----------------------------------------------------------*/


#define AKNANO_DOWNLOAD_ENDPOINT "detsch.gobolinux.org"

#define AKNANO_DOWNLOAD_ENDPOINT_LEN sizeof(AKNANO_DOWNLOAD_ENDPOINT)-1


#define AKNANO_REQUEST_BODY ""
#define AKNANO_REQUEST_BODY_LEN sizeof(AKNANO_REQUEST_BODY)-1

static const char donwloadServer_ROOT_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIGTTCCBTWgAwIBAgISAxxpPJ3hYiEHaWa8YuciZ+JoMA0GCSqGSIb3DQEBCwUA\n"
"MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD\n"
"EwJSMzAeFw0yMTEwMDcyMzM4NDRaFw0yMjAxMDUyMzM4NDNaMCMxITAfBgNVBAMT\n"
"GHd3dy5kZXRzY2guZ29ib2xpbnV4Lm9yZzCCAiIwDQYJKoZIhvcNAQEBBQADggIP\n"
"ADCCAgoCggIBAKY7IOPkXTmAy/aB5LQFaD6JuA0ZcWzEiG5pXeIjNAGCBPJI2fZm\n"
"nl6sTxmZBzuSPm1iSUoTK7KazPheQLoi1p36iaZR7loFstwuJ+WqG1PMGRaClGD7\n"
"4REJzWSJAgEeUATyxFd0fJ0orJ6ksrWMhGhg0NCbG0pefAEERTIhvudAf/oI/Qxz\n"
"5X9yhO0pwS0dEr3u7bxSpElj1S9WGb2zVOASHlhZ8IttS8oUphfiS3CHknkLNp42\n"
"qVlUX6W/Xi5b3sH7rK/2k9KPg9TKV0kunsAFuSkSHwu7/m+QDgxIRIjdfZ6BftU/\n"
"TkXalQN62hpbQbCnwnHVBAvWgviNsvCrqE2DuQQ9EWT/rVrkYy9zyiJgCU0ibEhg\n"
"H7O+LZ+7JYXq94LPkyaYuwE0xM/p3ilK8jpELQv3howeftPPYMefZX1VSUVXEVP/\n"
"aZf6zQpqvUwpg55ejOrOY+VRMIu7ofQfo8xflDi0bUYCYCkljy5LMvlnOAGUU9SE\n"
"cIcPPssjH3PD8jpo9B9kLs91ni37RjhWi8L7XSwIlB6t80Qmje/hMBsqH+ZwFDzg\n"
"8ZX3lcOfCOSffjOuCPDtUejKFGiUuKL+NGnAh/tZrfIKKyP77l0g3UCv39HQeW0u\n"
"DVqq+94eSkpKGs+/4HNUmH3jzcFYCuuIi0HejMZU9u5O50Dn++x6IP3xAgMBAAGj\n"
"ggJqMIICZjAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsG\n"
"AQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFHBelb0QexAthfmoMoG2nShK\n"
"zQasMB8GA1UdIwQYMBaAFBQusxe3WFbLrlAJQOYfr52LFMLGMFUGCCsGAQUFBwEB\n"
"BEkwRzAhBggrBgEFBQcwAYYVaHR0cDovL3IzLm8ubGVuY3Iub3JnMCIGCCsGAQUF\n"
"BzAChhZodHRwOi8vcjMuaS5sZW5jci5vcmcvMDkGA1UdEQQyMDCCFGRldHNjaC5n\n"
"b2JvbGludXgub3Jnghh3d3cuZGV0c2NoLmdvYm9saW51eC5vcmcwTAYDVR0gBEUw\n"
"QzAIBgZngQwBAgEwNwYLKwYBBAGC3xMBAQEwKDAmBggrBgEFBQcCARYaaHR0cDov\n"
"L2Nwcy5sZXRzZW5jcnlwdC5vcmcwggEFBgorBgEEAdZ5AgQCBIH2BIHzAPEAdgDf\n"
"pV6raIJPH2yt7rhfTj5a6s2iEqRqXo47EsAgRFwqcwAAAXxdVoi/AAAEAwBHMEUC\n"
"IE8gukdNsmpXBKRF/9EO9wql+BMfvww94dxDfsMfGguYAiEA2KpontE0v/F61hK4\n"
"OgFDPjKrMvj8Ka291qog4TmEOVAAdwBGpVXrdfqRIDC1oolp9PN9ESxBdL79SbiF\n"
"q/L8cP5tRwAAAXxdVo0/AAAEAwBIMEYCIQCeGOPmQtiOoTSXnuC7grRkCg1eVLl7\n"
"vIrMjnVWcvo5nQIhAJ7QlXKRxqEqOy29pHZqkbn5WtLUAWhTGw3yLesN62GpMA0G\n"
"CSqGSIb3DQEBCwUAA4IBAQCi4Zfg+lSFJhllh/Nkx+wNS9rYbQLESqNBSjhLJ2Df\n"
"fvhtDc1HXc1wA0HYs7IZIzqWF3ef8HNt1IHJDSqcUa3fbImjHpvqEC9sJtNhycSM\n"
"00ZeRt3Ch7j/DWQ5bF3Yljn7MT6+/mzQrPHMg143HHnEPKz73RaBYc5WvVQvGGKY\n"
"I+PgjQ8KtywhIObO1+ZZ6cHvwKw8O3EjdebqgYuK6ZLyUEsr/8HWtdbSlJR2F7Lb\n"
"e9y3CX4H860sk6ggvyYz1kpNqp2hKIRACi74lnuGGvFSE8QZVi8BNd4BvOZt37Jx\n"
"Ji/ObIgZ07mAY70w3qc2bULoTHuxwPsIRFu4AzISK/o3\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n"
"WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
"RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
"AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n"
"R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n"
"sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n"
"NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n"
"Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n"
"/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n"
"Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n"
"FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n"
"AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n"
"Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n"
"gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n"
"PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n"
"ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n"
"CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n"
"lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n"
"avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n"
"yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n"
"yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n"
"hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n"
"HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n"
"MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n"
"nLRbwHOoq7hHwg==\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";

static const uint32_t donwloadServer_ROOT_CERTIFICATE_LENGTH = sizeof( donwloadServer_ROOT_CERTIFICATE_PEM );

static BaseType_t prvConnectToDownloadServer( NetworkContext_t * pxNetworkContext )
{
    ServerInfo_t xServerInfo = { 0 };
    SocketsConfig_t xSocketsConfig = { 0 };
    BaseType_t xStatus = pdPASS;
    TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_SUCCESS;

    /* Initializer server information. */
    xServerInfo.pHostName = AKNANO_DOWNLOAD_ENDPOINT;
    xServerInfo.hostNameLength = AKNANO_DOWNLOAD_ENDPOINT_LEN;
    xServerInfo.port = 443;

    /* Configure credentials for TLS mutual authenticated session. */
    xSocketsConfig.enableTls = true;
    xSocketsConfig.pAlpnProtos = NULL;
    xSocketsConfig.maxFragmentLength = 0;
    xSocketsConfig.disableSni = false;
    xSocketsConfig.pRootCa = donwloadServer_ROOT_CERTIFICATE_PEM;
    xSocketsConfig.rootCaSize = donwloadServer_ROOT_CERTIFICATE_LENGTH;
    xSocketsConfig.sendTimeoutMs = 1000;
    xSocketsConfig.recvTimeoutMs = 1000;

    /* Establish a TLS session with the HTTP server. This example connects to
     * the HTTP server as specified in democonfigAWS_IOT_ENDPOINT and
     * democonfigAWS_HTTP_PORT in http_demo_mutual_auth_config.h. */
#if 0
    LogInfo( ( "Establishing a TLS session to %.*s:%d.",
               ( int32_t ) xServerInfo.hostNameLength,
               xServerInfo.pHostName,
               xServerInfo.port ) );
#else
    LogInfo( ( "Establishing a TLS session to binaries download server port %d.",
               xServerInfo.port ) );
#endif

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = SecureSocketsTransport_Connect( pxNetworkContext,
                                                     &xServerInfo,
                                                     &xSocketsConfig );

    LogInfo(("prvConnectToDownloadServer result=%d", xNetworkStatus));
    if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}

/**
 * @brief The length of the HTTP GET method.
 */
#define httpexampleHTTP_METHOD_GET_LENGTH                    ( sizeof( HTTP_METHOD_GET ) - 1 )

/**
 * @brief Field name of the HTTP range header to read from server response.
 */
#define httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD           "Content-Range"

/**
 * @brief Length of the HTTP range header field.
 */
#define httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD_LENGTH    ( sizeof( httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD ) - 1 )

/**
 * @brief The HTTP status code returned for partial content.
 */
#define httpexampleHTTP_STATUS_CODE_PARTIAL_CONTENT          206


#define democonfigRANGE_REQUEST_LENGTH  4 * 1024 * 30
#define democonfigUSER_BUFFER_LENGTH democonfigRANGE_REQUEST_LENGTH + 1024

/**
 * @brief A buffer used in the demo for storing HTTP request headers and
 * HTTP response headers and body.
 *
 * @note This demo shows how the same buffer can be re-used for storing the HTTP
 * response after the HTTP request is sent out. However, the user can also
 * decide to use separate buffers for storing the HTTP request and response.
 */
static uint8_t ucUserBuffer[ democonfigUSER_BUFFER_LENGTH ];


BaseType_t GetFileSize(size_t* pxFileSize, HTTPResponse_t *xResponse)
{
    
    /* The location of the file size in pcContentRangeValStr. */
    char * pcFileSizeStr = NULL;

    /* String to store the Content-Range header value. */
    char * pcContentRangeValStr = NULL;
    size_t xContentRangeValStrLength = 0;

    HTTPStatus_t xHTTPStatus = HTTPClient_ReadHeader( xResponse,
                                             ( char * ) httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD,
                                             ( size_t ) httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD_LENGTH,
                                             ( const char ** ) &pcContentRangeValStr,
                                             &xContentRangeValStrLength );

    if( xHTTPStatus != HTTPSuccess )
    {
        LogError( ( "Failed to read Content-Range header from HTTP response: Error=%s.",
                    HTTPClient_strerror( xHTTPStatus ) ) );
        return pdFAIL;
    }

    /* Parse the Content-Range header value to get the file size. */
    pcFileSizeStr = strstr( pcContentRangeValStr, "/" );

    if( pcFileSizeStr == NULL )
    {
        LogError( ( "'/' not present in Content-Range header value: %s.",
                    pcContentRangeValStr ) );
        return pdFAIL;
    }

    pcFileSizeStr += sizeof( char );
    *pxFileSize = ( size_t ) strtoul( pcFileSizeStr, NULL, 10 );

    if( ( *pxFileSize == 0 ) || ( *pxFileSize == UINT32_MAX ) )
    {
        LogError( ( "Error using strtoul to get the file size from %s: xFileSize=%d.",
                    pcFileSizeStr, ( int32_t ) *pxFileSize ) );
        return pdFAIL;
    }

    LogInfo( ( "The file is %d bytes long.", ( int32_t ) *pxFileSize ) );
    return pdPASS;
}

#include <time.h>
static BaseType_t osfGetTime() {
    static BaseType_t t;
    //  = clock() * 1000 / CLOCKS_PER_SEC;
    t++;
    // LogInfo(( "osfGetTime %d", t) );
    return t;
}
#include "mcuboot_app_support.h"
#include "mflash_common.h"
#include "mflash_drv.h"


static int HandleReceivedData(const unsigned char* data, int offset, int dataLen, uint32_t partition_log_addr, uint32_t partition_size)
{
    LogInfo(("** Handling received data offset=%d dataLen=%d  [ 0x%02x 0x%02x 0x%02x 0x%02x ] ...", offset, dataLen, data[0], data[1], data[2], data[3]));
#ifdef AKNANO_DRY_RUN
    LogInfo(("** Dry run mode, skipping flash operations"));
    return 0;
#endif
    int32_t retval = 0;

    uint32_t partition_phys_addr;

    uint32_t chunk_flash_addr;
    int32_t chunk_len;

    uint32_t next_erase_addr;

    /* Page buffers */
    uint32_t page_size;
    // uint32_t *page_buffer;

    /* Received/processed data counter */
    uint32_t total_processed = 0;

    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;

    unsigned char page_buffer[MFLASH_PAGE_SIZE];

    page_size   = MFLASH_PAGE_SIZE;

    /* Obtain physical address of FLASH to perform operations with */
    partition_phys_addr = mflash_drv_log2phys((void *)partition_log_addr, partition_size);
    if (partition_phys_addr == MFLASH_INVALID_ADDRESS)
        return -1;


    if (!mflash_drv_is_sector_aligned(partition_phys_addr) || !mflash_drv_is_sector_aligned(partition_size))
    {
        LogError(("store_update_image: partition not aligned"));
        return -1;
    }

    /* Pre-set address of area not erased so far */
    next_erase_addr  = partition_phys_addr + offset;
    chunk_flash_addr = partition_phys_addr + offset;
    total_processed  = 0;

    do
    {
        /* The data is epxected for be received by page sized chunks (except for the last one) */
        int remaining_bytes =  dataLen - total_processed;
        if (remaining_bytes < MFLASH_PAGE_SIZE)
            chunk_len = remaining_bytes;
        else
            chunk_len = MFLASH_PAGE_SIZE;
        
        memcpy(page_buffer, data + total_processed, chunk_len);

        if (chunk_len > 0)
        {
            if (chunk_flash_addr >= partition_phys_addr + partition_size)
            {
                /* Partition boundary exceeded */
                LogError(("store_update_image: partition boundary exceedded"));
                retval = -1;
                break;
            }

            /* Perform erase when encountering next sector */
            if (chunk_flash_addr >= next_erase_addr)
            {
                mflash_result = mflash_drv_sector_erase(next_erase_addr);
                if (mflash_result != 0)
                {
                    LogError(("store_update_image: Error erasing sector %d", mflash_result));
                    break;
                }
                next_erase_addr += MFLASH_SECTOR_SIZE;
            }

            /* Clear the unused portion of the buffer (applicable to the last chunk) */
            if (chunk_len < page_size)
            {
                memset((uint8_t *)page_buffer + chunk_len, 0xff, page_size - chunk_len);
            }

            /* Program the page */
            mflash_result = mflash_drv_page_program(chunk_flash_addr, page_buffer);
            if (mflash_result != 0)
            {
                LogError(("store_update_image: Error storing page %d", mflash_result));
                break;
            }

            total_processed += chunk_len;
            chunk_flash_addr += chunk_len;

            // LogInfo(("store_update_image: processed %i bytes", total_processed));
        }

    } while (chunk_len == page_size);
    LogInfo(("** Handled received data offset=%d dataLen=%d total_processed=%d", offset, dataLen, total_processed));
    return 0;
}


static BaseType_t prvDownloadFile(const NetworkContext_t *pxNetworkContext,
                                 const TransportInterface_t * pxTransportInterface,
                                           const char * pcPath )
{
    /* Return value of this method. */
    BaseType_t xStatus = pdFAIL;
    HTTPStatus_t xHTTPStatus = HTTPSuccess;
    int retriesLimit = 20;

    /* Configurations of the initial request headers that are passed to
    * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo;
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t xResponse;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders;

    /* The size of the file we are trying to download . */
    size_t xFileSize = 0;
    size_t pxFileSize;

    /* The number of bytes we want to request with in each range of the file
     * bytes. */
    size_t xNumReqBytes = 0;
    /* xCurByte indicates which starting byte we want to download next. */
    size_t xCurByte = 0;

    configASSERT( pcPath != NULL );


    /* Initialize all HTTP Client library API structs to 0. */
    ( void ) memset( &xRequestHeaders, 0, sizeof( xRequestHeaders ) );
    ( void ) memset( &xRequestInfo, 0, sizeof( xRequestInfo ) );
    ( void ) memset( &xResponse, 0, sizeof( xResponse ) );

    xResponse.getTime = osfGetTime; //nondet_boot();// ? NULL : GetCurrentTimeStub;

    /* Initialize the request object. */
    xRequestInfo.pHost = AKNANO_DOWNLOAD_ENDPOINT;
    xRequestInfo.hostLen = AKNANO_DOWNLOAD_ENDPOINT_LEN;
    xRequestInfo.pMethod = HTTP_METHOD_GET;
    xRequestInfo.methodLen = httpexampleHTTP_METHOD_GET_LENGTH;
    xRequestInfo.pPath = pcPath;
    xRequestInfo.pathLen = strlen( pcPath );

    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
     * can be sent over the same established TCP connection. This is done in
     * order to download the file in parts. */
    // Current server closes the connection anyway, and reports an error. So just disable the line bellow
    // xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

    /* Set the buffer used for storing request headers. */
    xRequestHeaders.pBuffer = ucUserBuffer;
    xRequestHeaders.bufferLen = democonfigUSER_BUFFER_LENGTH;

    /* Initialize the response object. The same buffer used for storing request
     * headers is reused here. */
    xResponse.pBuffer = ucUserBuffer;
    xResponse.bufferLen = democonfigUSER_BUFFER_LENGTH;

    // /* Verify the file exists by retrieving the file size. */
    #define FILE_SIZE_UNSET 999999999
    xFileSize = FILE_SIZE_UNSET;
    /* Set the number of bytes to request in each iteration, defined by the user
     * in democonfigRANGE_REQUEST_LENGTH. */
    // if( xFileSize < democonfigRANGE_REQUEST_LENGTH )
    // {
    //     xNumReqBytes = xFileSize;
    // }
    // else
    // {
        xNumReqBytes = democonfigRANGE_REQUEST_LENGTH;
    // }
    xStatus = pdPASS;



    partition_t update_partition;
    int32_t stored = 0;
    if (bl_get_update_partition_info(&update_partition) != kStatus_Success)
    {
        /* Could not get update partition info */
        LogError(("Could not get update partition info"));
        return pdFAIL;
    }
    

    /* Here we iterate sending byte range requests until the full file has been
     * downloaded. We keep track of the next byte to download with xCurByte, and
     * increment by xNumReqBytes after each iteration. When xCurByte reaches
     * xFileSize, we stop downloading. */
    while( ( xStatus == pdPASS ) && ( xHTTPStatus == HTTPSuccess ) && ( xCurByte < xFileSize ) )
    {

        xHTTPStatus = HTTPClient_InitializeRequestHeaders( &xRequestHeaders,
                                                           &xRequestInfo );

        if( xHTTPStatus == HTTPSuccess )
        {
            xHTTPStatus = HTTPClient_AddRangeHeader( &xRequestHeaders,
                                                     xCurByte,
                                                     xCurByte + xNumReqBytes - 1 );
        }
        else
        {
            LogError( ( "Failed to initialize HTTP request headers: Error=%s.",
                        HTTPClient_strerror( xHTTPStatus ) ) );
        }

        if( xHTTPStatus == HTTPSuccess )
        {
            LogInfo( ( "Downloading bytes %d-%d, out of %d total bytes, from %s...:  ",
                       ( int32_t ) ( xCurByte ),
                       ( int32_t ) ( xCurByte + xNumReqBytes - 1 ),
                       ( int32_t ) xFileSize == FILE_SIZE_UNSET? -1 : xFileSize,
                       "binary download server" ) );
            LogDebug( ( "Request Headers:\n%.*s",
                        ( int32_t ) xRequestHeaders.headersLen,
                        ( char * ) xRequestHeaders.pBuffer ) );
            xHTTPStatus = HTTPClient_Send( pxTransportInterface,
                                           &xRequestHeaders,
                                           NULL,
                                           0,
                                           &xResponse,
                                           0 );
        }
        else
        {
            LogError( ( "Failed to add Range header to request headers: Error=%s.",
                        HTTPClient_strerror( xHTTPStatus ) ) );
        }

        if( xHTTPStatus == HTTPSuccess )
        {
            if (xFileSize == FILE_SIZE_UNSET && GetFileSize(&pxFileSize, &xResponse) == pdPASS) {
                xFileSize = pxFileSize;
                LogInfo( ( "Setting xFileSize=%d", xFileSize));
            }

            LogInfo( ( "Received HTTP response from %s %s...",
                        "binary download server", pcPath ) );
            LogDebug( ( "Response Headers:\n%.*s",
                        ( int32_t ) xResponse.headersLen,
                        xResponse.pHeaders ) );
            LogInfo( ( "Response Body Len: %d\n",
                       ( int32_t ) xResponse.bodyLen) );


            HandleReceivedData(xResponse.pBody, xCurByte, xResponse.contentLength, update_partition.start, update_partition.size);
            stored += xResponse.contentLength;

            /* We increment by the content length because the server may not
             * have sent us the range we requested. */
            xCurByte += xResponse.contentLength;

            if( ( xFileSize - xCurByte ) < xNumReqBytes )
            {
                xNumReqBytes = xFileSize - xCurByte;
            }

            xStatus = ( xResponse.statusCode == httpexampleHTTP_STATUS_CODE_PARTIAL_CONTENT ) ? pdPASS : pdFAIL;
        }
        else
        {
#if 0
            // It is normal to get in here, because of the current server sinalization that the connection will be closed
            LogError( ( "An error occurred in downloading the file. "
                        "Failed to send HTTP GET request to %s %s: Error=%s.",
                        "binary download server", pcPath, HTTPClient_strerror( xHTTPStatus ) ) );
#endif
        }

            // Force re-connection
            if (xHTTPStatus == HTTPNetworkError && retriesLimit-- > 0) {
                // LogInfo(("Disconnecting retriesLimit=%d", retriesLimit));
                SecureSocketsTransport_Disconnect( pxNetworkContext );
                LogInfo(("Reconnecting"));
                vTaskDelay(50 / portTICK_PERIOD_MS);
                BaseType_t ret;
                ret = prvConnectToDownloadServer(pxNetworkContext);
                LogInfo(("Reconnect result = %d", ret));
                if (ret == pdPASS)
                    xHTTPStatus = HTTPSuccess;
            }

        if( xStatus != pdPASS )
        {
            LogError( ( "Received an invalid response from the server "
                        "(Status Code: %u).",
                        xResponse.statusCode ) );
        }
    }

    if (( xStatus == pdPASS ) && ( xHTTPStatus == HTTPSuccess )) {
#ifndef AKNANO_DRY_RUN
         LogInfo(("Validating image of size %d", stored));

             struct image_header *ih;
            struct image_tlv_info *it;
            uint32_t decl_size;
            uint32_t tlv_size;

            ih = (struct image_header *)update_partition.start;


         if (bl_verify_image((void *)update_partition.start, stored) <= 0)
         {
             /* Image validation failed */
             LogError(("Image validation failed magic=0x%X", ih->ih_magic));
             return false;
            // return true;
         } else {
            LogInfo(("Image validation succeded"));
            return true;
         }
#endif
    } else {
        return false;
    }
}

int AkNanoDownloadAndFlashImage(struct aknano_context *aknano_context)
{
    /* The transport layer interface used by the HTTP Client library. */
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    TransportSocketStatus_t xNetworkStatus;
    BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };

    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToDownloadServer(&xNetworkContext);
    // xDemoStatus = connectToServerWithBackoffRetries( prvConnectToServer,
    //                                                     &xNetworkContext );


    LogInfo(("AkNanoDownloadAndFlashImage: prvConnectToServer Result: %d", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        /* Set a flag indicating that a TLS connection exists. */
        xIsConnectionEstablished = pdTRUE;

        /* Define the transport interface. */
        xTransportInterface.pNetworkContext = &xNetworkContext;
        xTransportInterface.send = SecureSocketsTransport_Send;
        xTransportInterface.recv = SecureSocketsTransport_Recv;
    }
    else
    {
        /* Log error to indicate connection failure after all
            * reconnect attempts are over. */
        LogError( ( "Failed to connect to HTTP server") );
    }

    if( xDemoStatus == pdPASS )
    {
        char *relative_path = aknano_context->aknano_json_data.selected_target.uri + strlen("https://") + strlen(AKNANO_DOWNLOAD_ENDPOINT);
        LogInfo(("Download relativepath=%s", relative_path));
        xDemoStatus = prvDownloadFile(&xNetworkContext, &xTransportInterface, 
            relative_path);
    }

    /**************************** Disconnect. ******************************/

    /* Close the network connection to clean up any system resources that the
        * demo may have consumed. */
    if( xIsConnectionEstablished == pdTRUE )
    {
        /* Close the network connection.  */
        xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );

        if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
        {
            xDemoStatus = pdFAIL;
            LogError( ( "SecureSocketsTransport_Disconnect() failed to close the network connection. "
                        "StatusCode=%d.", ( int ) xNetworkStatus ) );
        }
    }
    return xDemoStatus;
}


#define AKNANO_DEVICE_GATEWAY_PORT 8443
#define AKNANO_DEVICE_GATEWAY_ENDPOINT "ac1f3cae-6b17-4872-b559-d197508b3620.ota-lite.foundries.io"

#define AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN ( ( uint16_t ) ( sizeof( AKNANO_DEVICE_GATEWAY_ENDPOINT ) - 1 ) )

/* FIO: */
static const char akNanoDeviceGateway_ROOT_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIBmDCCAT2gAwIBAgIUey/eV2WsUqW2lLoE4rTjoO7kShAwCgYIKoZIzj0EAwIw\n"
"KzETMBEGA1UEAwwKRmFjdG9yeS1DQTEUMBIGA1UECwwLbnhwLWhidC1wb2MwHhcN\n"
"MjExMDE1MTgyNzEwWhcNNDExMDEwMTgyNzEwWjArMRMwEQYDVQQDDApGYWN0b3J5\n"
"LUNBMRQwEgYDVQQLDAtueHAtaGJ0LXBvYzBZMBMGByqGSM49AgEGCCqGSM49AwEH\n"
"A0IABIT/AWqA/7PhOCAipzILFAWIoVQnkUyjyJQpgxzSR9KtUN2zgUWiIht8ZG81\n"
"KEmt0eNu9LHFEE86zwRu6WrePDCjPzA9MAwGA1UdEwQFMAMBAf8wCwYDVR0PBAQD\n"
"AgIEMCAGA1UdJQEB/wQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATAKBggqhkjOPQQD\n"
"AgNJADBGAiEA1IF/KOEvqfkZpOK3Ft+o53bINHowmRSU2OBfBFcrxa0CIQDS+TPs\n"
"Z8p3LrF8dshWpoglIY7j7gPb55i5cZ+XC1hvsw==\n"
"-----END CERTIFICATE-----\n";

static const uint32_t akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN = sizeof( akNanoDeviceGateway_ROOT_CERTIFICATE_PEM );

static BaseType_t prvConnectToDevicesGateway( NetworkContext_t * pxNetworkContext )
{
    // LogInfo( ( "prvConnectToDevicesGateway: BEGIN") );
    // vTaskDelay( pdMS_TO_TICKS( 100 ) );

    ServerInfo_t xServerInfo = { 0 };
    SocketsConfig_t xSocketsConfig = { 0 };
    BaseType_t xStatus = pdPASS;
    TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_SUCCESS;

    /* Initializer server information. */
    xServerInfo.pHostName = AKNANO_DEVICE_GATEWAY_ENDPOINT;
    xServerInfo.hostNameLength = AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN;
    xServerInfo.port = AKNANO_DEVICE_GATEWAY_PORT;

    /* Configure credentials for TLS mutual authenticated session. */
    xSocketsConfig.enableTls = true;
    xSocketsConfig.pAlpnProtos = NULL;
    xSocketsConfig.maxFragmentLength = 0;
    xSocketsConfig.disableSni = false;
    xSocketsConfig.pRootCa = akNanoDeviceGateway_ROOT_CERTIFICATE_PEM;
    xSocketsConfig.rootCaSize = akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN;
    xSocketsConfig.sendTimeoutMs = 3000;
    xSocketsConfig.recvTimeoutMs = 3000;

    /* Establish a TLS session with the HTTP server. This example connects to
     * the HTTP server as specified in democonfigAWS_IOT_ENDPOINT and
     * democonfigAWS_HTTP_PORT in http_demo_mutual_auth_config.h. */
    LogInfo( ( "Establishing a TLS session to %.*s:%d.",
               ( int32_t ) xServerInfo.hostNameLength,
               xServerInfo.pHostName,
               xServerInfo.port ) );

    vTaskDelay( pdMS_TO_TICKS( 100 ) );

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = SecureSocketsTransport_Connect( pxNetworkContext,
                                                     &xServerInfo,
                                                     &xSocketsConfig );

    LogInfo(("prvConnectToDevicesGateway result=%d", xNetworkStatus));
    if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}



static BaseType_t prvSendHttpRequest( const TransportInterface_t * pxTransportInterface,
                                      const char * pcMethod,
                                      const char * pcPath,
                                      const char * pcBody,
                                      size_t xBodyLen,
                                      HTTPResponse_t *pxResponse,
                                      struct aknano_settings *aknano_settings
                                      )
{
    /* Return value of this method. */
    BaseType_t xStatus = pdPASS;

    /* Configurations of the initial request headers that are passed to
     * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo;
    /* Represents a response returned from an HTTP server. */
    // HTTPResponse_t xResponse;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders;

    /* Return value of all methods from the HTTP Client library API. */
    HTTPStatus_t xHTTPStatus = HTTPSuccess;

    configASSERT( pcMethod != NULL );
    configASSERT( pcPath != NULL );

    /* Initialize all HTTP Client library API structs to 0. */
    ( void ) memset( &xRequestInfo, 0, sizeof( xRequestInfo ) );
    ( void ) memset( pxResponse, 0, sizeof( *pxResponse ) );
    ( void ) memset( &xRequestHeaders, 0, sizeof( xRequestHeaders ) );

    pxResponse->getTime = osfGetTime; //nondet_boot();// ? NULL : GetCurrentTimeStub;

    /* Initialize the request object. */
    xRequestInfo.pHost = AKNANO_DEVICE_GATEWAY_ENDPOINT;
    xRequestInfo.hostLen = AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN;
    xRequestInfo.pMethod = pcMethod;
    xRequestInfo.methodLen = strlen(pcMethod);
    xRequestInfo.pPath = pcPath;
    xRequestInfo.pathLen = strlen(pcPath);

    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
     * can be sent over the same established TCP connection. */
    xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

    /* Set the buffer used for storing request headers. */
    xRequestHeaders.pBuffer = ucUserBuffer;
    xRequestHeaders.bufferLen = democonfigUSER_BUFFER_LENGTH;

    xHTTPStatus = HTTPClient_InitializeRequestHeaders( &xRequestHeaders,
                                                       &xRequestInfo );


    char *tag = aknano_settings->tag;
    char *factory_name = aknano_settings->factory_name;
    int version = aknano_settings->running_version;

    char active_target[200];
    snprintf(active_target, sizeof(active_target), "%s-%d", factory_name, version);

    /* LogInfo(("HTTP_GET tag='%s'  target='%s'", tag, active_target)); */
    /* AkNano: Headers only required for HTTP GET root.json */
    HTTPClient_AddHeader(&xRequestHeaders, "x-ats-tags", strlen("x-ats-tags"), tag, strlen(tag));
    HTTPClient_AddHeader(&xRequestHeaders, "x-ats-target", strlen("x-ats-target"), active_target, strlen(active_target));

    if( xHTTPStatus == HTTPSuccess )
    {
        /* Initialize the response object. The same buffer used for storing
         * request headers is reused here. */
        pxResponse->pBuffer = ucUserBuffer;
        pxResponse->bufferLen = democonfigUSER_BUFFER_LENGTH;

        // LogInfo( ( "Sending HTTP %.*s request to %.*s%.*s...",
        //            ( int32_t ) xRequestInfo.methodLen, xRequestInfo.pMethod,
        //            ( int32_t ) AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN, AKNANO_DEVICE_GATEWAY_ENDPOINT,
        //            ( int32_t ) xRequestInfo.pathLen, xRequestInfo.pPath ) );
        // LogDebug( ( "Request Headers:\n%.*s\n"
        //             "Request Body:\n%.*s\n",
        //             ( int32_t ) xRequestHeaders.headersLen,
        //             ( char * ) xRequestHeaders.pBuffer,
        //             ( int32_t ) xBodyLen, pcBody ) );

        /* Send the request and receive the response. */
        xHTTPStatus = HTTPClient_Send( pxTransportInterface,
                                       &xRequestHeaders,
                                       ( uint8_t * ) pcBody,
                                       xBodyLen,
                                       pxResponse,
                                       0 );
    }
    else
    {
        LogError( ( "Failed to initialize HTTP request headers: Error=%s.",
                    HTTPClient_strerror( xHTTPStatus ) ) );
    }

    if( xHTTPStatus == HTTPSuccess )
    {
        LogInfo( ( "Received HTTP response from %.*s%.*s. Status Code=%u",
                   ( int32_t ) AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN, AKNANO_DEVICE_GATEWAY_ENDPOINT,
                   ( int32_t ) xRequestInfo.pathLen, xRequestInfo.pPath, pxResponse->statusCode ) );
        LogDebug( ( "Response Headers:\n%.*s\n",
                    ( int32_t ) pxResponse->headersLen, pxResponse->pHeaders ) );
        // LogInfo( ( "Status Code: %u",
        //             pxResponse->statusCode ) );
        LogDebug( ( "Response Body:\n%.*s",
                    ( int32_t ) pxResponse->bodyLen, pxResponse->pBody ) );
    }
    else
    {
        LogError( ( "Failed to send HTTP %.*s request to %.*s%.*s: Error=%s.",
                    ( int32_t ) xRequestInfo.methodLen, xRequestInfo.pMethod,
                    ( int32_t ) AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN, AKNANO_DEVICE_GATEWAY_ENDPOINT,
                    ( int32_t ) xRequestInfo.pathLen, xRequestInfo.pPath,
                    HTTPClient_strerror( xHTTPStatus ) ) );
    }

    if( xHTTPStatus != HTTPSuccess )
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}

//#include "netif.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"
#include "lwip/netifapi.h"


static char bodyBuffer[500];
extern struct netif netif;

static void fill_network_info(char* output, size_t max_length)
{
	char* ipv4 = (char*)&netif.ip_addr.addr;
	uint8_t* mac = netif.hwaddr;

	snprintf(output, max_length,
		"{" \
		" \"local_ipv4\": \"%u.%u.%u.%u\"," \
		" \"mac\": \"%02x:%02x:%02x:%02x:%02x:%02x\"" \
		"}",
		ipv4[0], ipv4[1], ipv4[2], ipv4[3],
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
	);

	LogInfo(("fill_network_info: %s", output));
}

// size_t aknano_write_to_nvs(int id, void *data, size_t len)
// {
//     return 0;
// }

static void parse_config(char* config_data, int buffer_len, struct aknano_settings *aknano_settings)
{
    JSONStatus_t result = JSON_Validate( config_data, buffer_len );
    char * value;
    size_t valueLength;
    int int_value;


    // {"polling_interval":{"Value":"10","Unencrypted":true},"tag":{"Value":"devel","Unencrypted":true},"z-50-fioctl.toml":{"Value":"\n[pacman]\n  tags = \"devel\"\n","Unencrypted":true,"OnChanged":["/usr/share/fioconfig/handlers/aktualizr-toml-update"]}}"
    if( result == JSONSuccess )
    {
        result = JSON_Search( config_data, buffer_len,
                             "tag.Value", strlen("tag.Value"),
                              &value, &valueLength );

        if( result == JSONSuccess ) {
            if (strncmp(value, aknano_settings->tag, valueLength)) {
                LogInfo(("parse_config_data: Tag has changed ('%s' => '%s'). Updating value",
                        aknano_settings->tag,
                        value));
                strncpy(aknano_settings->tag, value,
                        valueLength);
                aknano_settings->tag[valueLength] = '\0';
                // aknano_write_to_nvs(AKNANO_NVS_ID_TAG, aknano_settings->tag,
                //         strnlen(aknano_settings->tag,
                //         AKNANO_MAX_TAG_LENGTH));
            }
        } else {
            LogInfo(("parse_config_data: Tag config not found"));
        }

        result = JSON_Search( config_data, buffer_len,
                             "polling_interval.Value", strlen("polling_interval.Value"),
                              &value, &valueLength );

        if( result == JSONSuccess ) {
            if (sscanf(value, "%d", &int_value) <= 0) {
                LogWarn(("Invalid polling_interval '%s'", value));
                return -1;
            }

            if (int_value != aknano_settings->polling_interval) {
                LogInfo(("parse_config_data: Polling interval has changed (%d => %d). Updating value",
                        aknano_settings->polling_interval, int_value));
                aknano_settings->polling_interval = int_value;
                // aknano_write_to_nvs(AKNANO_NVS_ID_POLLING_INTERVAL,
                //         &aknano_settings->polling_interval,
                //         sizeof(aknano_settings->polling_interval));
            } else {
                LogInfo(("parse_config_data: Polling interval is unchanged (%d)",
                        aknano_settings->polling_interval));
            }
        } else {
            LogInfo(("parse_config_data: polling_interval config not found"));
        }
    } else {
        LogWarn(("Invalid config JSON result=%d", result));
    }
}

#include "compile_epoch.h"
#ifndef BUILD_EPOCH_MS
#define BUILD_EPOCH_MS 1637778974000
#endif

static void get_pseudo_time_str(time_t boot_up_epoch, char *output, const char* event_type, int success)
{
    int event_delta = 0;
    int base_delta = 180;

	if (boot_up_epoch == 0)
		boot_up_epoch = BUILD_EPOCH_MS / 1000;


	if (!strcmp(event_type, AKNANO_EVENT_DOWNLOAD_STARTED)) {
		event_delta = 1;
	} else if (!strcmp(event_type, AKNANO_EVENT_DOWNLOAD_COMPLETED)) {
        event_delta = 20;
	} else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_STARTED)) {
        event_delta = 21;
	} else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_APPLIED)) {
        event_delta = 22;
	} else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_COMPLETED)) {
        if (success == AKNANO_EVENT_SUCCESS_TRUE)
            event_delta = 42;
        else
            event_delta = 242;
    }

	// time_t current_epoch_ms = boot_up_epoch_ms + base_delta + event_delta;
	time_t current_epoch_sec = boot_up_epoch + base_delta + event_delta;
	struct tm *tm = gmtime(&current_epoch_sec);

	sprintf(output, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		0);

	LogInfo(("get_pseudo_time_str: %s", output));
}


static void get_time_str(time_t boot_up_epoch, char *output)
{
	if (boot_up_epoch == 0)
		boot_up_epoch = 1637778974; // 2021-11-24

	time_t current_epoch_sec = boot_up_epoch + (xTaskGetTickCount() / 1000);
	struct tm *tm = gmtime(&current_epoch_sec);

	sprintf(output, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
		tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec,
		0);

	LogInfo(("get_time_str: %s (boot_up_epoch=%d)", output, boot_up_epoch));
}


#include "psa/crypto.h"
static void btox(char *xp, const char *bb, int n) 
{
	const char xx[]= "0123456789ABCDEF";
	while (--n >= 0) xp[n] = xx[(bb[n>>1] >> ((1 - (n&1)) << 2)) & 0xF];
}

static void serial_string_to_uuid_string(const char* serial, char *uuid)
{
	const char *s;
	char *u;
	int i;

	s = serial;
	u = uuid;

	for(i=0; i<36; i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23)
			*u = '-';
		else {
			*u = *s;
			s++;
		}
		u++;
	}
}

#include "drivers/fsl_trng.h"
status_t AkNanoGenRandomBytes(char *output, size_t size)
{
    trng_config_t trng_config;
    int status = TRNG_GetDefaultConfig(&trng_config);
    if (status != kStatus_Success)
    {
        return status;
    }
    // Set sample mode of the TRNG ring oscillator to Von Neumann, for better random data.
    trng_config.sampleMode = kTRNG_SampleModeVonNeumann;
    status = TRNG_Init(TRNG, &trng_config);
    if (status != kStatus_Success)
    {
        return status;
    }
    status = TRNG_GetRandomData(TRNG, output, size);
    if (status != kStatus_Success)
    {
        return status;
    }
    return kStatus_Success;
}

int aknano_gen_serial_and_uuid(char *uuid_string, char *serial_string)
{
	char serial_bytes[16];
    AkNanoGenRandomBytes(serial_bytes, sizeof(serial_bytes));
	btox(serial_string, serial_bytes, sizeof(serial_bytes) * 2);
	serial_string_to_uuid_string(serial_string, uuid_string);
	uuid_string[36] = '\0';
	serial_string[32] = '\0';
	LogInfo(("uuid='%s', serial='%s'", uuid_string, serial_string));
	return 0;
}


#include <time.h>

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "sntp_example.h"
#include "lwip/netif.h"

void sntp_set_system_time(u32_t sec)
{
    LogInfo(("SNTP sntp_set_system_time"));
    char buf[32];
    struct tm current_time_val;
    time_t current_time = (time_t)sec;

    localtime_r(&current_time, &current_time_val);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &current_time_val);
    //xaknano_settings.boot_up_epoch_ms = (sec * 1000);// - xTaskGetTickCount();
    xaknano_settings.boot_up_epoch = sec;// - xTaskGetTickCount();

    LogInfo(("SNTP time: %s  sec=%d xaknano_settings.boot_up_epoch=%d xTaskGetTickCount()=%llu\n", buf, sec, xaknano_settings.boot_up_epoch, xTaskGetTickCount()));
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // LOCK_TCPIP_CORE();
    sntp_stop();
    // UNLOCK_TCPIP_CORE();
}

void sntp_example_init(void)
{
    ip4_addr_t ip_sntp_server;

    IP4_ADDR(&ip_sntp_server, 208, 100, 4, 52);

    LOCK_TCPIP_CORE();

    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setserver(0, &ip_sntp_server);

    sntp_init();
    UNLOCK_TCPIP_CORE();
    LogInfo(("SNTP Initialized"));
}

static void SNTPRequest()
{
    sntp_example_init();

    int i;
    for(i=0; i<5; i++) {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        if (xaknano_settings.boot_up_epoch) {
            vTaskDelay( pdMS_TO_TICKS( 500 ) );
            break;
        }
    }
    if (i == 5) {
        // LOCK_TCPIP_CORE();
        sntp_stop();
        // UNLOCK_TCPIP_CORE();
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }

    LogInfo(("Proceeding after sntp"));
}

static bool fill_event_payload(char *payload,
					struct aknano_settings *aknano_settings,
					const char* event_type,
					int version, int success)
{
	int old_version = aknano_settings->running_version;
	int new_version = version;
	char details[200];
	char current_time_str[50];
	char *correlation_id = aknano_settings->ongoing_update_correlation_id;
	char target[AKNANO_MAX_FACTORY_NAME_LENGTH+15];
	char evt_uuid[AKNANO_MAX_UUID_LENGTH], _serial_string[AKNANO_MAX_SERIAL_LENGTH];
	char* success_string;

    if (success == AKNANO_EVENT_SUCCESS_UNDEFINED) {
        success_string = version < 0? "\"success\": false," : "";
    } else if (success == AKNANO_EVENT_SUCCESS_TRUE) {
        success_string = "\"success\": true,";
    } else {
        success_string = "\"success\": false,";
    }

	aknano_gen_serial_and_uuid(evt_uuid, _serial_string);

	if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_COMPLETED)) {
		old_version = aknano_settings->last_confirmed_version;
		new_version = aknano_settings->running_version;
	}
	snprintf(target, sizeof(target), "%s-%d", aknano_settings->factory_name, new_version);

	if (strnlen(correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH) == 0)
		snprintf(correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH, "%s-%s", target, aknano_settings->uuid);

	if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_APPLIED)) {
		snprintf(details, sizeof(details), "Updating from v%d to v%d tag: %s. Image written to flash. Rebooting.",
			old_version, new_version, aknano_settings->tag);
	} else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_COMPLETED)) {
		if (version < 0) {
			snprintf(details, sizeof(details), "Rollback to v%d after failed update to v%d tag: %s",
				old_version, aknano_settings->last_applied_version, aknano_settings->tag);
		} else {
			snprintf(details, sizeof(details), "Updated from v%d to v%d tag: %s. Image confirmed.",
				old_version, new_version, aknano_settings->tag);

		}
	} else {
		snprintf(details, sizeof(details), "Updating from v%d to v%d tag: %s.",
			old_version, new_version, aknano_settings->tag);
	}

    if (aknano_settings->boot_up_epoch)
        get_time_str(aknano_settings->boot_up_epoch, current_time_str);
    else
        get_pseudo_time_str(aknano_settings->boot_up_epoch, current_time_str, event_type, success);

    LogInfo(("fill_event_payload: current_time_str=%s", current_time_str));
    LogInfo(("fill_event_payload: old_version=%d", old_version));
    LogInfo(("fill_event_payload: new_version=%d", new_version));
    LogInfo(("fill_event_payload: version=%d", version));
    LogInfo(("fill_event_payload: aknano_settings->tag=%s", aknano_settings->tag));
    LogInfo(("fill_event_payload: correlation_id=%s", correlation_id));
    LogInfo(("fill_event_payload: evt_uuid=%s", evt_uuid));

	snprintf(payload, 1000,
		"[{" \
			"\"id\": \"%s\","\
			"\"deviceTime\": \"%s\","\
			"\"eventType\": {"\
				"\"id\": \"%s\","\
				"\"version\": 0"\
			"},"\
			"\"event\": {"\
				"\"correlationId\": \"%s\","\
				"\"targetName\": \"%s\","\
				"\"version\": \"%d\","\
				"%s"\
				"\"details\": \"%s\""\
			"}"\
		"}]",
		evt_uuid, current_time_str, event_type,
		correlation_id, target, new_version,
		success_string, details);
	LogInfo(("Event: %s %s%s", event_type, details, success_string));
	return true;
}

static bool AkNanoSendEvent(struct aknano_settings *aknano_settings,
					const char* event_type,
					int version, int success)
{
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    TransportSocketStatus_t xNetworkStatus;
    BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;

    LogInfo(("AkNanoSendEvent BEGIN %s", event_type));
    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToDevicesGateway(&xNetworkContext);

    LogInfo(("prvConnectToServer Result: %d", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        /* Set a flag indicating that a TLS connection exists. */
        xIsConnectionEstablished = pdTRUE;

        /* Define the transport interface. */
        xTransportInterface.pNetworkContext = &xNetworkContext;
        xTransportInterface.send = SecureSocketsTransport_Send;
        xTransportInterface.recv = SecureSocketsTransport_Recv;
    }
    else
    {
        /* Log error to indicate connection failure after all
            * reconnect attempts are over. */
        LogError( ( "Failed to connect to HTTP server") );
    }

    if( xDemoStatus == pdPASS )
    {
        fill_event_payload(bodyBuffer, aknano_settings, event_type, version, success);

        LogInfo((ANSI_COLOR_YELLOW "Sending %s event" ANSI_COLOR_RESET,
                event_type));
        LogInfo(("Event payload: %.50s", bodyBuffer));

        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_POST,
                                            "/events", bodyBuffer, strlen(bodyBuffer), &xResponse, aknano_settings);

    }

    /* Close the network connection.  */
    xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );

    LogInfo(("AkNanoSendEvent END %s", event_type));
}


char single_target_buffer[2000];
void aknano_handle_manifest_data(struct aknano_context *context,
					uint8_t *dst, size_t *offset, 
					uint8_t *src, size_t len);

static void AkNanoUpdateSettingsInFlash(struct aknano_settings *aknano_settings);


int AkNanoPoll(struct aknano_context *aknano_context)
{
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    TransportSocketStatus_t xNetworkStatus;
    BaseType_t xIsConnectionEstablished = pdFALSE;
    UBaseType_t uxDemoRunCount = 0UL;
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;
    bool isUpdateRequired = false;
    bool isRebootRequired = false;

    LogInfo(("AkNanoPoll BEGIN"));
    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToDevicesGateway(&xNetworkContext);


    struct aknano_settings *aknano_settings = aknano_context->settings;

    LogInfo(("prvConnectToServer Result: %d", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        /* Set a flag indicating that a TLS connection exists. */
        xIsConnectionEstablished = pdTRUE;

        /* Define the transport interface. */
        xTransportInterface.pNetworkContext = &xNetworkContext;
        xTransportInterface.send = SecureSocketsTransport_Send;
        xTransportInterface.recv = SecureSocketsTransport_Recv;
    }
    else
    {
        /* Log error to indicate connection failure after all
            * reconnect attempts are over. */
        LogError( ( "Failed to connect to HTTP server") );
    }

    if( xDemoStatus == pdPASS )
    {
        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_GET,
                            "/repo/99999.root.json", "", 0, &xResponse, aknano_context->settings);


        xDemoStatus = prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_GET,
                            "/config", "", 0, &xResponse, aknano_context->settings);
        if (xDemoStatus == pdPASS)
            parse_config(xResponse.pBody, xResponse.bodyLen, aknano_context->settings);


        xDemoStatus = prvSendHttpRequest( &xTransportInterface,HTTP_METHOD_GET,
                            "/repo/targets.json", "", 0, &xResponse, aknano_context->settings);
        if (xDemoStatus == pdPASS) {
            size_t offset = 0;
            aknano_handle_manifest_data(aknano_context, single_target_buffer, &offset, xResponse.pBody, xResponse.bodyLen);
            if (aknano_context->aknano_json_data.selected_target.version == 0) {
                LogInfo(("* No matching target found in manifest"));
            } else {
                LogInfo(("* Manifest data parsing result: highest version=%u last unconfirmed applied=%d",
                    aknano_context->aknano_json_data.selected_target.version,
                    aknano_settings->last_applied_version));

                if (aknano_context->aknano_json_data.selected_target.version == aknano_settings->last_applied_version) {
                    LogInfo(("* Same version was already applied (and failed). Do not retrying it"));

                } else if (aknano_context->settings->running_version != aknano_context->aknano_json_data.selected_target.version) {
                    LogInfo((ANSI_COLOR_GREEN "* Update required: %u -> %u" ANSI_COLOR_RESET, 
                            aknano_context->settings->running_version, aknano_context->aknano_json_data.selected_target.version));
                    isUpdateRequired = true;
                } else {
                    LogInfo(("* No update required"));
                }
            }
        }

        snprintf(bodyBuffer, sizeof(bodyBuffer),
            "{ "\
            " \"product\": \"%s\"," \
            " \"description\": \"MCUXpresso PoC\"," \
            " \"claimed\": true, "\
            " \"serial\": \"%s\", "\
            " \"id\": \"%s\", "\
            " \"class\": \"MCU\" "\
            "}",
            CONFIG_BOARD, aknano_settings->serial, CONFIG_BOARD);
        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_PUT,
                                            "/system_info", bodyBuffer, strlen(bodyBuffer), &xResponse, aknano_context->settings);

        fill_network_info(bodyBuffer, sizeof(bodyBuffer));
        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_PUT,
                                            "/system_info/network", bodyBuffer, strlen(bodyBuffer), &xResponse, aknano_context->settings);

        LogInfo(("aknano_settings->tag=%s",aknano_settings->tag));
        sprintf(bodyBuffer,
                "[aknano_settings]\n" \
                "poll_interval = %d\n" \
                "hw_id = \"%s\"\n" \
                "tag = \"%s\"\n" \
                "binary_compilation_local_time = \""__DATE__ " " __TIME__"\"",
                aknano_settings->polling_interval,
                CONFIG_BOARD,
                aknano_settings->tag);
        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_PUT,
                                            "/system_info/config", bodyBuffer, strlen(bodyBuffer), &xResponse, aknano_context->settings);

    }

    /* Close the network connection.  */
    xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );


    if (isUpdateRequired) {

        // version = hb_context.aknano_json_data.selected_target.version;

        char unused_serial[AKNANO_MAX_SERIAL_LENGTH];
        /* Gen a random correlation ID for the update events */
        aknano_gen_serial_and_uuid(aknano_settings->ongoing_update_correlation_id,
                    unused_serial);


        // aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID,
        //             aknano_settings.ongoing_update_correlation_id,
        //             AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH);

        AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_STARTED, aknano_context->aknano_json_data.selected_target.version, AKNANO_EVENT_SUCCESS_UNDEFINED);
        if (AkNanoDownloadAndFlashImage(aknano_context)) {
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED, aknano_context->aknano_json_data.selected_target.version, AKNANO_EVENT_SUCCESS_TRUE);
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_INSTALLATION_STARTED, aknano_context->aknano_json_data.selected_target.version, AKNANO_EVENT_SUCCESS_UNDEFINED);
            
            aknano_settings->last_applied_version = aknano_context->aknano_json_data.selected_target.version;
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_INSTALLATION_APPLIED, aknano_context->aknano_json_data.selected_target.version, AKNANO_EVENT_SUCCESS_TRUE);

            LogInfo(("Requesting update on next boot (ReadyForTest)"));

            status_t status;
            status = bl_update_image_state(kSwapType_ReadyForTest);
            if (status != kStatus_Success) {
                LogWarn(("Error setting image as ReadyForTest. status=%d", status));
            } else {
                isRebootRequired = true;
            }
        } else {
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED, aknano_context->aknano_json_data.selected_target.version, AKNANO_EVENT_SUCCESS_FALSE);
        }

        AkNanoUpdateSettingsInFlash(aknano_settings);

        // TODO: Add download failed event
        // AkNanoSendEvent(aknano_context, AKNANO_EVENT_DOWNLOAD_COMPLETED, aknano_context->aknano_json_data.selected_target.version);
    }

    if (isRebootRequired) {
        LogInfo(("Rebooting...."));
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
        NVIC_SystemReset();
    }

    LogInfo(("AkNanoPoll END"));

    return 0;
}

void InitFlashStorage();
status_t ReadFlashStorage(int offset, void *output, size_t outputMaxLen);
status_t UpdateFlashStoragePage(int offset, void *data);

static void AkNanoUpdateSettingsInFlash(struct aknano_settings *aknano_settings)
{
    char flashPageBuffer[256];
    memset(flashPageBuffer, 0, sizeof(flashPageBuffer));

    memcpy(flashPageBuffer, &aknano_settings->last_applied_version, sizeof(int));
    memcpy(flashPageBuffer + sizeof(int), &aknano_settings->last_confirmed_version, sizeof(int));
    memcpy(flashPageBuffer + sizeof(int) * 2, aknano_settings->ongoing_update_correlation_id, sizeof(aknano_settings->ongoing_update_correlation_id));
    
    
    LogInfo(("Saving buffer: 0x%x 0x%x 0x%x 0x%x    0x%x 0x%x 0x%x 0x%x    0x%x 0x%x 0x%x 0x%x", 
    flashPageBuffer[0], flashPageBuffer[1], flashPageBuffer[2], flashPageBuffer[3],
    flashPageBuffer[4], flashPageBuffer[5], flashPageBuffer[6], flashPageBuffer[7],
    flashPageBuffer[8], flashPageBuffer[9], flashPageBuffer[10], flashPageBuffer[11]));
    UpdateFlashStoragePage(AKNANO_FLASH_OFF_STATE_BASE, flashPageBuffer);
}

static void AkNanoInitSettings(struct aknano_settings *aknano_settings)
{
    memset(aknano_settings, 0, sizeof(*aknano_settings));
    strcpy(aknano_settings->tag, "devel");
    aknano_settings->polling_interval = 60;
    strcpy(aknano_settings->factory_name, "nxp-hbt-poc");

    // strcpy(aknano_settings->serial, "000000000000");
    
    bl_get_image_build_num(&aknano_settings->running_version);
    LogInfo(("AkNanoInitSettings: aknano_settings->running_version=%u", aknano_settings->running_version));

    ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, aknano_settings->device_certificate, sizeof(aknano_settings->device_certificate));
    LogInfo(("AkNanoInitSettings: device_certificate=%.30s", aknano_settings->device_certificate));

    ReadFlashStorage(AKNANO_FLASH_OFF_DEV_KEY, aknano_settings->device_priv_key, sizeof(aknano_settings->device_priv_key));
    LogInfo(("AkNanoInitSettings: device_priv_key=%.30s", aknano_settings->device_priv_key));

    ReadFlashStorage(AKNANO_FLASH_OFF_DEV_SERIAL, aknano_settings->serial, sizeof(aknano_settings->serial));
    LogInfo(("AkNanoInitSettings: serial=%s", aknano_settings->serial));

    ReadFlashStorage(AKNANO_FLASH_OFF_DEV_UUID, aknano_settings->uuid, sizeof(aknano_settings->uuid));
    LogInfo(("AkNanoInitSettings: uuid=%s", aknano_settings->uuid));

    ReadFlashStorage(AKNANO_FLASH_OFF_LAST_APPLIED_VERSION, &aknano_settings->last_applied_version, sizeof(aknano_settings->last_applied_version));
    if (aknano_settings->last_applied_version < 0 || aknano_settings->last_applied_version > 999999999)
        aknano_settings->last_applied_version = 0;
    LogInfo(("AkNanoInitSettings: last_applied_version=%d", aknano_settings->last_applied_version));

    ReadFlashStorage(AKNANO_FLASH_OFF_LAST_CONFIRMED_VERSION, &aknano_settings->last_confirmed_version, sizeof(aknano_settings->last_confirmed_version));
    if (aknano_settings->last_confirmed_version < 0 || aknano_settings->last_confirmed_version > 999999999)
        aknano_settings->last_confirmed_version = 0;
    LogInfo(("AkNanoInitSettings: last_confirmed_version=%d", aknano_settings->last_confirmed_version));

    ReadFlashStorage(AKNANO_FLASH_OFF_ONGOING_UPDATE_COR_ID, &aknano_settings->ongoing_update_correlation_id, sizeof(aknano_settings->ongoing_update_correlation_id));
    if (aknano_settings->ongoing_update_correlation_id[0] == 0xFF)
        aknano_settings->ongoing_update_correlation_id[0] = 0;
    LogInfo(("AkNanoInitSettings: ongoing_update_correlation_id=%s", aknano_settings->ongoing_update_correlation_id));

}

// extern void UpdateClientCertificate(const char *clientCertPem, const char *clientKeyPem);

static int aknano_handle_img_confirmed(struct aknano_settings *aknano_settings)
{
	bool image_ok;
    uint32_t currentStatus;

    if (bl_get_image_state(&currentStatus) == kStatus_Success) {
        if (currentStatus == kSwapType_Testing) {
            LogInfo(("Current image state is Testing. Marking as permanent"));
            bl_update_image_state(kSwapType_Permanent);
            // AkNanoSendEvent(aknano_settings, AKNANO_EVENT_INSTALLATION_APPLIED, aknano_settings->running_version);
        } else if (currentStatus == kSwapType_ReadyForTest) {
            LogInfo(("Current image state is ReadyForTest"));
        } else {
            image_ok = true;
            LogInfo(("Current image state is Permanent"));
            // TODO: Only send on Testing image
            // AkNanoSendEvent(aknano_settings, AKNANO_EVENT_INSTALLATION_APPLIED, aknano_settings->running_version);
        }
    } else {
        LogWarn(("Erro getting image state"));
        image_ok = true;
    }

	// ret = boot_write_img_confirmed_multi(1);
	// LOG_INF("Confirmed image 1 %d", ret);
	// image_ok = boot_is_img_confirmed();
	LogInfo(("Image is%s confirmed OK", image_ok ? "" : " not"));

	LogInfo(("aknano_settings.ongoing_update_correlation_id='%s'", aknano_settings->ongoing_update_correlation_id));

	if (aknano_settings->last_applied_version
		&& aknano_settings->last_applied_version != aknano_settings->running_version
		&& strnlen(aknano_settings->ongoing_update_correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH) > 0) {

		LogInfo(("A rollback was done"));
		AkNanoSendEvent(aknano_settings,
				  AKNANO_EVENT_INSTALLATION_COMPLETED,
				  -1, AKNANO_EVENT_SUCCESS_FALSE);
		//aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID, "", 0);
		
        memset(aknano_settings->ongoing_update_correlation_id, 0, sizeof(aknano_settings->ongoing_update_correlation_id));
        AkNanoUpdateSettingsInFlash(aknano_settings);
	}

	if (!image_ok) {
		//ret = boot_write_img_confirmed();
		//if (ret < 0) {
		//	LogError(("Couldn't confirm this image: %d", ret));
		//	return ret;
		//}

		// LogInfo(("Marked image as OK"));

		// image_ok = boot_is_img_confirmed();
		// LogInfo(("after Image is%s confirmed OK", image_ok ? "" : " not"));

		AkNanoSendEvent(aknano_settings, AKNANO_EVENT_INSTALLATION_COMPLETED, 0, AKNANO_EVENT_SUCCESS_TRUE);

		// aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID, "", 0);
		// aknano_write_to_nvs(AKNANO_NVS_ID_LAST_CONFIRMED_VERSION, &aknano_settings.running_version, sizeof(aknano_settings.running_version));
		// aknano_write_to_nvs(AKNANO_NVS_ID_LAST_APPLIED_VERSION, &zero, sizeof(zero));

		memset(aknano_settings->ongoing_update_correlation_id, 0, sizeof(aknano_settings->ongoing_update_correlation_id));
		aknano_settings->last_applied_version = 0;
		aknano_settings->last_confirmed_version = aknano_settings->running_version;
        AkNanoUpdateSettingsInFlash(aknano_settings);

		// aknano_write_to_nvs(AKNANO_NVS_ID_LAST_CONFIRMED_TAG, aknano_settings.tag, sizeof(aknano_settings.tag));

        // TODO: do the same in FreeRTOS?
		// ret = boot_erase_img_bank(FLASH_AREA_ID(image_1));
		// if (ret) {
		// 	LOG_ERR("Failed to erase second slot");
		// 	return ret;
		// }
	} else {
#if 0
		// TODO: testing this
		ret = boot_write_img_confirmed();
		if (ret < 0) {
			LOG_ERR("Couldn't confirm this image (forcing): %d", ret);
			return ret;
		}

		LOG_INF("Marked image as OK (forcing)");
		aknano_write_to_nvs(AKNANO_NVS_ID_LAST_CONFIRMED_VERSION, &aknano_settings.running_version, sizeof(aknano_settings.running_version));
		aknano_write_to_nvs(AKNANO_NVS_ID_LAST_APPLIED_VERSION, &zero, sizeof(zero));
#endif
	}

	if (aknano_settings->last_confirmed_version != aknano_settings->running_version)
	{
		// TODO: Should not be required, but doing it here because of temp/permanent bug
        AkNanoSendEvent(aknano_settings, AKNANO_EVENT_INSTALLATION_COMPLETED, 0, AKNANO_EVENT_SUCCESS_TRUE);
		memset(aknano_settings->ongoing_update_correlation_id, 0, sizeof(aknano_settings->ongoing_update_correlation_id));
		aknano_settings->last_applied_version = 0;
		aknano_settings->last_confirmed_version = aknano_settings->running_version;
        AkNanoUpdateSettingsInFlash(aknano_settings);

        LogInfo(("Updating aknano_settings->running_version in flash (%d -> %d)", aknano_settings->last_confirmed_version, aknano_settings->running_version));
    	aknano_settings->last_confirmed_version = aknano_settings->running_version;
        // strcpy(aknano_settings->ongoing_update_correlation_id, "ABCDEFGHIJKLMNOPQRSTUVXYZ");
        AkNanoUpdateSettingsInFlash(aknano_settings);
		// aknano_write_to_nvs(AKNANO_NVS_ID_LAST_CONFIRMED_VERSION, &aknano_settings.running_version, sizeof(aknano_settings.running_version));
	}

	return 0;
}

/* Perform device provisioning using the default TLS client credentials. */
void vDevModeKeyProvisioning_new(uint8_t *client_key, uint8_t *client_certificate )
{
    ProvisioningParams_t xParams;

    xParams.pucJITPCertificate = NULL;
    xParams.pucClientPrivateKey = client_key;
    xParams.pucClientCertificate = client_certificate;

    /* If using a JITR flow, a JITR certificate must be supplied. If using credentials generated by
     * AWS, this certificate is not needed. */
    if( ( NULL != xParams.pucJITPCertificate ) &&
        ( 0 != strcmp( "", ( const char * ) xParams.pucJITPCertificate ) ) )
    {
        /* We want the NULL terminator to be written to storage, so include it
         * in the length calculation. */
        xParams.ulJITPCertificateLength = sizeof( char ) + strlen( ( const char * ) xParams.pucJITPCertificate );
    }
    else
    {
        xParams.pucJITPCertificate = NULL;
    }

    /* The hard-coded client certificate and private key can be useful for
     * first-time lab testing. They are optional after the first run, though, and
     * not recommended at all for going into production. */
    if( ( NULL != xParams.pucClientPrivateKey ) &&
        ( 0 != strcmp( "", ( const char * ) xParams.pucClientPrivateKey ) ) )
    {
        /* We want the NULL terminator to be written to storage, so include it
         * in the length calculation. */
        xParams.ulClientPrivateKeyLength = sizeof( char ) + strlen( ( const char * ) xParams.pucClientPrivateKey );
    }
    else
    {
        xParams.pucClientPrivateKey = NULL;
    }

    if( ( NULL != xParams.pucClientCertificate ) &&
        ( 0 != strcmp( "", ( const char * ) xParams.pucClientCertificate ) ) )
    {
        /* We want the NULL terminator to be written to storage, so include it
         * in the length calculation. */
        xParams.ulClientCertificateLength = sizeof( char ) + strlen( ( const char * ) xParams.pucClientCertificate );
    }
    else
    {
        xParams.pucClientCertificate = NULL;
    }

    vAlternateKeyProvisioning( &xParams );
}

static void AkNanoInit(struct aknano_settings *aknano_settings) 
{
    InitFlashStorage();
    AkNanoInitSettings(aknano_settings);

    SNTPRequest();
    
    vDevModeKeyProvisioning_new((uint8_t*)xaknano_settings.device_priv_key, 
                                (uint8_t*)xaknano_settings.device_certificate );
    LogInfo(("vDevModeKeyProvisioning_new done"));
    // UpdateClientCertificate(aknano_settings->device_certificate, aknano_settings->device_priv_key);
    aknano_handle_img_confirmed(aknano_settings);
}

static void AkNanoInitContext(struct aknano_context *aknano_context, 
                            struct aknano_settings *aknano_settings)
{
    memset(aknano_context, 0, sizeof(*aknano_context));
    aknano_context->settings = aknano_settings;
}


/**
 * @brief Entry point of demo.
 */
int RunAkNanoDemo( bool xAwsIotMqttMode,
                        const char * pIdentifier,
                        void * pNetworkServerInfo,
                        void * pNetworkCredentialInfo,
                        const IotNetworkInterface_t * pxNetworkInterface )
{
    int sleepTime;

    ( void ) xAwsIotMqttMode;
    ( void ) pIdentifier;
    ( void ) pNetworkServerInfo;
    ( void ) pNetworkCredentialInfo;
    ( void ) pxNetworkInterface;

    LogInfo(("AKNano RunAkNanoDemo"));
    AkNanoInit(&xaknano_settings);
    while (true) {
        AkNanoInitContext(&xaknano_context, &xaknano_settings);
        AkNanoPoll(&xaknano_context);
        sleepTime = xaknano_settings.polling_interval * 1000;
        if (sleepTime < 5000)
            sleepTime = 5000;
        else if (sleepTime > 60 * 60 * 1000)
            sleepTime = 60 * 60 * 1000;

        LogInfo(("Sleeping %d ms\n\n", sleepTime));
        vTaskDelay( pdMS_TO_TICKS( sleepTime ) );
    }
    return 0;
}
