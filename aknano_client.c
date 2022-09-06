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

#include "fsl_caam.h"

/*
 * Random numbers generator
 */
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

/*
 * Time
 */
#include "compile_epoch.h"
#ifndef BUILD_EPOCH_MS
#define BUILD_EPOCH_MS 1637778974000
#endif

static time_t boot_up_epoch;
void sntp_set_system_time(u32_t sec)
{
    LogInfo(("SNTP sntp_set_system_time"));
    char buf[32];
    struct tm current_time_val;
    time_t current_time = (time_t)sec;

    localtime_r(&current_time, &current_time_val);
    strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &current_time_val);
    //boot_up_epoch_ms = (sec * 1000);// - xTaskGetTickCount();
    boot_up_epoch = sec;// - xTaskGetTickCount();

    LogInfo(("SNTP time: %s  sec=%lu boot_up_epoch=%lld xTaskGetTickCount()=%lu\n",
             buf, sec, boot_up_epoch, xTaskGetTickCount()));
    vTaskDelay(50 / portTICK_PERIOD_MS);

    // LOCK_TCPIP_CORE();
    sntp_stop();
    // UNLOCK_TCPIP_CORE();
}

void sntp_example_init(void)
{
    ip4_addr_t ip_sntp_server;

    /* Using a.time.steadfast.net */
    IP4_ADDR(&ip_sntp_server, 208, 100, 4, 52);

    LOCK_TCPIP_CORE();

    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    sntp_setserver(0, &ip_sntp_server);

    sntp_init();
    UNLOCK_TCPIP_CORE();
    LogInfo(("SNTP Initialized"));
}

static void SNTPRequest()
{
    sntp_example_init();

    int i;
    for(i=0; i<5; i++) {
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        if (boot_up_epoch) {
            vTaskDelay( pdMS_TO_TICKS( 500 ) );
            break;
        }
    }
    if (i == 5) {
        // LOCK_TCPIP_CORE();
        sntp_stop();
        // UNLOCK_TCPIP_CORE();
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }

    LogInfo(("Proceeding after sntp"));
}


time_t get_current_epoch()
{
    if (boot_up_epoch == 0)
        boot_up_epoch = 1637778974; // 2021-11-24

    return boot_up_epoch + (xTaskGetTickCount() / 1000);
}

int initTime()
{
        SNTPRequest();
        return 0;
}

int initStorage()
{
        return InitFlashStorage();
}

/*
 * Device Gateway:
 * - Connect
 * - Disconnect
 * - HTTP request (prvSendHttpRequest)
 *
 */

/*
 * API:
 * - Connect
 * - Disconnect
 * - HTTP request (prvSendHttpRequest)
 *
 */

