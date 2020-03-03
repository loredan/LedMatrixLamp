#include "driver/rmt.h"

typedef struct
{
    void (*set_pixel)(ws2812b_strip_controller_t *strip, uint32_t index, uint8_t red, uint8_t green, uint8_t blue);
    void (*refresh)(ws2812b_strip_controller_t *strip);
    void (*clear)(ws2812b_strip_controller_t *strip);
    void (*del)(ws2812b_strip_controller_t *strip);
} ws2812b_strip_controller_t;

typedef struct
{
    uint32_t num_leds;
    rmt_channel_t channel;
} ws2812b_strip_config_t;

void ws2812b_init_channel(rmt_channel_t channel, int gpio);
ws2812b_strip_controller_t ws2812b_init_strip(ws2812b_strip_config_t config);
