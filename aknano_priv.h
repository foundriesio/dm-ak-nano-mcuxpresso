/*
 * Copyright (c) 2020 Linumiz
 * Copyright (c) 2021 Foundries.io
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 *
 * @brief This file contains structures representing JSON messages
 * exchanged with a hawkbit
 */

#ifndef __AKNANO_PRIV_H__
#define __AKNANO_PRIV_H__

#include "board.h"

#include "fsl_common.h"
#include <time.h>

#define size_t int32_t

#define CONFIG_BOARD BOARD_NAME

#define AKNANO_SLEEP_LENGTH 8


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

#define AKNANO_MAX_TAG_LENGTH 30
#define AKNANO_MAX_UPDATE_AT_LENGTH 30
#define AKNANO_MAX_URI_LENGTH 120

#define AKNANO_MAX_TOKEN_LENGTH 100
#define AKNANO_CERT_BUF_SIZE 1024
#define AKNANO_MAX_DEVICE_NAME_SIZE 100
#define AKNANO_MAX_UUID_LENGTH 100
#define AKNANO_MAX_SERIAL_LENGTH 100
#define AKNANO_MAX_FACTORY_NAME_LENGTH 100
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
	char uri[AKNANO_MAX_URI_LENGTH];
	int32_t version;
};

struct aknano_json_data {
	size_t offset;
	uint8_t data[AKNANO_JSON_BUFFER_SIZE];
	struct aknano_target selected_target;
};

struct aknano_download {
	int download_status;
	int download_progress;
	size_t downloaded_size;
	size_t http_content_size;
};


/* Settings are kept between iterations */
struct aknano_settings {
	char tag[AKNANO_MAX_TAG_LENGTH];
	char token[AKNANO_MAX_TOKEN_LENGTH];
	char device_certificate[AKNANO_CERT_BUF_SIZE];
	char device_priv_key[AKNANO_CERT_BUF_SIZE];
	char device_name[AKNANO_MAX_DEVICE_NAME_SIZE];
	char uuid[AKNANO_MAX_UUID_LENGTH];
	char serial[AKNANO_MAX_SERIAL_LENGTH];
	char factory_name[AKNANO_MAX_FACTORY_NAME_LENGTH];
	int running_version;
	int last_applied_version;
	int last_confirmed_version;
	// char running_tag[AKNANO_MAX_TAG_LENGTH];
	int polling_interval;
	time_t boot_up_epoch;
	char ongoing_update_correlation_id[AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH];
};

/* Context is not kept between iterations */
struct aknano_context {
	int sock;
	int32_t action_id;
	uint8_t response_data[RESPONSE_BUFFER_SIZE];
	struct aknano_json_data aknano_json_data;
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
};

#endif /* __AKNANO_PRIV_H__ */
