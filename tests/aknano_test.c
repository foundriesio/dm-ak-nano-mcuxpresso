
#include "unity.h"
#include "unity_fixture.h"

#define LIBRARY_LOG_LEVEL LOG_INFO

#include <time.h>
#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "sntp_example.h"
#include "lwip/netif.h"

#include "aknano_priv.h"
#include "aknano_secret.h"


/**
 * @brief Test group definition.
 */
TEST_GROUP( Full_AKNano );

TEST_SETUP( Full_AKNano )
{
}

TEST_TEAR_DOWN( Full_AKNano )
{
}

TEST_GROUP_RUNNER( Full_AKNano )
{
    RUN_TEST_CASE( Full_AKNano, akNano_TestFlashAccess );
    RUN_TEST_CASE( Full_AKNano, akNano_TestDeviceGatewayAccess );
}

TEST( Full_AKNano, akNano_TestFlashAccess )
{
    uint8_t original_data[128];
    uint8_t new_data[128];
    uint8_t i;
    status_t ret;

    ret = ReadFlashStorage(AKNANO_FLASH_OFF_STATE_BASE,
                     original_data,
                     sizeof(original_data));

    TEST_ASSERT(ret == kStatus_Success);

    for (i=0; i< sizeof(new_data); i++)
        new_data[i] = i;

    ret = UpdateFlashStoragePage(AKNANO_FLASH_OFF_STATE_BASE,
                     new_data);

    TEST_ASSERT(ret == kStatus_Success);
    ret = ReadFlashStorage(AKNANO_FLASH_OFF_STATE_BASE,
                     new_data, sizeof(new_data));

    TEST_ASSERT(ret == kStatus_Success);

    for (i=0; i< sizeof(new_data); i++)
        TEST_ASSERT(new_data[i] == i);

    ret = UpdateFlashStoragePage(AKNANO_FLASH_OFF_STATE_BASE,
                     original_data);

    TEST_ASSERT(ret == kStatus_Success);

    ret = ReadFlashStorage(AKNANO_FLASH_OFF_STATE_BASE,
                     new_data, sizeof(new_data));

    TEST_ASSERT(ret == kStatus_Success);

    TEST_ASSERT(ret == kStatus_Success);
    for (i=0; i< sizeof(new_data); i++)
        TEST_ASSERT(new_data[i] == original_data[i]);
}

TEST( Full_AKNano, akNano_TestDeviceGatewayAccess )
{
    struct aknano_network_context network_context;
    BaseType_t xDemoStatus = pdPASS;

    if (!is_valid_certificate_available(false))
        TEST_IGNORE_MESSAGE("Device certificate is not available. Skipping Device Gateway connection test");

    xDemoStatus = AkNano_ConnectToDevicesGateway(&network_context);
    TEST_ASSERT(xDemoStatus == pdPASS);

    /* Close the network connection.  */
    aknano_mtls_disconnect(&network_context);
}

int RunAkNanoTest( void )
{
    int status = -1;

    /* Initialize unity. */
    UnityFixture.Verbose = 1;
    UnityFixture.GroupFilter = 0;
    UnityFixture.NameFilter = 0;
    UnityFixture.RepeatCount = 1;
    UnityFixture.Silent = 0;

    UNITY_BEGIN();

    /* Run the test group. */
    RUN_TEST_GROUP( Full_AKNano );

    status = UNITY_END();
    return status;
}
