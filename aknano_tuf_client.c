#define LIBRARY_LOG_NAME "aknano_manifest"
#define LIBRARY_LOG_LEVEL LOG_INFO
#include "aknano_priv.h"
#include "logging_stack.h"

#include <stdio.h>
#include "core_json.h"

#include "libtufnano.h"

#define JSON_ARRAY_LIMIT_COUNT 10

/* TUF "callbacks" */
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

int tuf_client_write_local_file(enum tuf_role role, const unsigned char *data, size_t len, void *application_context)
{
    // int i;
    int initial_offset;
    status_t ret;

    initial_offset = get_flash_offset_for_role(role);
    // LogInfo(("write_local_file: role=%d initial_offset=%d len=%d", role, initial_offset, len));
    ret = WriteDataToFlash(initial_offset, data, len);
    LogInfo(("tuf_client_write_local_file: role=%s len=%d %s", tuf_get_role_name(role), len, ret? "ERROR" : "OK"));

    return ret;
}

int tuf_client_fetch_file(const char *file_base_name, unsigned char *target_buffer, size_t target_buffer_len, size_t *file_size, void *application_context)
{
    struct aknano_context *aknano_context = application_context;
    BaseType_t ret;

    snprintf((char*)aknano_context->url_buffer, sizeof(aknano_context->url_buffer), "/repo/%s", file_base_name);
    ret = AkNano_SendHttpRequest(
            aknano_context->dg_network_context,
            HTTP_METHOD_GET,
            (char*)aknano_context->url_buffer, "", 0,
            aknano_context->settings);

    if (ret == pdPASS) {
        LogInfo(("tuf_client_fetch_file: %s HTTP operation return code %d. Body length=%ld ", file_base_name, aknano_context->dg_network_context->reply_http_code, aknano_context->dg_network_context->reply_body_len));
        if ((aknano_context->dg_network_context->reply_http_code / 100) == 2) {
            if (aknano_context->dg_network_context->reply_body_len > target_buffer_len) {
                LogError(("tuf_client_fetch_file: %s retrieved file is too big. Maximum %ld, got %ld", file_base_name, target_buffer_len, aknano_context->dg_network_context->reply_body_len));
                return TUF_ERROR_DATA_EXCEEDS_BUFFER_SIZE;
            }
            *file_size = aknano_context->dg_network_context->reply_body_len;
            memcpy(target_buffer, aknano_context->dg_network_context->reply_body, aknano_context->dg_network_context->reply_body_len);
            target_buffer[aknano_context->dg_network_context->reply_body_len] = '\0';
            return TUF_SUCCESS;
        } else {
            return -aknano_context->dg_network_context->reply_http_code;
        }
    } else {
        LogInfo(("tuf_client_fetch_file: %s HTTP operation failed", file_base_name));
        return -1;
    }
}
