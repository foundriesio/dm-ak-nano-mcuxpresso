/*
 * Copyright 2021-2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_NAME "aknano_manifest"
#define LIBRARY_LOG_LEVEL LOG_INFO
#include "aknano_priv.h"
#include "board.h"
#include "logging_stack.h"

#include <stdio.h>
#include "core_json.h"

#include "libtufnano.h"

#define JSON_ARRAY_LIMIT_COUNT 10

static int hex_to_bin(const unsigned char *s, unsigned char *dst, size_t len)
{
    size_t i, j, k;

    memset(dst, 0, len);
    for (i = 0; i < len * 2; i++, s++) {
        if (*s >= '0' && *s <= '9') j = *s - '0'; else
        if (*s >= 'A' && *s <= 'F') j = *s - '7'; else
        if (*s >= 'a' && *s <= 'f') j = *s - 'W'; else
            return -1;

        k = ((i & 1) != 0) ? j : j << 4;

        dst[i >> 1] = (unsigned char)(dst[i >> 1] | k);
    }

    return 0;
}

/*
 * We might want call this function from within libtufnano if we switch to a
 * stream approach.
 *
 * That's why we use void* as parameter type.
 */
static int tuf_parse_single_target(const char *target_key, size_t target_key_len, const char *data, size_t len, void *application_context)
{
    struct aknano_context *aknano_context = (struct aknano_context *)application_context;

    bool found_match = false;
    const char *out_value, *out_sub_value;
    size_t out_value_len, out_sub_value_len;
    int i;
    struct aknano_target target;

    memset(&target, 0, sizeof(target));

    LogInfo(("tuf_client_parse_single_target: %.*s", target_key_len, target_key));
    // LogInfo(("handle_json_data: Parsing target data with len=%d", len));
    JSONStatus_t result = JSON_Validate(data, len);

    if (result != JSONSuccess) {
        LogInfo(("handle_json_data: Got invalid targets JSON: %s\n", data));
        return -1;
    }

    LogDebug(("tuf_parse_single_target: handling target"));
    found_match = false;
    result = JSON_SearchConst(data, len, "custom/version", strlen("custom/version"), &out_value, &out_value_len, NULL);
    if (result == JSONSuccess) {
        sscanf(out_value, "%ld", &target.version);
        // LogInfo(("tuf_parse_single_target: version=%d selected_version=%d\n", version, aknano_context->selected_target.version));
        if (target.version <= aknano_context->selected_target.version)
            return 0;
    } else {
        LogInfo(("handle_json_data: custom/version not found\n"));
        return 0;
    }

    result = JSON_SearchConst(data, len, "custom/hardwareIds", strlen("custom/hardwareIds"), &out_value, &out_value_len, NULL);
    if (result == JSONSuccess) {
        // LogInfo(("handle_json_data: custom.hardwareIds=%.*s", out_value_len, out_value));

        for (i = 0; i < JSON_ARRAY_LIMIT_COUNT; i++) {
            char s[10];
            snprintf(s, sizeof(s), "[%d]", i);
            if (JSON_SearchConst(out_value, out_value_len, s, strnlen(s, sizeof(s)), &out_sub_value, &out_sub_value_len, NULL) != JSONSuccess)
                break;
            if (strncmp(out_sub_value, aknano_context->settings->hwid, out_sub_value_len) == 0)
                // LogInfo(("Found matching hardwareId" ));
                found_match = true;
        }
    } else {
        LogInfo(("handle_json_data: custom/hardwareIds not found\n"));
        return 0;
    }
    if (!found_match) {
        LogInfo(("Matching hardwareId not found (%s)", aknano_context->settings->hwid));
        return 0;
    }

    found_match = false;
    result = JSON_SearchConst((char *)data, len, "custom/tags", strlen("custom/tags"), &out_value, &out_value_len, NULL);
    if (result == JSONSuccess) {
        // LogInfo(("handle_json_data: custom.tags=%.*s", out_value_len, out_value));

        for (i = 0; i < JSON_ARRAY_LIMIT_COUNT; i++) {
            char s[10];
            snprintf(s, sizeof(s), "[%d]", i);
            if (JSON_SearchConst(out_value, out_value_len, s, strlen(s), &out_sub_value, &out_sub_value_len, NULL) != JSONSuccess)
                break;
            if (strncmp(out_sub_value, aknano_context->settings->tag, out_sub_value_len) == 0)
                // LogInfo(("Found matching tag" ));
                found_match = true;
        }
    } else {
        LogInfo(("handle_json_data: custom/tags not found\n"));
        return 0;
    }
    if (!found_match) {
        LogInfo(("Matching tag not found (%s)", aknano_context->settings->tag));
        return 0;
    }

    // result = JSON_SearchConst(data, len, "custom.updatedAt", strlen("custom.updatedAt"), &out_value, &out_value_len);
    // if (result == JSONSuccess) {
    //         LogInfo(("handle_json_data: custom.updatedAt=%.*s", out_value_len, out_value));
    // } else {
    //         LogWarn(("handle_json_data: custom.updatedAt not found"));
    //         return -2;
    // }

    /* Handle hashes/sha256 */
    result = JSON_SearchConst(data, len, "hashes/sha256", strlen("hashes/sha256"), &out_value, &out_value_len, NULL);
    if (result != JSONSuccess) {
        LogInfo(("handle_json_data: hashes/sha256 not found\n"));
        return 0;
    }
    if (out_value_len != AKNANO_SHA256_LEN * 2) {
        LogInfo(("handle_json_data: hashes/sha256 string has invalid length: %d\n", out_value_len));
        return 0;
    }
    if (hex_to_bin((unsigned char *)out_value, (unsigned char *)&target.expected_hash, AKNANO_SHA256_LEN)) {
        LogInfo(("handle_json_data: hashes/sha256 string is not a valid hex value: '%.*s'\n", out_value_len, out_value));
        return 0;
    }

    /* Handle length */
    result = JSON_SearchConst(data, len, "length", strlen("length"), &out_value, &out_value_len, NULL);
    if (result != JSONSuccess) {
        LogInfo(("handle_json_data: length not found\n"));
        return 0;
    }
    sscanf(out_value, "%u", &target.expected_size);

    /* All good, update selected_target */
    memcpy(&aknano_context->selected_target, &target, sizeof(aknano_context->selected_target));

    // LogInfo(("Updating highest version to %u %.*s", version, out_value_len, out_value));
    return 0;
}


int parse_targets_metadata(const char *data, int len, void *application_context)
{
    JSONStatus_t result;
    int ret;

    const char *out_value;
    size_t out_value_len;
    size_t start, next;
    JSONPair_t pair;

    result = JSON_Validate(data, len);
    if (result != JSONSuccess) {
        LogError(("parse_targets_metadata: Got invalid JSON: %s", data));
        return TUF_ERROR_INVALID_METADATA;
    }

    result = JSON_SearchConst(data, len, "signed" TUF_JSON_QUERY_KEY_SEPARATOR "targets", strlen("signed" TUF_JSON_QUERY_KEY_SEPARATOR "targets"), &out_value, &out_value_len, NULL);
    if (result != JSONSuccess) {
        LogError(("parse_targets_metadata: \"signed" TUF_JSON_QUERY_KEY_SEPARATOR "targets\" not found"));
        return TUF_ERROR_FIELD_MISSING;
    }

    /* Iterate over each target */
    start = 0;
    next = 0;
    while ((result = JSON_Iterate(out_value, out_value_len, &start, &next, &pair)) == JSONSuccess) {
        ret = tuf_parse_single_target(pair.key, pair.keyLength, pair.value, pair.valueLength, application_context);
        if (ret < 0) {
            LogError(("Error processing target %.*s", (int)pair.keyLength, pair.key));
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Give some time to log task handle trace
    }

    return 0;
}
