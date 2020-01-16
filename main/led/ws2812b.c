#include "ws2812b.h"

#include "stdint.h"
#include "driver/rmt.h"

#define WS2812B_CLK_DIV 4       // 80 MHz clock, div 4, 20 MHz tick frequency, 0.05 us per tick
#define WS2812B_T0H_TICK 7      // 0.35 us
#define WS2812B_T1H_TICK 18     // 0.9 us
#define WS2812B_T0L_TICK 18     // 0.9 us
#define WS2812B_T1L_TICK 7      // 0.35 us
#define WS2812B_RESET_TICK 1200 // 60 us (>50 us in manual)

const rmt_item32_t t0_bit = {{{WS2812B_T0H_TICK, 1, WS2812B_T0L_TICK, 0}}};
const rmt_item32_t t1_bit = {{{WS2812B_T1H_TICK, 1, WS2812B_T1L_TICK, 0}}};

static void ws2812b_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num)
{
    if (src == NULL || dest == NULL)
    {
        *translated_size = 0;
        *item_num = 0;
        return;
    }
    size_t size = 0;
    size_t num = 0;
    uint8_t *psrc = (uint8_t *)src;
    rmt_item32_t *pdest = dest;
    while (size < src_size && num < wanted_num)
    {
        for (int i = 0; i < 8; i++)
        {
            // MSB first
            if (*psrc & (1 << (7 - i)))
            {
                pdest->val = t1_bit.val;
            }
            else
            {
                pdest->val = t0_bit.val;
            }
            num++;
            pdest++;
        }
        size++;
        psrc++;
    }
    *translated_size = size;
    *item_num = num;
}

void ws2812b_init_channel(rmt_channel_t channel, int gpio)
{
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_TX;
    config.channel = channel;
    config.gpio_num = gpio;
    config.mem_block_num = 1;
    config.clk_div = WS2812B_CLK_DIV;

    config.tx_config.loop_en = 0;
    config.tx_config.carrier_en = 0;
    config.tx_config.idle_output_en = 1;
    config.tx_config.idle_level = 0;

    rmt_config(&config);
    rmt_driver_install(channel, 0, 0);

    rmt_translator_init(channel, *ws2812b_rmt_adapter);
}

void ws2812b_set_pixel(ws2812b_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue)
{
    ws2812b_t *ws2812b = __containerof(strip, ws2812b_t, parent);
    uint32_t start = index * 3;
    // In thr order of GRB
    ws2812b->buffer[start + 0] = green & 0xFF;
    ws2812b->buffer[start + 1] = red & 0xFF;
    ws2812b->buffer[start + 2] = blue & 0xFF;
}

void ws2812b_refresh(ws2812b_strip_t *strip, uint32_t timeout_ms)
{
    ws2812b_t *ws2812b = __containerof(strip, ws2812b_t, parent);
    rmt_write_sample(ws2812b->config.channel, ws2812b->buffer, ws2812b->config.num_leds * 3, true);
}

void ws2812b_clear(ws2812b_strip_t *strip, uint32_t timeout_ms)
{
    ws2812b_t *ws2812b = __containerof(strip, ws2812b_t, parent);
    memset(ws2812b->buffer, 0, ws2812b->config.num_leds * 3);
    return ws2812b_refresh(strip, timeout_ms);
}

void ws2812b_del(ws2812b_strip_t *strip)
{
    ws2812b_t *ws2812b = __containerof(strip, ws2812b_t, parent);
    free(ws2812b);
}

ws2812b_strip_t ws2812b_init_strip(ws2812b_strip_config_t config)
{
    uint32_t ws2812b_size = sizeof(ws2812b_t) + config.num_leds * 3;
    ws2812b_t *ws2812b = calloc(1, ws2812b_size);

    ws2812b->config = config;
    ws2812b->parent.set_pixel = ws2812b_set_pixel;
    ws2812b->parent.refresh = ws2812b_refresh;
    ws2812b->parent.clear = ws2812b_clear;
    ws2812b->parent.del = ws2812b_del;
}

typedef struct
{
    ws2812b_strip_t parent;
    ws2812b_strip_config_t config;
    uint8_t buffer[0];
} ws2812b_t;
