
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
// #include "flexspi_flash_config.h"


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
    /************* otaPAL_CloseFile Tests *************/
    RUN_TEST_CASE( Full_AKNano, akNano_BasicTest );
    RUN_TEST_CASE( Full_AKNano, akNano_TestFlashAccess );
    RUN_TEST_CASE( Full_AKNano, akNano_TestDeviceGatewayAccess );
    RUN_TEST_CASE( Full_AKNano, akNano_TestDeviceGatewayGetRoot );
    RUN_TEST_CASE( Full_AKNano, akNano_TestDeviceGatewayGetTargets );
}


TEST( Full_AKNano, akNano_BasicTest )
{
    TEST_ASSERT_EQUAL( 1, 1 );
    TEST_ASSERT_EQUAL( 2, 2 );
}

TEST( Full_AKNano, akNano_TestFlashAccess )
{
    char buf[500];

    status_t ret = ReadFlashStorage(AKNANO_FLASH_OFF_LAST_APPLIED_VERSION,
                     buf,
                     4);

    TEST_ASSERT(ret == kStatus_Success);
}

TEST( Full_AKNano, akNano_TestDeviceGatewayAccess )
{
    TransportInterface_t xTransportInterface;
    NetworkContext_t xNetworkContext = { 0 };
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    BaseType_t xDemoStatus;

    struct aknano_settings aknano_settings;
    struct aknano_context aknano_context;

    AkNanoInitSettings(&aknano_settings);
    if (aknano_settings.device_certificate[0] == 0) {
        TEST_IGNORE_MESSAGE("Device certificate is not set. Skipping execution of test");
    }

    memset(&aknano_context, 0, sizeof(aknano_context));
    aknano_context.settings = &aknano_settings;

    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = AkNano_ConnectToDevicesGateway(&xNetworkContext, &xTransportInterface);

    /* Close the network connection.  */
    SecureSocketsTransport_Disconnect( &xNetworkContext );

    TEST_ASSERT(xDemoStatus == pdPASS);
}

TEST( Full_AKNano, akNano_TestDeviceGatewayGetRoot )
{
    TransportInterface_t xTransportInterface;
    NetworkContext_t xNetworkContext = { 0 };
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;
    BaseType_t xDemoStatus;

    struct aknano_settings aknano_settings;

    AkNanoInitSettings(&aknano_settings);
    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = AkNano_ConnectToDevicesGateway(&xNetworkContext, &xTransportInterface);
    TEST_ASSERT(xDemoStatus == pdPASS);

    xDemoStatus = AkNano_GetRootMetadata(&xTransportInterface, &aknano_settings, &xResponse);
    TEST_ASSERT(xDemoStatus == pdPASS);
    TEST_ASSERT(xResponse.statusCode == 404);

    /* Close the network connection.  */
    SecureSocketsTransport_Disconnect( &xNetworkContext );

    TEST_ASSERT(xDemoStatus == pdPASS);
}

TEST( Full_AKNano, akNano_TestDeviceGatewayGetTargets )
{
    TransportInterface_t xTransportInterface;
    NetworkContext_t xNetworkContext = { 0 };
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };
    HTTPResponse_t xResponse;
    BaseType_t xDemoStatus;

    struct aknano_settings aknano_settings;

    AkNanoInitSettings(&aknano_settings);
    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = AkNano_ConnectToDevicesGateway(&xNetworkContext, &xTransportInterface);
    TEST_ASSERT(xDemoStatus == pdPASS);

    xDemoStatus = AkNano_GetTargets(&xTransportInterface, &aknano_settings, &xResponse);
    TEST_ASSERT(xDemoStatus == pdPASS);
    TEST_ASSERT(xResponse.statusCode == 200);

    /* Close the network connection.  */
    SecureSocketsTransport_Disconnect( &xNetworkContext );

    TEST_ASSERT(xDemoStatus == pdPASS);
}

int RunAkNanoTest( void )
{

    // InitFlashStorage();

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
