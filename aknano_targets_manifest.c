#define LIBRARY_LOG_NAME "aknano_manifest"
#define LIBRARY_LOG_LEVEL LOG_INFO
#include "aknano_priv.h"
#include "board.h"
#include "logging_stack.h"

#include <stdio.h>
#include "core_json.h"


#define JSON_ARRAY_LIMIT_COUNT 10

static int handle_json_data(struct aknano_context *aknano_context, uint8_t *data, size_t len)
{
	bool foundMatch = false;
        char *outValue, *outSubValue, *uri;
        size_t outValueLength, outSubValueLength;
        int i;
        uint32_t version;

        // LogInfo(("handle_json_data: Parsing target data with len=%d", len));
        JSONStatus_t result = JSON_Validate((char*)data, len );
        if( result != JSONSuccess )
        {
                LogWarn(("handle_json_data: Got invalid targets JSON: %s", data));
                return -1;
        }

        foundMatch = false;
        result = JSON_Search((char*)data, len, "custom.version", strlen("custom.version"), &outValue, &outValueLength);
        if (result == JSONSuccess) {
                // LogInfo(("handle_json_data: custom.version=%.*s", outValueLength, outValue));
                sscanf(outValue, "%u", &version);
                if (version <= aknano_context->aknano_json_data.selected_target.version) {
                        return 0;
                }

        } else {
                LogWarn(("handle_json_data: custom.version not found"));
                return -2;
        }

        result = JSON_Search((char*)data, len, "custom.hardwareIds", strlen("custom.hardwareIds"), &outValue, &outValueLength);
        if (result == JSONSuccess) {
                // LogInfo(("handle_json_data: custom.hardwareIds=%.*s", outValueLength, outValue));

                for (i=0; i<JSON_ARRAY_LIMIT_COUNT; i++) {
                        char s[10];
                        snprintf(s, sizeof(s), "[%d]", i);
                        if (JSON_Search(outValue, outValueLength, s, strlen(s), &outSubValue, &outSubValueLength) != JSONSuccess)
                                break;
                        if (strncmp(outSubValue, CONFIG_BOARD, outSubValueLength) == 0) {
                                // LogInfo(("Found matching hardwareId" ));
                                foundMatch = true;
                        } 
                }
        } else {
                LogWarn(("handle_json_data: custom.hardwareIds not found"));
                return -2;
        }
        if (!foundMatch) {
                // LogInfo(("Matching hardwareId not found (%s)", CONFIG_BOARD));
                return 0;
        }

        foundMatch = false;
        result = JSON_Search((char*)data, len, "custom.tags", strlen("custom.tags"), &outValue, &outValueLength);
        if (result == JSONSuccess) {
                // LogInfo(("handle_json_data: custom.tags=%.*s", outValueLength, outValue));

                for (i=0; i<JSON_ARRAY_LIMIT_COUNT; i++) {
                        char s[10];
                        snprintf(s, sizeof(s), "[%d]", i);
                        if (JSON_Search(outValue, outValueLength, s, strlen(s), &outSubValue, &outSubValueLength) != JSONSuccess)
                                break;
                        if (strncmp(outSubValue, aknano_context->settings->tag, outSubValueLength) == 0) {
                                // LogInfo(("Found matching tag" ));
                                foundMatch = true;
                        } 
                }
        } else {
                LogWarn(("handle_json_data: custom.tags not found"));
                return -2;
        }
        if (!foundMatch) {
                // LogInfo(("Matching tag not found (%s)", aknano_context->settings->tag));
                return 0;
        }


        // result = JSON_Search(data, len, "custom.updatedAt", strlen("custom.updatedAt"), &outValue, &outValueLength);
        // if (result == JSONSuccess) {
        //         LogInfo(("handle_json_data: custom.updatedAt=%.*s", outValueLength, outValue));
        // } else {
        //         LogWarn(("handle_json_data: custom.updatedAt not found"));
        //         return -2;
        // }

        result = JSON_Search(data, len, "custom.uri", strlen("custom.uri"), &outValue, &outValueLength);
        if (result == JSONSuccess) {
                // LogInfo(("handle_json_data: custom.uri=%.*s", outValueLength, outValue));
        } else {
                LogWarn(("handle_json_data: custom.uri not found"));
                return -2;
        }


        // LogInfo(("Updating highest version to %u %.*s", version, outValueLength, outValue));

        aknano_context->aknano_json_data.selected_target.version = version;
        strncpy(aknano_context->aknano_json_data.selected_target.uri, outValue, outValueLength);
	return 0;
}

void aknano_handle_manifest_data(struct aknano_context *context,
					uint8_t *dst, size_t *offset, 
					uint8_t *src, size_t len)
{
	static bool trunc_wrn_printed = false;
	bool is_relevant_data;
	uint8_t *p = src;

	/* 
	 * Based on current format, counting brackets ({ and }) is enough 
	 * to determine when a "target" section starts and ends. 
	 * We should process all data that is inside the 4th bracket level.
	 * And a target section is known to end every time we go back to the
	 * 1st bracket level.
	 */
	const int reference_bracket_level = 4; 
	is_relevant_data = context->json_pasring_bracket_level >= reference_bracket_level;

	while (p < src + len) {
		switch (*p) {
		case '{': 
			context->json_pasring_bracket_level++;
			is_relevant_data = context->json_pasring_bracket_level >= reference_bracket_level;
			break;
		case '}':
			context->json_pasring_bracket_level--;
			break;
		}
		if (is_relevant_data) {
			if (*offset < RESPONSE_BUFFER_SIZE) {
				*(dst + *offset) = *p;
				(*offset)++;
			} else if (!trunc_wrn_printed) {
				LogWarn(("Truncating incomming data"
						" (target description limit is %d)",
						RESPONSE_BUFFER_SIZE));
				trunc_wrn_printed = true;
			}

			if (context->json_pasring_bracket_level == reference_bracket_level-1) {
				*(dst + *offset) = '\0';
				/* A complete target section was received. Process it */
				handle_json_data(context, dst, *offset);
				is_relevant_data = false;
				trunc_wrn_printed = false;
				*offset = 0;
			}
		}
		p++;
	}
}
