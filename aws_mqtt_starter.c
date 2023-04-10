#ifdef AKNANO_ENABLE_AWS_MQTT_DEMO_TASK
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

void aws_mqtt_starter()
{
    /* Wait for a few seconds, while aknano initializes */
    vTaskDelay(pdMS_TO_TICKS(5000));
    RunCoreMqttAgentDemo(false, NULL, NULL, NULL, NULL);
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
#endif
