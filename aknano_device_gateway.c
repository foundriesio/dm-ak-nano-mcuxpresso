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
               ( int32_t ) xServerInfo.hostNameLength,
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
                             "tag.Value", strlen("tag.Value"),
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
                             "polling_interval.Value", strlen("polling_interval.Value"),
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
                             "btn_polling_interval.Value", strlen("btn_polling_interval.Value"),
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

    status_t status = kStatus_Fail;
    uint32_t number;
    uint32_t data[RNG_EXAMPLE_RANDOM_NUMBERS] = {0};

    LogInfo(("RNG : "));

    LogInfo(("Generate %u-bit random number: ", RNG_EXAMPLE_RANDOM_NUMBER_BITS));
    status = CAAM_RNG_GetRandomData(base, &caamHandle, kCAAM_RngStateHandle0, output, RNG_EXAMPLE_RANDOM_BYTES,
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

int AkNanoPoll(struct aknano_context *aknano_context)
{
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;
    BaseType_t xDemoStatus;
    bool isUpdateRequired = false;
    bool isRebootRequired = false;
    off_t offset = 0;
    struct aknano_settings *aknano_settings = aknano_context->settings;

    LogInfo(("AkNanoPoll. Version=%u  Tag=%s", aknano_settings->running_version, aknano_settings->tag));

    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = AkNano_ConnectToDevicesGateway(&xNetworkContext, &xTransportInterface);

    if( xDemoStatus == pdPASS )
    {
        xDemoStatus = prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_GET,
                                          "/config", "", 0,
                                          &xResponse, aknano_context->settings);
        if (xDemoStatus == pdPASS)
            parse_config((char*)xResponse.pBody, xResponse.bodyLen, aknano_context->settings);

        AkNano_GetRootMetadata(&xTransportInterface, aknano_context->settings, &xResponse);

        xDemoStatus = AkNano_GetTargets(&xTransportInterface, aknano_context->settings, &xResponse);
        if (xDemoStatus == pdPASS) {
            aknano_handle_manifest_data(aknano_context, single_target_buffer, &offset, (uint8_t*)xResponse.pBody, xResponse.bodyLen);
            if (aknano_context->aknano_json_data.selected_target.version == 0) {
                LogInfo(("* No matching target found in manifest"));
            } else {
                LogInfo(("* Manifest data parsing result: highest version=%u  last unconfirmed applied=%d  running version=%d",
                    aknano_context->aknano_json_data.selected_target.version,
                    aknano_settings->last_applied_version,
                    aknano_settings->running_version));

                if (aknano_context->aknano_json_data.selected_target.version == aknano_settings->last_applied_version) {
                    LogInfo(("* Same version was already applied (and failed). Do not retrying it"));

                } else if (aknano_context->settings->running_version != aknano_context->aknano_json_data.selected_target.version) {
                    LogInfo((ANSI_COLOR_GREEN "* Update required: %u -> %u" ANSI_COLOR_RESET,
                            aknano_context->settings->running_version,
                            aknano_context->aknano_json_data.selected_target.version));
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
                            "/system_info", bodyBuffer, strlen(bodyBuffer),
                            &xResponse, aknano_context->settings);

        fill_network_info(bodyBuffer, sizeof(bodyBuffer));
        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_PUT,
                            "/system_info/network", bodyBuffer, strlen(bodyBuffer),
                            &xResponse, aknano_context->settings);

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
        prvSendHttpRequest( &xTransportInterface, HTTP_METHOD_PUT,
                            "/system_info/config", bodyBuffer, strlen(bodyBuffer),
                            &xResponse, aknano_context->settings);

    }

    /* Close the network connection.  */
    SecureSocketsTransport_Disconnect( &xNetworkContext );


    if (isUpdateRequired) {

        // version = hb_context.aknano_json_data.selected_target.version;

        char unused_serial[AKNANO_MAX_SERIAL_LENGTH];
        /* Gen a random correlation ID for the update events */
        aknano_gen_serial_and_uuid(aknano_settings->ongoing_update_correlation_id,
                    unused_serial);


        // aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID,
        //             aknano_settings.ongoing_update_correlation_id,
        //             AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH);

        AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_STARTED,
                        aknano_context->aknano_json_data.selected_target.version,
                        AKNANO_EVENT_SUCCESS_UNDEFINED);
        if (AkNanoDownloadAndFlashImage(aknano_context)) {
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                            aknano_context->aknano_json_data.selected_target.version,
                            AKNANO_EVENT_SUCCESS_TRUE);
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_INSTALLATION_STARTED,
                            aknano_context->aknano_json_data.selected_target.version,
                            AKNANO_EVENT_SUCCESS_UNDEFINED);

            aknano_settings->last_applied_version = aknano_context->aknano_json_data.selected_target.version;
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_INSTALLATION_APPLIED,
                            aknano_context->aknano_json_data.selected_target.version,
                            AKNANO_EVENT_SUCCESS_TRUE);

            LogInfo(("Requesting update on next boot (ReadyForTest)"));
            enable_image_and_set_boot_image_position(aknano_settings->image_position);
            isRebootRequired = true;
        } else {
            AkNanoSendEvent(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                            aknano_context->aknano_json_data.selected_target.version,
                            AKNANO_EVENT_SUCCESS_FALSE);
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

    // LogInfo(("AkNanoPoll END"));

    return 0;
}
