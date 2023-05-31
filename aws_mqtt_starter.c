#ifdef AKNANO_ENABLE_AWS_MQTT_DEMO_TASK
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "aknano_public_api.h"

int RunCoreMqttAgentDemo(bool awsIotMqttMode,
                         const char *pIdentifier,
                         void *pNetworkServerInfo,
                         void *pNetworkCredentialInfo,
                         const void *pNetworkInterface);

void aws_mqtt_starter()
{
    /* Wait for aknano to initialize */
    while (!aknano_is_initialized())
        vTaskDelay(pdMS_TO_TICKS(1000));

    RunCoreMqttAgentDemo(false, NULL, NULL, NULL, NULL);
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
#endif
