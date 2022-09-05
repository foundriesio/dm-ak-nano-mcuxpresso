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
#include <stdio.h>

#include "aknano_priv.h"
#include "aknano_secret.h"
#include "libtufnano.h"

#define AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN ( ( uint16_t ) ( sizeof( AKNANO_DEVICE_GATEWAY_ENDPOINT ) - 1 ) )

static const uint32_t akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN = sizeof( AKNANO_DEVICE_GATEWAY_CERTIFICATE );

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
    xSocketsConfig.pRootCa = AKNANO_DEVICE_GATEWAY_CERTIFICATE;
    xSocketsConfig.rootCaSize = akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN;
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
                   ( int ) AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN, AKNANO_DEVICE_GATEWAY_ENDPOINT,
                   ( int ) xRequestInfo.pathLen, xRequestInfo.pPath, pxResponse->statusCode ) );
        LogDebug( ( "Response Headers:\n%.*s\n",
                    ( int ) pxResponse->headersLen, pxResponse->pHeaders ) );
        // LogInfo( ( "Status Code: %u",
        //             pxResponse->statusCode ) );
        LogDebug( ( "Response Body:\n%.*s",
                    ( int ) pxResponse->bodyLen, pxResponse->pBody ) );
    }
    else
    {
        LogError( ( "Failed to send HTTP %.*s request to %.*s%.*s: Error=%s.",
                    ( int ) xRequestInfo.methodLen, xRequestInfo.pMethod,
                    ( int ) AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN, AKNANO_DEVICE_GATEWAY_ENDPOINT,
                    ( int ) xRequestInfo.pathLen, xRequestInfo.pPath,
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
void UpdateSettingValue(const char*, int);

static void parse_config(char* config_data, int buffer_len, struct aknano_settings *aknano_settings)
{
    JSONStatus_t result = JSON_Validate( config_data, buffer_len );
    char * value;
    unsigned int valueLength;
    int int_value;


    // {"polling_interval":{"Value":"10","Unencrypted":true},"tag":{"Value":"devel","Unencrypted":true},"z-50-fioctl.toml":{"Value":"\n[pacman]\n  tags = \"devel\"\n","Unencrypted":true,"OnChanged":["/usr/share/fioconfig/handlers/aktualizr-toml-update"]}}"
    if( result == JSONSuccess )
    {
        result = JSON_Search( config_data, buffer_len,
                             "tag" TUF_JSON_QUERY_KEY_SEPARATOR "Value", strlen("tag" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                              &value, &valueLength );

        if( result == JSONSuccess ) {
            if (strncmp(value, aknano_settings->tag, valueLength)) {
                LogInfo(("parse_config_data: Tag has changed ('%s' => '%.*s')",
                        aknano_settings->tag,
                        valueLength, value));
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
                             "polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value", strlen("polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                              &value, &valueLength );

        if( result == JSONSuccess ) {
            if (sscanf(value, "%d", &int_value) <= 0) {
                LogWarn(("Invalid polling_interval '%s'", value));
                // return;
            }

            if (int_value != aknano_settings->polling_interval) {
                LogInfo(("parse_config_data: Polling interval has changed (%d => %d)",
                        aknano_settings->polling_interval, int_value));
                aknano_settings->polling_interval = int_value;
                // aknano_write_to_nvs(AKNANO_NVS_ID_POLLING_INTERVAL,
                //         &aknano_settings->polling_interval,
                //         sizeof(aknano_settings->polling_interval));
            } else {
                // LogInfo(("parse_config_data: Polling interval is unchanged (%d)",
                //         aknano_settings->polling_interval));
            }
        } else {
            LogInfo(("parse_config_data: polling_interval config not found"));
        }

        result = JSON_Search( config_data, buffer_len,
                             "btn_polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value", strlen("btn_polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                              &value, &valueLength );

        if( result == JSONSuccess ) {
            if (sscanf(value, "%d", &int_value) <= 0) {
                LogWarn(("Invalid btn_polling_interval '%s'", value));
                // return;
            }
            UpdateSettingValue("btn_polling_interval", int_value);
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

time_t get_current_epoch(time_t boot_up_epoch)
{
    if (boot_up_epoch == 0)
        boot_up_epoch = 1637778974; // 2021-11-24

    return boot_up_epoch + (xTaskGetTickCount() / 1000);
}

static void get_time_str(time_t boot_up_epoch, char *output)
{
    time_t current_epoch_sec = get_current_epoch(boot_up_epoch);
    struct tm *tm = gmtime(&current_epoch_sec);

    sprintf(output, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        0);

    LogInfo(("get_time_str: %s (boot_up_epoch=%lld)", output, boot_up_epoch));
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

#include "fsl_caam.h"

#define RNG_EXAMPLE_RANDOM_NUMBERS     (4U)
#define RNG_EXAMPLE_RANDOM_BYTES       (16U)
#define RNG_EXAMPLE_RANDOM_NUMBER_BITS (RNG_EXAMPLE_RANDOM_NUMBERS * 8U * sizeof(uint32_t))

static bool randomInitialized = false;
static CAAM_Type *base = CAAM;
static caam_handle_t caamHandle;
static caam_config_t caamConfig;

/*! @brief CAAM job ring interface 0 in system memory. */
static caam_job_ring_interface_t s_jrif0;
/*! @brief CAAM job ring interface 1 in system memory. */
static caam_job_ring_interface_t s_jrif1;
/*! @brief CAAM job ring interface 2 in system memory. */
static caam_job_ring_interface_t s_jrif2;
/*! @brief CAAM job ring interface 3 in system memory. */
static caam_job_ring_interface_t s_jrif3;


void initRandom() {
    /* Get default configuration. */
    CAAM_GetDefaultConfig(&caamConfig);

    /* setup memory for job ring interfaces. Can be in system memory or CAAM's secure memory.
     * Although this driver example only uses job ring interface 0, example setup for job ring interface 1 is also
     * shown.
     */
    caamConfig.jobRingInterface[0] = &s_jrif0;
    caamConfig.jobRingInterface[1] = &s_jrif1;
    caamConfig.jobRingInterface[2] = &s_jrif2;
    caamConfig.jobRingInterface[3] = &s_jrif3;

    /* Init CAAM driver, including CAAM's internal RNG */
    if (CAAM_Init(base, &caamConfig) != kStatus_Success)
    {
        /* Make sure that RNG is not already instantiated (reset otherwise) */
        LogError(("- failed to init CAAM&RNG!"));
    }

    /* in this driver example, requests for CAAM jobs use job ring 0 */
    LogInfo(("*CAAM Job Ring 0* :"));
    caamHandle.jobRing = kCAAM_JobRing0;

}

status_t AkNanoGenRandomBytes(char *output, size_t size)
{
    if (!randomInitialized) 
    {
        initRandom();
        randomInitialized = true;
    }

    // status_t status; // = kStatus_Fail;
    // uint32_t number;
    // uint32_t data[RNG_EXAMPLE_RANDOM_NUMBERS] = {0};

    LogInfo(("RNG : "));

    LogInfo(("Generate %u-bit random number: ", RNG_EXAMPLE_RANDOM_NUMBER_BITS));
    // TODO: verify return code
    CAAM_RNG_GetRandomData(base, &caamHandle, kCAAM_RngStateHandle0, output, RNG_EXAMPLE_RANDOM_BYTES,
                                    kCAAM_RngDataAny, NULL);
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
            snprintf(details, sizeof(details), "Rollback to v%d after failed update to v%d.",
                old_version, aknano_settings->last_applied_version);
        } else {
            snprintf(details, sizeof(details), "Updated from v%d to v%d. Image confirmed. Running on %s slot.",
                old_version, new_version, aknano_settings->image_position == 1? "PRIMARY" : "SECONDARY");
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
    // LogInfo(("fill_event_payload: version=%d", version));
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
    LogInfo(("Event: %s %s %s", event_type, details, success_string));
    return true;
}

bool AkNanoSendEvent(struct aknano_settings *aknano_settings,
                    const char* event_type,
                    int version, int success)
{
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    TransportSocketStatus_t xNetworkStatus;
    // BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;

    if (!aknano_settings->is_device_registered) {
        LogInfo(("AkNanoSendEvent: Device is not registered. Skipping send of event %s", event_type));
        return TRUE;
    }

    LogInfo(("AkNanoSendEvent BEGIN %s", event_type));
    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToDevicesGateway(&xNetworkContext);

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
        fill_event_payload(bodyBuffer, aknano_settings, event_type, version, success);

        LogInfo((ANSI_COLOR_YELLOW "Sending %s event" ANSI_COLOR_RESET,
                event_type));
        LogInfo(("Event payload: %.80s (...)", bodyBuffer));

        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_POST,
                            "/events",
                            bodyBuffer, strlen(bodyBuffer),
                            &xResponse, aknano_settings);

    }

    /* Close the network connection.  */
    xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );
    if (xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS)
        LogError(("AkNanoSendEvent Disconnection error: %d", xNetworkStatus));

    LogInfo(("AkNanoSendEvent END %s", event_type));

    return TRUE;
}


uint8_t single_target_buffer[2000];


void AkNano_InitializeTransportInterface(TransportInterface_t *pTransportInterface,
                                        NetworkContext_t *pNetworkContext)
{
    /* Define the transport interface. */
    pTransportInterface->pNetworkContext = pNetworkContext;
    pTransportInterface->send = SecureSocketsTransport_Send;
    pTransportInterface->recv = SecureSocketsTransport_Recv;
}

BaseType_t AkNano_ConnectToDevicesGateway(NetworkContext_t *pNetworkContext,
                                        TransportInterface_t *pTransportInterface)
{
    BaseType_t xDemoStatus;
    
    xDemoStatus = prvConnectToDevicesGateway(pNetworkContext);
    // LogInfo(("prvConnectToServer Result: %d", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        AkNano_InitializeTransportInterface(pTransportInterface, pNetworkContext);
    }
    else
    {
        /* Log error to indicate connection failure after all
            * reconnect attempts are over. */
        LogError( ( "Failed to connect to HTTP server") );
    }
    return xDemoStatus;
}

BaseType_t AkNano_GetRootMetadata(TransportInterface_t *pTransportInterface,
                                struct aknano_settings *aknano_settings,
                                HTTPResponse_t *pResponse)
{
    return prvSendHttpRequest( pTransportInterface, HTTP_METHOD_GET,
                        "/repo/99999.root.json", "", 0,
                        pResponse, aknano_settings);
}

BaseType_t AkNano_GetTargets(TransportInterface_t *pTransportInterface,
                                struct aknano_settings *aknano_settings,
                                HTTPResponse_t *pResponse)
{
    return prvSendHttpRequest( pTransportInterface, HTTP_METHOD_GET,
                            "/repo/targets.json", "", 0,
                            pResponse, aknano_settings);
}


#define TUF_DATA_BUFFER_LEN 10 * 1024
static unsigned char tuf_data_buffer[TUF_DATA_BUFFER_LEN];
int parse_targets_metadata(const char *data, int len, void *application_context);

int AkNanoPoll(struct aknano_context *aknano_context)
{
    // TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    // HTTPResponse_t xResponse;
    BaseType_t xDemoStatus;
    bool isUpdateRequired = false;
    bool isRebootRequired = false;
    // off_t offset = 0;
    struct aknano_settings *aknano_settings = aknano_context->settings;

    LogInfo(("AkNanoPoll. Version=%lu  Tag=%s", aknano_settings->running_version, aknano_settings->tag));

    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = AkNano_ConnectToDevicesGateway(&xNetworkContext, &aknano_context->xTransportInterface);

    if( xDemoStatus == pdPASS )
    {
        xDemoStatus = prvSendHttpRequest( &aknano_context->xTransportInterface, HTTP_METHOD_GET,
                                          "/config", "", 0,
                                          &aknano_context->xResponse, aknano_context->settings);
        if (xDemoStatus == pdPASS)
            parse_config((char*)aknano_context->xResponse.pBody, aknano_context->xResponse.bodyLen, aknano_context->settings);

        
        time_t reference_time = get_current_epoch(aknano_settings->boot_up_epoch);
// #define TUF_FORCE_DATE_IN_FUTURE 1
#ifdef TUF_FORCE_DATE_IN_FUTURE
        LogInfo((ANSI_COLOR_RED "Forcing TUF reference date to be 1 year from now" ANSI_COLOR_RESET));
        reference_time += 31536000; // Add 1 year
#endif
        int tuf_ret = tuf_refresh(aknano_context, reference_time, tuf_data_buffer, sizeof(tuf_data_buffer)); /* TODO: Get epoch from system clock */
        LogInfo(("tuf_refresh %s (%d)", tuf_get_error_string(tuf_ret), tuf_ret));

        // AkNano_GetRootMetadata(&xTransportInterface, aknano_context->settings, &xResponse);

        // xDemoStatus = AkNano_GetTargets(&xTransportInterface, aknano_context->settings, &xResponse);
        if (tuf_ret == 0) {
            parse_targets_metadata(tuf_data_buffer, strlen(tuf_data_buffer), aknano_context);

            // aknano_handle_manifest_data(aknano_context, single_target_buffer, &offset, (uint8_t*)xResponse.pBody, xResponse.bodyLen);
            if (aknano_context->selected_target.version == 0) {
                LogInfo(("* No matching target found in manifest"));
            } else {
                LogInfo(("* Manifest data parsing result: highest version=%ld  last unconfirmed applied=%d  running version=%ld",
                    aknano_context->selected_target.version,
                    aknano_settings->last_applied_version,
                    aknano_settings->running_version));

                if (aknano_context->selected_target.version == aknano_settings->last_applied_version) {
                    LogInfo(("* Same version was already applied (and failed). Do not retrying it"));

                } else if (aknano_context->settings->running_version != aknano_context->selected_target.version) {
                    LogInfo((ANSI_COLOR_GREEN "* Update required: %lu -> %ld" ANSI_COLOR_RESET,
                            aknano_context->settings->running_version,
                            aknano_context->selected_target.version));
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
        prvSendHttpRequest( &aknano_context->xTransportInterface, HTTP_METHOD_PUT,
                            "/system_info", bodyBuffer, strlen(bodyBuffer),
                            &aknano_context->xResponse, aknano_context->settings);

        fill_network_info(bodyBuffer, sizeof(bodyBuffer));
        prvSendHttpRequest( &aknano_context->xTransportInterface, HTTP_METHOD_PUT,
                            "/system_info/network", bodyBuffer, strlen(bodyBuffer),
                            &aknano_context->xResponse, aknano_context->settings);

        // LogInfo(("aknano_settings->tag=%s",aknano_settings->tag));
        sprintf(bodyBuffer,
                "[aknano_settings]\n" \
                "poll_interval = %d\n" \
                "hw_id = \"%s\"\n" \
                "tag = \"%s\"\n" \
                "binary_compilation_local_time = \""__DATE__ " " __TIME__"\"",
                aknano_settings->polling_interval,
                CONFIG_BOARD,
                aknano_settings->tag);
        prvSendHttpRequest( &aknano_context->xTransportInterface, HTTP_METHOD_PUT,
                            "/system_info/config", bodyBuffer, strlen(bodyBuffer),
                            &aknano_context->xResponse, aknano_context->settings);

    }

    /* Close the network connection.  */
    SecureSocketsTransport_Disconnect( &xNetworkContext );


    if (isUpdateRequired) {

        // version = hb_context.selected_target.version;

        char unused_serial[AKNANO_MAX_SERIAL_LENGTH];
        /* Gen a random correlation ID for the update events */
        aknano_gen_serial_and_uuid(aknano_settings->ongoing_update_correlation_id,
                    unused_serial);


        // aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID,
        //             aknano_settings.ongoing_update_correlation_id,
        //             AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH);

        AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_STARTED,
                        aknano_context->selected_target.version,
                        AKNANO_EVENT_SUCCESS_UNDEFINED);
        if (AkNanoDownloadAndFlashImage(aknano_context)) {
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                            aknano_context->selected_target.version,
                            AKNANO_EVENT_SUCCESS_TRUE);
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_INSTALLATION_STARTED,
                            aknano_context->selected_target.version,
                            AKNANO_EVENT_SUCCESS_UNDEFINED);

            aknano_settings->last_applied_version = aknano_context->selected_target.version;
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_INSTALLATION_APPLIED,
                            aknano_context->selected_target.version,
                            AKNANO_EVENT_SUCCESS_TRUE);

            LogInfo(("Requesting update on next boot (ReadyForTest)"));
            enable_image_and_set_boot_image_position(aknano_settings->image_position);
            isRebootRequired = true;
        } else {
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                            aknano_context->selected_target.version,
                            AKNANO_EVENT_SUCCESS_FALSE);
        }

        AkNanoUpdateSettingsInFlash(aknano_settings);

        // TODO: Add download failed event
        // AkNanoSendEvent(aknano_context, AKNANO_EVENT_DOWNLOAD_COMPLETED, aknano_context->selected_target.version);
    }

    if (isRebootRequired) {
        LogInfo(("Rebooting...."));
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
        NVIC_SystemReset();
    }

    // LogInfo(("AkNanoPoll END"));

    return 0;
}

/* TUF "callbacks" */
int tuf_client_fetch_file(const char *file_base_name, unsigned char *target_buffer, size_t target_buffer_len, size_t *file_size, void *application_context)
{
    struct aknano_context *aknano_context = application_context;
    BaseType_t ret;

    snprintf(aknano_context->url_buffer, sizeof(aknano_context->url_buffer), "/repo/%s", file_base_name);
    ret = prvSendHttpRequest( &aknano_context->xTransportInterface, HTTP_METHOD_GET,
                    (char*)aknano_context->url_buffer, "", 0,
                    &aknano_context->xResponse, aknano_context->settings);

    if (ret == pdPASS) {
        LogInfo(("tuf_client_fetch_file: %s HTTP operation return code %d. Body length=%ld ", file_base_name, aknano_context->xResponse.statusCode, aknano_context->xResponse.bodyLen));
        if ((aknano_context->xResponse.statusCode / 100) == 2) {
            if (aknano_context->xResponse.bodyLen > target_buffer_len) {
                LogError(("tuf_client_fetch_file: %s retrieved file is too big. Maximum %ld, got %ld", file_base_name, target_buffer_len, aknano_context->xResponse.bodyLen));
                return TUF_ERROR_DATA_EXCEEDS_BUFFER_SIZE;
            }
            *file_size = aknano_context->xResponse.bodyLen;
            memcpy(target_buffer, aknano_context->xResponse.pBody, aknano_context->xResponse.bodyLen);
            target_buffer[aknano_context->xResponse.bodyLen] = '\0';
            return TUF_SUCCESS;
        } else {
            return -aknano_context->xResponse.statusCode;
        }
    } else {
        LogInfo(("tuf_client_fetch_file: %s HTTP operation failed", file_base_name));
        return -1;
    }
}

#define AKNANO_FLASH_OFF_TUF_ROLES (16 * 1024)
#define AKNANO_FLASH_OFF_TUF_ROLE_ROOT (AKNANO_FLASH_OFF_TUF_ROLES)
#define AKNANO_FLASH_OFF_TUF_ROLE_TIMESTAMP (AKNANO_FLASH_OFF_TUF_ROLE_ROOT + 4096)
#define AKNANO_FLASH_OFF_TUF_ROLE_SNAPSHOT (AKNANO_FLASH_OFF_TUF_ROLE_TIMESTAMP + 4096)
#define AKNANO_FLASH_OFF_TUF_ROLE_TARGETS (AKNANO_FLASH_OFF_TUF_ROLE_SNAPSHOT + 4096)


static int get_flash_offset_for_role(enum tuf_role role)
{
    switch(role) {
        case ROLE_ROOT: return AKNANO_FLASH_OFF_TUF_ROLE_ROOT;
        case ROLE_TIMESTAMP: return AKNANO_FLASH_OFF_TUF_ROLE_TIMESTAMP;
        case ROLE_SNAPSHOT: return AKNANO_FLASH_OFF_TUF_ROLE_SNAPSHOT;
        case ROLE_TARGETS: return AKNANO_FLASH_OFF_TUF_ROLE_TARGETS;
        default: return -1;
    }
}

const char *sample_root_json = "{\"signatures\":[{\"keyid\":\"91f89e098b6c3ee0878b9f1518c2f88624dad3301ed82f1c688310de952fce0c\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"H+IKxiUR8sVstCU4vTXLGXEZPHNawM8xiYoC8jQ+eTJsVoN7Ri4vs6h81cESdphFirht5q3TBH0gQopRGZasLUTgZi8Yl2IoikvK3CKZqW0ViZNwYLbSo7cxZZbBeheD8G5eRRztN+ErcEHJ5kZuySu5ektCZBGEOYxou7fs32zm5FV2HUFsJ0AEvVosidZkorj3RkwCWiSkn18lCXIElw8gfLrSnxDzr4ut6I3VLlbVZrkxvCJOWB1m98utqH2DbK+zHOq+vOrUuIsHyc+YgCUX+FTCW6s1pzC4SFUQJsuCX4+jOVM+GZamoaaISIR41HguurQ7cL4N7ISYrJNlwQ==\"},{\"keyid\":\"8e126cc4e3ed6eaf4b638216c856c12ae2d8fa36b0b646bad1c98581a3d8e0df\",\"method\":\"rsassa-pss-sha256\",\"sig\":\"Ene2DbCEA8t0RYjVDUxIuUhSe57ojtIfUAPIQEAd1d86XUILmGPgw0TDULi6nnWjQgiyAeHxWjLNZrAAc6gle00bvMTCYLFU8XLwk1TEh1vL+kI+wOeY7g7iqJBBJaAAhx9D4LE/tfT9caj9Pd1YQAh6IIKqr2vkDViPW3sVMNRhTrwG5UCe/FFW2V25VlkSVSWCdin2AsRfdjJh87BzGQcwvc0s3bZxvnGlGrb1W898LBK+v8u9Ze6c5oP4pwJ+z7Fzo7BkqOrtXumzIeDierzllqaxbCrEjFMvyas3ne6QtkAt9WAQhPx31O88OV4hUrg6YPiIuTPRKFqf0GghMQ==\"}],\"signed\":{\"_type\":\"Root\",\"consistent_snapshot\":false,\"expires\":\"2022-10-14T19:56:08Z\",\"keys\":{\"22e8f6d06eec2e4d50187b74ff21ddbc15326fb56e2d62fd68d10c0cb43b4e7e\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsac985OY9LESVVHaRJBU\\nV2i1uHCkocMKHihaUEFZbE1dv6EMwBrdM+Z0b2I4A6E6GXmAJE3FVrsXikatleOb\\n7yau+yzv2b4wiK/7OBgz61hPmVsK1k1QVK1f3v0J27Koa6YeVUbpisXCuTQrrA23\\nczuvZlW9tHtJHY3uD03MfwlcENr+gvppDxEHCUzoUvN16IHsnGGGdgL8q4uNelDq\\n3iJCz/ArhEWOkq613sLZbOq83TyYzVgw0lcxPJ1oX+NA4iAC2Pl/uRLUtVawefUc\\nZ5k4DgpouAy2ot9d4oGRjs2LDnmKFwFiMbLgMQf6nIK8PDcso1cA44UwZuC7xSFU\\n6QIDAQAB\\n-----END PUBLIC KEY-----\\n\"}},\"2e6d5e7f1469cb08be5f371ff5f211a45005223e5aff20815f36880c9bd418cb\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAstQmVmyZIA2tejyNDkug\\npUsncb2cBGl/vGkrsDJQvxTPpNnmtRQTdQoZ/qqPWonNZ/JEJRVL1xc24ioyx/dS\\nCKvdZzAsIxSoklxoDslSP8jDKwdFLj34snGBgtdDJ+bh44Oei6532GX5iy7Xj3SE\\na5pVoQ6nLWz5AULw7gmR01qIA3J1OZ7oVhR5hF4W/gNc8hAQg1gMRSrm+PUxzRr2\\n5YfZznE9JVsvuTi/e0iMDBeE1cXlUzo1/B2b+7072xlBsGP61tuln6c6kRA7PbIg\\nHX5Q+vs7svBY3wl07an8IxvHi9iZUYW9V8HH67/jJxree04kjC2KhaozEJLITwE0\\n4QIDAQAB\\n-----END PUBLIC KEY-----\\n\"}},\"91f89e098b6c3ee0878b9f1518c2f88624dad3301ed82f1c688310de952fce0c\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmu+MksWTfMScaFw4KUBk\\nwKcSeROX8atN4D8r42BHgCsLr4OcXmHzkDVSuCCymJ2SEkgnd6pJxIaWs+HS0Nni\\nu3Gxqv9+6ZUKiMzG89gFkx6kU4RZRd3TcMZOTZaabWhDuVpg6Gkig759qL6B/jNi\\nK1FBAKNGPp3S0rZ+zghdrvrKzUSlVLmvOqTI0PhddkzoNGDO9v6F40n58NKvlOUY\\nCn8wk1n8DGG36CActHIjoAUoQsueBTRNdUy5vNmX4BuEdhUdwDaaJwEkvIvoU3S/\\nwLNlSexU5EJjqWlNeUEWvUJjbxXpSMqAhTtT1MG5En+yqPhH1tGuzK3w6JCS9aou\\nvQIDAQAB\\n-----END PUBLIC KEY-----\\n\"}},\"c0919e0f82b94be9eef55dcd5f224d8d8b4da80299a4ebef58018ab68fee0a8d\":{\"keytype\":\"RSA\",\"keyval\":{\"public\":\"-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyX9EqnICo4ewErN82S00\\nWmCPhEfoEDwpG593Kh1E04MDC+PB3YIdkcqFTSeMkZW25GgWclLyDEAuUmGEO6yN\\nrR1S4/e36ImiIW2vEZhlMIX8a/SEb11Rhkvi/GRpt/ZyLp1El4M17nYO/GnYr1dE\\nhn2PCaZIo3jVuxmZ/+nzY/9whanjKoomGRcGlAne927DHYV0XiI1dBt+4gvpMdXp\\nEH8ZCHPQHjsUe0S433niVMIa4pIaQWvdEFliJ703Dqbxn1iVHUi+p8j+z6oh2v1z\\nZz0aiECwKOtRsGUNtJWeZfx5ZOOoeKyE47DZXPsolw7DlJxZRZlGny1KpX8Y0yTA\\n7QIDAQAB\\n-----END PUBLIC KEY-----\\n\"}}},\"roles\":{\"root\":{\"keyids\":[\"91f89e098b6c3ee0878b9f1518c2f88624dad3301ed82f1c688310de952fce0c\"],\"threshold\":1},\"snapshot\":{\"keyids\":[\"2e6d5e7f1469cb08be5f371ff5f211a45005223e5aff20815f36880c9bd418cb\"],\"threshold\":1},\"targets\":{\"keyids\":[\"22e8f6d06eec2e4d50187b74ff21ddbc15326fb56e2d62fd68d10c0cb43b4e7e\"],\"threshold\":1},\"timestamp\":{\"keyids\":[\"c0919e0f82b94be9eef55dcd5f224d8d8b4da80299a4ebef58018ab68fee0a8d\"],\"threshold\":1}},\"version\":2}}";

int tuf_client_read_local_file(enum tuf_role role, unsigned char *target_buffer, size_t target_buffer_len, size_t *file_size, void *application_context)
{
    int ret;
    int initial_offset;
    
    if (role == ROLE_ROOT) {
        strncpy(target_buffer, sample_root_json, target_buffer_len);
        target_buffer[target_buffer_len-1] = 0;
        *file_size = strnlen((char*)target_buffer, target_buffer_len);
        LogInfo(("tuf_client_read_local_file: role=%s file_size=%d strlen=%d OK [HARDCODED TEST FILE]", tuf_get_role_name(role), *file_size, strlen(target_buffer)));
        vTaskDelay( pdMS_TO_TICKS( 20 ) );
        return TUF_SUCCESS;
    }

    initial_offset = get_flash_offset_for_role(role);
    if (initial_offset < 0) {
        LogInfo(("tuf_client_read_local_file: role=%s error reading flash", tuf_get_role_name(role)));
        return -1;
    }

    ret = ReadFlashStorage(initial_offset, target_buffer, target_buffer_len);
    if (ret < 0)
        return ret;
    if (target_buffer[0] != '{') {
        LogInfo(("tuf_client_read_local_file: role=%s file not found. buf[0]=%X", tuf_get_role_name(role), target_buffer[0]));
        return -1; // File not found
    }

    for (int i=0; i<target_buffer_len; i++) {
        if (target_buffer[i] == 0xFF) {
            target_buffer[i] = 0;
            break;
        }
    }
    target_buffer[target_buffer_len-1] = 0;
    *file_size = strnlen((char*)target_buffer, target_buffer_len);
    LogInfo(("tuf_client_read_local_file: role=%s file_size=%d strlen=%d OK", tuf_get_role_name(role), *file_size, strlen(target_buffer)));
	return TUF_SUCCESS;
}

status_t WriteDataToFlash(int offset, void *data, size_t data_len);

int tuf_client_write_local_file(enum tuf_role role, const unsigned char *data, size_t len, void *application_context)
{
    int i;
    int initial_offset;
    status_t ret;
    
    initial_offset = get_flash_offset_for_role(role);
    // LogInfo(("write_local_file: role=%d initial_offset=%d len=%d", role, initial_offset, len));
    ret = WriteDataToFlash(initial_offset, data, len);
    LogInfo(("tuf_client_write_local_file: role=%s len=%d %s", tuf_get_role_name(role), len, ret? "ERROR" : "OK"));
    // vTaskDelay( pdMS_TO_TICKS( 20 ) );

    return ret;
}
