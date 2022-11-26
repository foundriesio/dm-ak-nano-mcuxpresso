/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_LEVEL LOG_INFO
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#include "aknano_priv.h"
#include "aknano_client.h"
#include "aknano_secret.h"
#include "libtufnano.h"

#define AKNANO_DEVICE_GATEWAY_ENDPOINT AKNANO_FACTORY_UUID ".ota-lite.foundries.io"

#define AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN ( ( uint16_t ) ( sizeof( AKNANO_DEVICE_GATEWAY_ENDPOINT ) - 1 ) )

static const uint32_t akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN = sizeof( AKNANO_DEVICE_GATEWAY_CERTIFICATE );

static char bodyBuffer[500];

static void fill_network_info(char* output, size_t max_length)
{
    char ipv4[4];
    uint8_t mac[6];

    aknano_get_ipv4_and_mac(ipv4, mac);

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

void UpdateSettingValue(const char*, int);

static void parse_config(const char* config_data, int buffer_len, struct aknano_settings *aknano_settings)
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


// static void get_pseudo_time_str(time_t boot_up_epoch, char *output, const char* event_type, int success)
// {
//     int event_delta = 0;
//     int base_delta = 180;

//     if (boot_up_epoch == 0)
//         boot_up_epoch = BUILD_EPOCH_MS / 1000;


//     if (!strcmp(event_type, AKNANO_EVENT_DOWNLOAD_STARTED)) {
//         event_delta = 1;
//     } else if (!strcmp(event_type, AKNANO_EVENT_DOWNLOAD_COMPLETED)) {
//         event_delta = 20;
//     } else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_STARTED)) {
//         event_delta = 21;
//     } else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_APPLIED)) {
//         event_delta = 22;
//     } else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_COMPLETED)) {
//         if (success == AKNANO_EVENT_SUCCESS_TRUE)
//             event_delta = 42;
//         else
//             event_delta = 242;
//     }

//     // time_t current_epoch_ms = boot_up_epoch_ms + base_delta + event_delta;
//     time_t current_epoch_sec = boot_up_epoch + base_delta + event_delta;
//     struct tm *tm = gmtime(&current_epoch_sec);

//     sprintf(output, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
//         tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
//         tm->tm_hour, tm->tm_min, tm->tm_sec,
//         0);

//     LogInfo(("get_pseudo_time_str: %s", output));
// }


static void get_time_str(time_t boot_up_epoch, char *output)
{
    time_t current_epoch_sec = get_current_epoch(boot_up_epoch);
    struct tm *tm = gmtime(&current_epoch_sec);

    sprintf(output, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        0);

    // LogInfo(("get_time_str: %s (boot_up_epoch=%lld)", output, boot_up_epoch));
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


int aknano_gen_serial_and_uuid(char *uuid_string, char *serial_string)
{
    char serial_bytes[16];
    AkNanoGenRandomBytes(serial_bytes, sizeof(serial_bytes));
    btox(serial_string, serial_bytes, sizeof(serial_bytes) * 2);
    serial_string_to_uuid_string(serial_string, uuid_string);
    uuid_string[36] = '\0';
    serial_string[32] = '\0';
    // LogInfo(("uuid='%s', serial='%s'", uuid_string, serial_string));
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
    char target[sizeof(aknano_settings->hwid)+15];
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
    snprintf(target, sizeof(target), "%s-v%d", aknano_settings->hwid, new_version);

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

    // if (aknano_settings->boot_up_epoch)
        get_time_str(aknano_settings->boot_up_epoch, current_time_str);
    // else
    //     get_pseudo_time_str(aknano_settings->boot_up_epoch, current_time_str, event_type, success);

    LogInfo(("fill_event_payload: time=%s cor_id=%s uuid=%s", current_time_str, correlation_id, evt_uuid));

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

BaseType_t AkNano_ConnectToDevicesGateway(struct aknano_network_context *network_context)
{
    BaseType_t ret;

    init_network_context(network_context);
    ret = aknano_mtls_connect(network_context,
                                        AKNANO_DEVICE_GATEWAY_ENDPOINT,
                                        AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN,
                                        AKNANO_DEVICE_GATEWAY_PORT,
                                        AKNANO_DEVICE_GATEWAY_CERTIFICATE,
                                        akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN);

    // LogInfo(("prvConnectToServer Result: %d", xDemoStatus));
    if(ret != pdPASS)
    {
        /* Log error to indicate connection failure after all
            * reconnect attempts are over. */
        LogError( ( "Failed to connect to HTTP server") );
        return pdFAIL;
    }
    return pdPASS;
}

BaseType_t AkNano_SendHttpRequest( struct aknano_network_context *network_context,
                                      const char * pcMethod,
                                      const char * pcPath,
                                      const char * pcBody,
                                      size_t xBodyLen,
                                      struct aknano_settings *aknano_settings
                                      )
{
    char *tag = aknano_settings->tag;
    int version = aknano_settings->running_version;

    char active_target[200];
    snprintf(active_target, sizeof(active_target), "%s-v%d", aknano_settings->hwid, version);

    const char *header_keys[] = { "x-ats-tags", "x-ats-target" };
    const char *header_values[] = { tag, active_target };

    BaseType_t ret = aknano_mtls_send_http_request(
        network_context,
        AKNANO_DEVICE_GATEWAY_ENDPOINT,
        AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN,
        pcMethod,
        pcPath,
        pcBody,
        xBodyLen,
        ucUserBuffer,
        democonfigUSER_BUFFER_LENGTH,
        header_keys,
        header_values,
        2);

    return ret;
}


bool AkNanoSendEvent(struct aknano_settings *aknano_settings,
                    const char* event_type,
                    int version, int success)
{
    struct aknano_network_context network_context;

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    if (!aknano_settings->is_device_registered) {
        LogInfo(("AkNanoSendEvent: Device is not registered. Skipping send of event %s", event_type));
        return TRUE;
    }
#endif AKNANO_ENABLE_EXPLICIT_REGISTRATION

    BaseType_t xDemoStatus = pdPASS;

    xDemoStatus = AkNano_ConnectToDevicesGateway(&network_context);
    if (xDemoStatus != pdPASS)
        return TRUE;

    fill_event_payload(bodyBuffer, aknano_settings, event_type, version, success);

    LogInfo((ANSI_COLOR_YELLOW "Sending %s event" ANSI_COLOR_RESET,
            event_type));
    LogInfo(("Event payload: %.80s (...)", bodyBuffer));

    AkNano_SendHttpRequest(
        &network_context,
        HTTP_METHOD_POST,
        "/events",
        bodyBuffer,
        strlen(bodyBuffer),
        aknano_settings);


    /* Close the network connection.  */
    aknano_mtls_disconnect(&network_context);
    return TRUE;
}

#define TUF_DATA_BUFFER_LEN 10 * 1024
static unsigned char tuf_data_buffer[TUF_DATA_BUFFER_LEN];
int parse_targets_metadata(const char *data, int len, void *application_context);

int AkNanoPoll(struct aknano_context *aknano_context)
{
    struct aknano_network_context network_context;
    BaseType_t xDemoStatus;
    bool isUpdateRequired = false;
    bool isRebootRequired = false;
    // off_t offset = 0;
    struct aknano_settings *aknano_settings = aknano_context->settings;

    LogInfo(("AkNanoPoll. Version=%lu  Tag=%s", aknano_settings->running_version, aknano_settings->tag));

    xDemoStatus = AkNano_ConnectToDevicesGateway(&network_context);
    if( xDemoStatus == pdPASS )
    {
        xDemoStatus =  AkNano_SendHttpRequest(
                                        &network_context,
                                        HTTP_METHOD_GET,
                                        "/config", "", 0,
                                        aknano_context->settings);
        if (xDemoStatus == pdPASS) {
            if (network_context.reply_body == NULL)
                LogInfo(("Device has no config set"));
            else
                parse_config((const char*)network_context.reply_body, network_context.reply_body_len, aknano_context->settings);
        }

        time_t reference_time = get_current_epoch(aknano_settings->boot_up_epoch);
// #define TUF_FORCE_DATE_IN_FUTURE 1
#ifdef TUF_FORCE_DATE_IN_FUTURE
        LogInfo((ANSI_COLOR_RED "Forcing TUF reference date to be 1 year from now" ANSI_COLOR_RESET));
        reference_time += 31536000; // Add 1 year
#endif
        aknano_context->dg_network_context = &network_context;
        int tuf_ret = tuf_refresh(aknano_context, reference_time, tuf_data_buffer, sizeof(tuf_data_buffer)); /* TODO: Get epoch from system clock */
        LogInfo((ANSI_COLOR_MAGENTA "tuf_refresh %s (%d)" ANSI_COLOR_RESET, tuf_get_error_string(tuf_ret), tuf_ret));

        // AkNano_GetRootMetadata(&xTransportInterface, aknano_context->settings, &xResponse);

        // xDemoStatus = AkNano_GetTargets(&xTransportInterface, aknano_context->settings, &xResponse);
        if (tuf_ret == 0) {
            parse_targets_metadata((const char*)tuf_data_buffer, strlen((char*)tuf_data_buffer), aknano_context);

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

                } else if (aknano_context->settings->running_version < aknano_context->selected_target.version) {
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

        AkNano_SendHttpRequest(&network_context, HTTP_METHOD_PUT,
                            "/system_info", bodyBuffer, strlen(bodyBuffer),
                            aknano_context->settings);

        fill_network_info(bodyBuffer, sizeof(bodyBuffer));
        AkNano_SendHttpRequest( &network_context, HTTP_METHOD_PUT,
                            "/system_info/network", bodyBuffer, strlen(bodyBuffer),
                            aknano_context->settings);

        // LogInfo(("aknano_settings->tag=%s",aknano_settings->tag));
        sprintf(bodyBuffer,
                "[aknano_settings]\n" \
                "poll_interval = %d\n" \
                "hw_id = \"%s\"\n" \
                "tag = \"%s\"\n" \
                "binary_compilation_local_time = \""__DATE__ " " __TIME__"\"\n"
                "provisioning_mode = \"" AKNANO_PROVISIONING_MODE "\"",
                aknano_settings->polling_interval,
                CONFIG_BOARD,
                aknano_settings->tag);
        AkNano_SendHttpRequest( &network_context, HTTP_METHOD_PUT,
                            "/system_info/config", bodyBuffer, strlen(bodyBuffer),
                            aknano_context->settings);

    }

    /* Close the network connection.  */
    aknano_mtls_disconnect(&network_context);


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
            status_t status;
            status = bl_update_image_state(kSwapType_ReadyForTest);
            if (status != kStatus_Success) {
                LogWarn(("Error setting image as ReadyForTest. status=%d", status));
            } else {
                isRebootRequired = true;
            }
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
