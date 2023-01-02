/*
 * Copyright 2021-2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
#define LIBRARY_LOG_LEVEL LOG_INFO
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#include "aknano_priv.h"

#define AKNANO_REST_API_PORT 443
#define AKNANO_REST_API_ENDPOINT "api.foundries.io"
#define AKNANO_REST_API_ENDPOINT_LEN ( ( uint16_t ) ( sizeof( AKNANO_REST_API_ENDPOINT ) - 1 ) )

/* FIO: */
static const char akNanoRestApi_ROOT_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n" \
"MIIFOTCCBCGgAwIBAgISBPmwrOSd+NTDWmpQBjrWS1YOMA0GCSqGSIb3DQEBCwUA\n" \
"MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD\n" \
"EwJSMzAeFw0yMjAxMDMxNjM1MjFaFw0yMjA0MDMxNjM1MjBaMBsxGTAXBgNVBAMT\n" \
"EGFwaS5mb3VuZHJpZXMuaW8wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB\n" \
"AQDBDyOT42FEfS2xELyGSLRNokTiAWnpvC++NWFzZQtQngCs+k+SB/vvkjalaZJv\n" \
"wVQt31+CdIoVomgzAX/1QN9Mw6FSUnuLQZ1lam/Aruseq8Tv5paMh1DUTPsrMtzi\n" \
"KZJbW/KkurXcQ6Mp2we3bzKiUUdLkXM9BY4iMtT0Xkarls2t4eXaAU/t1yorBwQu\n" \
"xnmFk4EEv59MUQbsAg/UAJ8VNgO/y3El4zQjHv8PYGD0mtRni4wlVuenZb4YaQvw\n" \
"sjHfnrELNtqvBzwMKtlCBj89QsWOpNsVzGsX+EmihcQJEK3Dgz0WEZ2ZCO33TqL4\n" \
"2TpNKhQqCVLzynhGnOisYv1BAgMBAAGjggJeMIICWjAOBgNVHQ8BAf8EBAMCBaAw\n" \
"HQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMAwGA1UdEwEB/wQCMAAwHQYD\n" \
"VR0OBBYEFAzvtPKWiqZZks5scP1CKquyGlPFMB8GA1UdIwQYMBaAFBQusxe3WFbL\n" \
"rlAJQOYfr52LFMLGMFUGCCsGAQUFBwEBBEkwRzAhBggrBgEFBQcwAYYVaHR0cDov\n" \
"L3IzLm8ubGVuY3Iub3JnMCIGCCsGAQUFBzAChhZodHRwOi8vcjMuaS5sZW5jci5v\n" \
"cmcvMC0GA1UdEQQmMCSCEGFwaS5mb3VuZHJpZXMuaW+CEGFwcC5mb3VuZHJpZXMu\n" \
"aW8wTAYDVR0gBEUwQzAIBgZngQwBAgEwNwYLKwYBBAGC3xMBAQEwKDAmBggrBgEF\n" \
"BQcCARYaaHR0cDovL2Nwcy5sZXRzZW5jcnlwdC5vcmcwggEFBgorBgEEAdZ5AgQC\n" \
"BIH2BIHzAPEAdgBByMqx3yJGShDGoToJQodeTjGLGwPr60vHaPCQYpYG9gAAAX4h\n" \
"AovaAAAEAwBHMEUCIQCrU8p277axgjNR8gHJjNrgco4cU/SgMTbH0pOJ71UwxgIg\n" \
"UD52p+3xkgE3iEJ2+y4nkpNCLlayvsfsoCtUdI3jU+8AdwApeb7wnjk5IfBWc59j\n" \
"pXflvld9nGAK+PlNXSZcJV3HhAAAAX4hAovqAAAEAwBIMEYCIQClRWqaRohClNvW\n" \
"2caYF0LniTVlo/zLdsSgS2g1vJSAXAIhAKvESkMdWrc+MGoUGnaXaQTJGmDJHXvV\n" \
"/53CeTYR/ydNMA0GCSqGSIb3DQEBCwUAA4IBAQADfUSC4jvc3RALMRt6EmvQR2wQ\n" \
"S/7M15+V0dsfcn4UzyTFoE9UBf3uS+s5HTEf8T+yzBE2ovKlOlro5YvUPWWwAzVv\n" \
"17Jh8eKleNFkieRLddWwcJ13Ck6IA204hrJMFvZzgJzGkpaf26x9STWu/pQR3CRS\n" \
"IBlsTkFDquWckUyG70kHx6mmcTDYBXjwtFNNYrvXkDzyJkWeC0vmr0acaymsfuwB\n" \
"lGMITfTE7WJQ715WlUyypLMIGI9q4gJEaqyAquuaVpy8r+xuZOh5oDyJLybsNPYV\n" \
"GKjYtLKRjKCkaaYRotFuYD164HO+6CjTk5cse71Ce2sw5qJr1J6zdsNtfasL\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n" \
"WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
"RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
"AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n" \
"R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n" \
"sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n" \
"NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n" \
"Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n" \
"/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n" \
"Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n" \
"FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n" \
"AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n" \
"Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n" \
"gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n" \
"PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n" \
"ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n" \
"CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n" \
"lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n" \
"avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n" \
"yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n" \
"yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n" \
"hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n" \
"HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n" \
"MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n" \
"nLRbwHOoq7hHwg==\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

static const uint32_t akNanoRestApi_ROOT_CERTIFICATE_PEM_LEN = \
    sizeof( akNanoRestApi_ROOT_CERTIFICATE_PEM );


static char bodyBuffer[1000];

static BaseType_t prvConnectToRestApiServer( NetworkContext_t * pxNetworkContext )
{
    // LogInfo( ( "prvConnectToDevicesGateway: BEGIN") );
    // vTaskDelay( pdMS_TO_TICKS( 100 ) );

    ServerInfo_t xServerInfo = { 0 };
    SocketsConfig_t xSocketsConfig = { 0 };
    BaseType_t xStatus = pdPASS;
    TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_SUCCESS;

    /* Initializer server information. */
    xServerInfo.pHostName = AKNANO_REST_API_ENDPOINT;
    xServerInfo.hostNameLength = AKNANO_REST_API_ENDPOINT_LEN;
    xServerInfo.port = AKNANO_REST_API_PORT;

    /* Configure credentials for TLS mutual authenticated session. */
    xSocketsConfig.enableTls = true;
    xSocketsConfig.pAlpnProtos = NULL;
    xSocketsConfig.maxFragmentLength = 0;
    xSocketsConfig.disableSni = false;
    xSocketsConfig.pRootCa = akNanoRestApi_ROOT_CERTIFICATE_PEM;
    xSocketsConfig.rootCaSize = akNanoRestApi_ROOT_CERTIFICATE_PEM_LEN;
    xSocketsConfig.sendTimeoutMs = 3000;
    xSocketsConfig.recvTimeoutMs = 3000;

    /* Establish a TLS session with the HTTP server. This example connects to
     * the HTTP server as specified in democonfigAWS_IOT_ENDPOINT and
     * democonfigAWS_HTTP_PORT in http_demo_mutual_auth_config.h. */
    LogInfo( ( "Establishing a TLS session to %.*s:%d.",
               ( int ) xServerInfo.hostNameLength,
               xServerInfo.pHostName,
               xServerInfo.port ) );

    vTaskDelay( pdMS_TO_TICKS( 100 ) );

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = SecureSocketsTransport_Connect( pxNetworkContext,
                                                     &xServerInfo,
                                                     &xSocketsConfig );

    LogInfo( ( "SecureSocketsTransport_Connect result = %d", xNetworkStatus) );

    vTaskDelay( pdMS_TO_TICKS( 100 ) );

    if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
    {
        LogError(("Error connecting to device gateway. result=%d", xNetworkStatus));
        xStatus = pdFAIL;
    }

    LogInfo(("TLS session to device gatweway established"));
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

    pxResponse->getTime = AkNanoGetTime; //nondet_boot();// ? NULL : GetCurrentTimeStub;

    /* Initialize the request object. */
    xRequestInfo.pHost = AKNANO_REST_API_ENDPOINT;
    xRequestInfo.hostLen = AKNANO_REST_API_ENDPOINT_LEN;
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
    HTTPClient_AddHeader(&xRequestHeaders, "OSF-TOKEN", strlen("OSF-TOKEN"), aknano_settings->token, strlen(aknano_settings->token));
    HTTPClient_AddHeader(&xRequestHeaders, "Content-Type", strlen("Content-Type"), "application/json", strlen("application/json"));

    if( xHTTPStatus == HTTPSuccess )
    {
        /* Initialize the response object. The same buffer used for storing
         * request headers is reused here. */
        pxResponse->pBuffer = ucUserBuffer;
        pxResponse->bufferLen = democonfigUSER_BUFFER_LENGTH;

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
                   ( int ) AKNANO_REST_API_ENDPOINT_LEN, AKNANO_REST_API_ENDPOINT,
                   ( int ) xRequestInfo.pathLen, xRequestInfo.pPath, pxResponse->statusCode ) );
        LogDebug( ( "Response Headers:\n%.*s\n",
                    ( int ) pxResponse->headersLen, pxResponse->pHeaders ) );
        // LogInfo( ( "Status Code: %u",
        //             pxResponse->statusCode ) );
        LogInfo( ( "Response Body:\n%.*s",
                    ( int ) pxResponse->bodyLen, pxResponse->pBody ) );
    }
    else
    {
        LogError( ( "Failed to send HTTP %.*s request to %.*s%.*s: Error=%s.",
                    ( int ) xRequestInfo.methodLen, xRequestInfo.pMethod,
                    ( int ) AKNANO_REST_API_ENDPOINT_LEN, AKNANO_REST_API_ENDPOINT,
                    ( int ) xRequestInfo.pathLen, xRequestInfo.pPath,
                    HTTPClient_strerror( xHTTPStatus ) ) );
    }

    /* TODO: Improve verification of registration error codes */
    if( xHTTPStatus != HTTPSuccess || pxResponse->statusCode == 403)
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}

bool AkNanoRegisterDevice(struct aknano_settings *aknano_settings)
{
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    TransportSocketStatus_t xNetworkStatus;
    // BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;

    LogInfo(("AkNanoRegisterDevice BEGIN"));
    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;
    bool ret = FALSE;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToRestApiServer(&xNetworkContext);

    // LogInfo(("prvConnectToServer Result: %d", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        /* Set a flag indicating that a TLS connection exists. */
        // xIsConnectionEstablished = pdTRUE;

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
        char pem_to_send[1024];
        memset(pem_to_send, 0, 1024);
        char *s = &pem_to_send[0];
        char *p = aknano_settings->device_certificate;

        for (int i = 0; i < 1024;)
        {
            if (*p == '\0')
                break;

            if (*p != '\n' && *p != '\r')
            {
                *s = *p;
                // printk("%c", *s);
                // if (i % 100 == 0)
                // 	printk("\n");
                s++;
                i++;
            }
            else
            {
                *s = '\\';
                s++;
                *s = 'n';
                s++;
                i++;
            }

            p++;
        }

        snprintf(bodyBuffer, sizeof(bodyBuffer),
                "{"
                "\"client.pem\": \"%s\",\n"
                "\"name\": \"%s\""
                "}",
                pem_to_send, aknano_settings->device_name);

        LogInfo(("** bodyBuffer=%s", bodyBuffer));
        ret = prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_PUT,
                            "/ota/devices/",
                            bodyBuffer, strlen(bodyBuffer),
                            &xResponse, aknano_settings);
    }

    /* Close the network connection.  */
    xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );
    if (xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS)
        LogError(("AkNanoRegisterDevice Disconnection error: %d", xNetworkStatus));

    LogInfo(("AkNanoRegisterDevice END ret=%d", ret));

    return ret;
}

#endif
