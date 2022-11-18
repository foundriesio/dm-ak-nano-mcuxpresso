#define LIBRARY_LOG_LEVEL LOG_INFO

#include <time.h>
#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "sntp_example.h"
#include "lwip/netif.h"

#include "aknano_priv.h"
#include "aknano_secret.h"
#include "flexspi_flash_config.h"


/**
 * @brief A buffer used in the demo for storing HTTP request headers and
 * HTTP response headers and body.
 *
 * @note This demo shows how the same buffer can be re-used for storing the HTTP
 * response after the HTTP request is sent out. However, the user can also
 * decide to use separate buffers for storing the HTTP request and response.
 */
uint8_t ucUserBuffer[ democonfigUSER_BUFFER_LENGTH ];

/**
 * @brief Global entry time into the application to use as a reference timestamp
 * in the #prvGetTimeMs function. #prvGetTimeMs will always return the difference
 * between the current time and the global entry time. This will reduce the
 * chances of overflow for the 32 bit unsigned integer used for holding the
 * timestamp.
 */
// static uint32_t ulGlobalEntryTimeMs;

static struct aknano_settings xaknano_settings;
static struct aknano_context xaknano_context;


long unsigned int AkNanoGetTime(void)
{
    static long unsigned int t;
    //  = clock() * 1000 / CLOCKS_PER_SEC;
    t++;
    // LogInfo(( "osfGetTime %d", t) );
    return t;
}


void AkNanoUpdateSettingsInFlash(struct aknano_settings *aknano_settings)
{
    char flashPageBuffer[256];
    memset(flashPageBuffer, 0, sizeof(flashPageBuffer));

    memcpy(flashPageBuffer, &aknano_settings->last_applied_version, sizeof(int));
    memcpy(flashPageBuffer + sizeof(int), &aknano_settings->last_confirmed_version, sizeof(int));
    memcpy(flashPageBuffer + sizeof(int) * 2, aknano_settings->ongoing_update_correlation_id,
           sizeof(aknano_settings->ongoing_update_correlation_id));
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    flashPageBuffer[sizeof(int)*2 + sizeof(aknano_settings->ongoing_update_correlation_id)] =
        aknano_settings->is_device_registered;
#endif
    LogInfo(("Saving buffer: 0x%x 0x%x 0x%x 0x%x    0x%x 0x%x 0x%x 0x%x    0x%x 0x%x 0x%x 0x%x",
    flashPageBuffer[0], flashPageBuffer[1], flashPageBuffer[2], flashPageBuffer[3],
    flashPageBuffer[4], flashPageBuffer[5], flashPageBuffer[6], flashPageBuffer[7],
    flashPageBuffer[8], flashPageBuffer[9], flashPageBuffer[10], flashPageBuffer[11]));
    UpdateFlashStoragePage(AKNANO_FLASH_OFF_STATE_BASE, flashPageBuffer);
}

void AkNanoInitSettings(struct aknano_settings *aknano_settings)
{
    uint32_t temp_value;

    memset(aknano_settings, 0, sizeof(*aknano_settings));
    strcpy(aknano_settings->tag, "devel");
    aknano_settings->polling_interval = 15;
    // strcpy(aknano_settings->factory_name, "nxp-hbt-poc");
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    strcpy(aknano_settings->token, AKNANO_API_TOKEN);
#endif

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    aknano_read_device_certificate(aknano_settings->device_certificate, sizeof(aknano_settings->device_certificate));
#endif
    // strcpy(aknano_settings->serial, "000000000000");
    // sfw_flash_read(REMAP_FLAG_ADDRESS, &aknano_settings->image_position, 1);
    aknano_settings->image_position = get_active_image() + 1;

    LogInfo(("AkNanoInitSettings: image_position=%u", aknano_settings->image_position));

    bl_get_image_build_num(&aknano_settings->running_version, aknano_settings->image_position);
    LogInfo(("AkNanoInitSettings: aknano_settings->running_version=%lu",
             aknano_settings->running_version));

    ReadFlashStorage(AKNANO_FLASH_OFF_DEV_SERIAL, aknano_settings->serial,
                     sizeof(aknano_settings->serial));
    if (aknano_settings->serial[0] == 0xff)
        aknano_settings->serial[0] = 0;
    LogInfo(("AkNanoInitSettings: serial=%s", aknano_settings->serial));

    ReadFlashStorage(AKNANO_FLASH_OFF_DEV_UUID, aknano_settings->uuid,
                     sizeof(aknano_settings->uuid));
    if (aknano_settings->uuid[0] == 0xff)
        aknano_settings->uuid[0] = 0;
    LogInfo(("AkNanoInitSettings: uuid=%s", aknano_settings->uuid));

    ReadFlashStorage(AKNANO_FLASH_OFF_LAST_APPLIED_VERSION,
                     &aknano_settings->last_applied_version,
                     sizeof(aknano_settings->last_applied_version));
    if (aknano_settings->last_applied_version < 0 || aknano_settings->last_applied_version > 999999999)
        aknano_settings->last_applied_version = 0;
    LogInfo(("AkNanoInitSettings: last_applied_version=%d", aknano_settings->last_applied_version));

    ReadFlashStorage(AKNANO_FLASH_OFF_LAST_CONFIRMED_VERSION, &aknano_settings->last_confirmed_version,
                     sizeof(aknano_settings->last_confirmed_version));
    if (aknano_settings->last_confirmed_version < 0 || aknano_settings->last_confirmed_version > 999999999)
        aknano_settings->last_confirmed_version = 0;
    LogInfo(("AkNanoInitSettings: last_confirmed_version=%d",
             aknano_settings->last_confirmed_version));

    ReadFlashStorage(AKNANO_FLASH_OFF_ONGOING_UPDATE_COR_ID,
                     &aknano_settings->ongoing_update_correlation_id,
                     sizeof(aknano_settings->ongoing_update_correlation_id));
    if (aknano_settings->ongoing_update_correlation_id[0] == 0xFF)
        aknano_settings->ongoing_update_correlation_id[0] = 0;
    LogInfo(("AkNanoInitSettings: ongoing_update_correlation_id=%s",
             aknano_settings->ongoing_update_correlation_id));

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    ReadFlashStorage(AKNANO_FLASH_OFF_IS_DEVICE_REGISTERED,
                     &temp_value,
                     sizeof(temp_value));
    aknano_settings->is_device_registered = (temp_value & 0xFF) == 1;
    LogInfo(("AkNanoInitSettings:  is_device_registered=%d",
             aknano_settings->is_device_registered));
#endif

    snprintf(aknano_settings->device_name, sizeof(aknano_settings->device_name),
            "%s-%s",
            CONFIG_BOARD, aknano_settings->serial);

    LogInfo(("AkNanoInitSettings: device_name=%s",
             aknano_settings->device_name));

    aknano_settings->hwid = CONFIG_BOARD;
}

// #define AKNANO_TEST_ROLLBACK

static int aknano_handle_img_confirmed(struct aknano_settings *aknano_settings)
{
    bool image_ok = false;
    uint32_t currentStatus;

#ifdef AKNANO_TEST_ROLLBACK
    #warning "Compiling broken image for rollback test"
    LogError((ANSI_COLOR_RED "This is a rollback test. Rebooting in 5 seconds" ANSI_COLOR_RESET));
    vTaskDelay( pdMS_TO_TICKS( 5000 ) );
    NVIC_SystemReset();
#endif

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

    // LogInfo(("Image is%s confirmed OK", image_ok ? "" : " not"));

    LogInfo(("aknano_settings.ongoing_update_correlation_id='%s'", aknano_settings->ongoing_update_correlation_id));

    if (aknano_settings->last_applied_version
        && aknano_settings->last_applied_version != aknano_settings->running_version
        && strnlen(aknano_settings->ongoing_update_correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH) > 0) {

        LogInfo(("A rollback was done"));
        AkNanoSendEvent(aknano_settings,
                  AKNANO_EVENT_INSTALLATION_COMPLETED,
                  -1, AKNANO_EVENT_SUCCESS_FALSE);
        //aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID, "", 0);

        memset(aknano_settings->ongoing_update_correlation_id, 0,
               sizeof(aknano_settings->ongoing_update_correlation_id));
        AkNanoUpdateSettingsInFlash(aknano_settings);
    }

    if (!image_ok) {
        //ret = boot_write_img_confirmed();
        //if (ret < 0) {
        //    LogError(("Couldn't confirm this image: %d", ret));
        //    return ret;
        //}

        // LogInfo(("Marked image as OK"));

        // image_ok = boot_is_img_confirmed();
        // LogInfo(("after Image is%s confirmed OK", image_ok ? "" : " not"));

        AkNanoSendEvent(aknano_settings, AKNANO_EVENT_INSTALLATION_COMPLETED, 0, AKNANO_EVENT_SUCCESS_TRUE);

        // aknano_write_to_nvs(AKNANO_NVS_ID_ONGOING_UPDATE_COR_ID, "", 0);
        // aknano_write_to_nvs(AKNANO_NVS_ID_LAST_CONFIRMED_VERSION, &aknano_settings.running_version, sizeof(aknano_settings.running_version));
        // aknano_write_to_nvs(AKNANO_NVS_ID_LAST_APPLIED_VERSION, &zero, sizeof(zero));

        memset(aknano_settings->ongoing_update_correlation_id, 0,
               sizeof(aknano_settings->ongoing_update_correlation_id));
        aknano_settings->last_applied_version = 0;
        aknano_settings->last_confirmed_version = aknano_settings->running_version;
        AkNanoUpdateSettingsInFlash(aknano_settings);

        // aknano_write_to_nvs(AKNANO_NVS_ID_LAST_CONFIRMED_TAG, aknano_settings.tag, sizeof(aknano_settings.tag));

        // TODO: do the same in FreeRTOS?
        // ret = boot_erase_img_bank(FLASH_AREA_ID(image_1));
        // if (ret) {
        //     LOG_ERR("Failed to erase second slot");
        //     return ret;
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
        memset(aknano_settings->ongoing_update_correlation_id, 0,
               sizeof(aknano_settings->ongoing_update_correlation_id));
        aknano_settings->last_applied_version = 0;
        aknano_settings->last_confirmed_version = aknano_settings->running_version;
        AkNanoUpdateSettingsInFlash(aknano_settings);

        LogInfo(("Updating aknano_settings->running_version in flash (%d -> %lu)",
                 aknano_settings->last_confirmed_version, aknano_settings->running_version));
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

static bool is_certificate_valid(const char* pem)
{
    size_t cert_len;

    if (pem[0] != '-')
        return false;

    cert_len = strnlen(pem, AKNANO_CERT_BUF_SIZE);

    if (cert_len < 200 || cert_len >= AKNANO_CERT_BUF_SIZE)
        return false;

    return true;
}

static bool is_certificate_available_cache = false;
static bool is_valid_certificate_available_()
{
    static CK_RV cert_status;
    char device_certificate[AKNANO_CERT_BUF_SIZE];

    cert_status = aknano_read_device_certificate(device_certificate, sizeof(device_certificate));
    if (cert_status != CKR_OK) {
        is_certificate_available_cache = false;
    } else {
        is_certificate_available_cache = is_certificate_valid(device_certificate);
    }
    return is_certificate_available_cache;
}

bool is_valid_certificate_available(bool use_cached_value)
{
    if (use_cached_value)
        return is_certificate_available_cache;

    return is_valid_certificate_available_();
}


static void AkNanoInit(struct aknano_settings *aknano_settings)
{
    bool registrationOk;
    CK_RV cert_status;

#ifdef AKNANO_RESET_DEVICE_ID
    LogWarn((ANSI_COLOR_RED "AKNANO_RESET_DEVICE_ID is set. Removing provisioned device data" ANSI_COLOR_RESET));
    aknano_reset_device_id();
    prvDestroyDefaultCryptoObjects();
#endif

    vTaskDelay( pdMS_TO_TICKS( 60000 ) );
#if !defined(AKNANO_ENABLE_EL2GO) && defined(AKNANO_ALLOW_PROVISIONING)
    if (!is_valid_certificate_available(false)) {
        LogWarn((ANSI_COLOR_RED "Device certificate is not set. Running provisioning process" ANSI_COLOR_RESET));
        aknano_gen_and_store_random_device_certificate_and_key();
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (!is_valid_certificate_available(false)) {
            LogError((ANSI_COLOR_RED "Fatal: Error fetching device certificate" ANSI_COLOR_RESET));
            vTaskDelay(pdMS_TO_TICKS( 120000 ));
        }
    } else {
        LogInfo(("Device certificate is set"));
    }
#endif

#if defined(AKNANO_ENABLE_EL2GO) && defined(AKNANO_ALLOW_PROVISIONING)
    LogInfo(("EL2Go provisioning enabled. Waiting for secure objects to be retrieved"));
    while (!is_valid_certificate_available(false)) {
        vTaskDelay( pdMS_TO_TICKS( 10000 ) );
    }
    LogInfo(("EL2GO provisioning succeeded. Proceeding"));
#endif
    AkNanoInitSettings(aknano_settings);

    vTaskDelay(pdMS_TO_TICKS(100));
    aknano_handle_img_confirmed(aknano_settings);

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    if (!xaknano_settings.is_device_registered) {
        registrationOk = AkNanoRegisterDevice(&xaknano_settings);
        if (registrationOk) {
            xaknano_settings.is_device_registered = registrationOk;
            AkNanoUpdateSettingsInFlash(&xaknano_settings);
        }
    }
#endif

#ifdef AKNANO_DELETE_TUF_DATA
    LogWarn((ANSI_COLOR_RED "**** Reseting TUF data ****" ANSI_COLOR_RESET));
    #include "libtufnano.h"
    tuf_client_write_local_file(ROLE_ROOT, "\xFF", 1, NULL);
    tuf_client_write_local_file(ROLE_TIMESTAMP, "\xFF", 1, NULL);
    tuf_client_write_local_file(ROLE_SNAPSHOT, "\xFF", 1, NULL);
    tuf_client_write_local_file(ROLE_ROOT, "\xFF", 1, NULL);

    LogWarn((ANSI_COLOR_RED "**** Sleeping for 20 seconds ****" ANSI_COLOR_RESET));
    vTaskDelay(pdMS_TO_TICKS(20000));
#endif
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

    LogInfo(("AKNano RunAkNanoDemo mode '" AKNANO_PROVISIONING_MODE "'"));
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
