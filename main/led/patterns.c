#include "patterns.h"

#include "FreeRTOS.h"
#include "driver/rmt.h"
#include "ws2812b.h"

#define LED_PIN 4
#define NUM_LEDS 5

typedef struct
{
    ws2812b_strip_controller_t *controller;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} solidColorTaskParams;

void ledMetaTask(void *pvParameters)
{
    ws2812b_init_channel(RMT_CHANNEL_0, LED_PIN);
    ws2812b_strip_config_t config = {NUM_LEDS, RMT_CHANNEL_0};
    ws2812b_strip_controller_t controller = ws2812b_init_strip(config);

    vTaskCreate(solidColorTask, "solidColorTask", 1000, (void *)&controller, 10, NULL);
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void solidColorTask(void *pvParameters)
{
    solidColorTaskParams params = *(solidColorTaskParams *)pvParameters;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        params.controller->set_pixel(params.controller, i, params.red, params.green, params.blue);
    }
    params.controller->refresh(params.controller);
    vTaskDelete(NULL);
}