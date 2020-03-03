#include "FreeRTOS.h"
#include "led/patterns.h"

void app_main() {
    vTaskCreate(ledMetaTask, "LedMetaTask", 1000, NULL, 5, NULL);
}