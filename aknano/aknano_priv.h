/*
 * Copyright 2021-2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PRIV_H__
#define __AKNANO_PRIV_H__

#include "mbedtls/sha256.h"
#include "core_pkcs11_config.h"
#include "core_pkcs11.h"
#include "pkcs11t.h"
#include "board.h"

#include "fsl_common.h"
#include <time.h>

#include "mcuboot_app_support.h"
/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

#include "fsl_flexspi.h"
#include "aws_demo_config.h"
#include "aws_dev_mode_key_provisioning.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "core_json.h"

#include "iot_network.h"

/* Include PKCS11 helper for random number generation. */
#include "pkcs11_helpers.h"

/*Include backoff algorithm header for retry logic.*/
// #include "backoff_algorithm.h"

/* Transport interface include. */
#include "transport_interface.h"

/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

/* Include header for connection configurations. */
#include "aws_clientcredential.h"

#include "core_http_client.h"

// #define AKNANO_DRY_RUN

#define CONFIG_BOARD BOARD_NAME

#define AKNANO_SLEEP_LENGTH 8

#define AKNANO_SHA256_LEN 32

#define CANCEL_BASE_SIZE 50
#define RECV_BUFFER_SIZE 1640
#define URL_BUFFER_SIZE 300
#define STATUS_BUFFER_SIZE 200
#define DOWNLOAD_HTTP_SIZE 200
#define DEPLOYMENT_BASE_SIZE 50
#define RESPONSE_BUFFER_SIZE 1500

#define AKNANO_JSON_BUFFER_SIZE 1024
#define NETWORK_TIMEOUT (2 * MSEC_PER_SEC)
#define AKNANO_RECV_TIMEOUT (300 * MSEC_PER_SEC)

#define AKNANO_MAX_TAG_LENGTH 32
#define AKNANO_MAX_UPDATE_AT_LENGTH 32
#define AKNANO_MAX_URI_LENGTH 120

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
#define AKNANO_MAX_TOKEN_LENGTH 100
#endif
#define AKNANO_CERT_BUF_SIZE 1024
#define AKNANO_MAX_DEVICE_NAME_SIZE 100
#define AKNANO_MAX_UUID_LENGTH 100
#define AKNANO_MAX_SERIAL_LENGTH 100
// #define AKNANO_MAX_FACTORY_NAME_LENGTH 100
#define AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH 100
// #define AKNANO_MAX_TAG_LENGTH 20


#define AKNANO_FLASH_OFF_DEV_CERTIFICATE 0
#define AKNANO_FLASH_OFF_DEV_KEY 2048
#define AKNANO_FLASH_OFF_DEV_UUID 4096
#define AKNANO_FLASH_OFF_DEV_SERIAL AKNANO_FLASH_OFF_DEV_UUID + 128

// #define AKNANO_FLASH_OFF_REGISTRATION_STATUS 8192
#define AKNANO_FLASH_OFF_STATE_BASE 8192

#define AKNANO_FLASH_OFF_LAST_APPLIED_VERSION AKNANO_FLASH_OFF_STATE_BASE + 0
#define AKNANO_FLASH_OFF_LAST_CONFIRMED_VERSION AKNANO_FLASH_OFF_STATE_BASE + sizeof(int)
#define AKNANO_FLASH_OFF_ONGOING_UPDATE_COR_ID AKNANO_FLASH_OFF_STATE_BASE + sizeof(int) * 2

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
#define AKNANO_FLASH_OFF_IS_DEVICE_REGISTERED AKNANO_FLASH_OFF_STATE_BASE + sizeof(int) * 2 + AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH
#endif

// #define AKNANO_FLASH_OFF_ID_TAG 4096 + 256 * 4
// #define AKNANO_NVS_ID_LAST_APPLIED_TAG 8
// #define AKNANO_NVS_ID_LAST_CONFIRMED_TAG 9
// #define AKNANO_FLASH_OFF_POLLING_INTERVAL 256 * 5


#define AKNANO_EVENT_DOWNLOAD_STARTED "EcuDownloadStarted"
#define AKNANO_EVENT_DOWNLOAD_COMPLETED "EcuDownloadCompleted"
#define AKNANO_EVENT_INSTALLATION_STARTED "EcuInstallationStarted"
#define AKNANO_EVENT_INSTALLATION_APPLIED "EcuInstallationApplied"
#define AKNANO_EVENT_INSTALLATION_COMPLETED "EcuInstallationCompleted"

#define AKNANO_EVENT_SUCCESS_UNDEFINED 0
#define AKNANO_EVENT_SUCCESS_FALSE 1
#define AKNANO_EVENT_SUCCESS_TRUE 2

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#ifdef AKNANO_ENABLE_EL2GO
#define AKNANO_PROVISIONING_MODE "EdgeLock 2GO Managed"
#else
#ifdef AKNANO_ENABLE_SE05X
#define AKNANO_PROVISIONING_MODE "SE05X Standalone"
#else
#define AKNANO_PROVISIONING_MODE "No Secure Element"
#endif
#endif

enum aknano_response {
    AKNANO_NETWORKING_ERROR,
    AKNANO_UNCONFIRMED_IMAGE,
    AKNANO_METADATA_ERROR,
    AKNANO_DOWNLOAD_ERROR,
    AKNANO_OK,
    AKNANO_UPDATE_INSTALLED,
    AKNANO_NO_UPDATE,
    AKNANO_CANCEL_UPDATE,
};


struct aknano_target {
    char updatedAt[AKNANO_MAX_UPDATE_AT_LENGTH];
    size_t expected_size;
    int32_t version;
    uint8_t expected_hash[AKNANO_SHA256_LEN];
};

// struct aknano_json_data {
//     size_t offset;
//     uint8_t data[AKNANO_JSON_BUFFER_SIZE];
//     struct aknano_target selected_target;
// };

struct aknano_download {
    int download_status;
    int download_progress;
    size_t downloaded_size;
    size_t http_content_size;
};


/* Settings are kept between iterations */
struct aknano_settings {
    char tag[AKNANO_MAX_TAG_LENGTH];
    // char device_priv_key[AKNANO_CERT_BUF_SIZE];
    char device_name[AKNANO_MAX_DEVICE_NAME_SIZE];
    char uuid[AKNANO_MAX_UUID_LENGTH];
    char serial[AKNANO_MAX_SERIAL_LENGTH];
    // char factory_name[AKNANO_MAX_FACTORY_NAME_LENGTH];
    uint32_t running_version;
    int last_applied_version;
    int last_confirmed_version;
    // char running_tag[AKNANO_MAX_TAG_LENGTH];
    int polling_interval;
    time_t boot_up_epoch;
    char ongoing_update_correlation_id[AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH];
    uint8_t image_position;
    const char *	hwid;
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    char token[AKNANO_MAX_TOKEN_LENGTH];
    char device_certificate[AKNANO_CERT_BUF_SIZE];
    bool is_device_registered;
#endif
};

struct aknano_network_context;

/* Context is not kept between iterations */
struct aknano_context {
    int sock;
    int32_t action_id;
    uint8_t response_data[RESPONSE_BUFFER_SIZE];
    // struct aknano_json_data aknano_json_data;
    int32_t json_action_id;
    size_t url_buffer_size;
    size_t status_buffer_size;
    struct aknano_download dl;
    // struct http_request http_req;
    // struct flash_img_context flash_ctx;
    uint8_t url_buffer[URL_BUFFER_SIZE];
    uint8_t status_buffer[STATUS_BUFFER_SIZE];
    uint8_t recv_buf_tcp[RECV_BUFFER_SIZE];
    enum aknano_response code_status;
    
    int json_pasring_bracket_level;
    struct aknano_settings *settings; /* TODO: may not always be set yet */

    struct aknano_target selected_target;

    /* Connection to the device gateway */
    struct aknano_network_context *dg_network_context;

    mbedtls_sha256_context sha256_context;
};


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


#define democonfigRANGE_REQUEST_LENGTH  4096 * 4
#define democonfigUSER_BUFFER_LENGTH democonfigRANGE_REQUEST_LENGTH + 1024

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

struct aknano_network_context
{
    /* Platform specific fields */
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext;
    TransportSocketStatus_t xNetworkStatus;
    // BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams;
    HTTPResponse_t xResponse;

    /* Platform independent fields */
    const unsigned char *reply_body;
    size_t reply_body_len;
    int reply_http_code;
};


/**/
status_t InitFlashStorage();
status_t ReadFlashStorage(int offset, void *output, size_t outputMaxLen);
status_t UpdateFlashStoragePage(int offset, void *data);
status_t WriteDataToFlash(int offset, const void *data, size_t data_len);

int init_network_context(struct aknano_network_context *network_context);

BaseType_t aknano_mtls_connect(
        struct aknano_network_context *network_context,
        const char *hostname,
        size_t hostname_len,
        uint16_t port,
        const char *server_root_ca,
        size_t server_root_ca_len
    );

BaseType_t aknano_mtls_send_http_request(
        struct aknano_network_context *network_context,
        const char *hostname,
        size_t hostname_len,
        const char * pcMethod,
        const char * pcPath,
        const char * pcBody,
        size_t xBodyLen,
        unsigned char *buffer,
        size_t buffer_len,
        const char **header_keys,
        const char **header_values,
        size_t header_len
);

void aknano_mtls_disconnect(struct aknano_network_context *network_context);

int AkNanoDownloadAndFlashImage(struct aknano_context *aknano_context);
void AkNanoUpdateSettingsInFlash(struct aknano_settings *aknano_settings);
long unsigned int AkNanoGetTime(void);
bool AkNanoSendEvent(struct aknano_settings *aknano_settings,
                     const char* event_type,
                     int version, int success);

int AkNanoPoll(struct aknano_context *aknano_context);

void AkNanoInitSettings(struct aknano_settings *aknano_settings);


extern uint8_t ucUserBuffer[ democonfigUSER_BUFFER_LENGTH ];
BaseType_t AkNano_ConnectToDevicesGateway(struct aknano_network_context *network_context);

BaseType_t AkNano_SendHttpRequest( struct aknano_network_context *network_context,
                                      const char * pcMethod,
                                      const char * pcPath,
                                      const char * pcBody,
                                      size_t xBodyLen,
                                      struct aknano_settings *aknano_settings
                                      );

int aknano_gen_serial_and_uuid(char *uuid_string, char *serial_string);

void aknano_handle_manifest_data(struct aknano_context *context,
                                 uint8_t *dst, off_t *offset, 
                                 uint8_t *src, size_t len);

void aknano_get_ipv4_and_mac(char* ipv4, uint8_t* mac);

void aknano_dump_memory_info(const char *context);


#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
bool AkNanoRegisterDevice(struct aknano_settings *aknano_settings);
#endif

#if defined(AKNANO_ENABLE_EXPLICIT_REGISTRATION) || defined(AKNANO_ALLOW_PROVISIONING)
CK_RV aknano_read_device_certificate(char* dst, size_t dst_size);
#endif

#ifdef AKNANO_RESET_DEVICE_ID
CK_RV prvDestroyDefaultCryptoObjects( void );
#endif

#ifdef AKNANO_ALLOW_PROVISIONING
status_t ClearFlashSector(int offset);
status_t WriteFlashPage(int offset, const void *data);

void vDevModeKeyProvisioning_AkNano(uint8_t *client_key, uint8_t *client_certificate);
int aknano_provision_device();
#endif

bool is_valid_certificate_available(bool);

#endif /* __AKNANO_PRIV_H__ */
