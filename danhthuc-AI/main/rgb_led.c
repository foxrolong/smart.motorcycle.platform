/**
 * @file rgb_led.c
 * @brief Điều khiển LED RGB WS2812B tích hợp trên mạch qua RMT
 *
 * Phần cứng:
 *   - LED WS2812B trên GPIO 48 (ESP32-S3-DevKitC-1 onboard RGB LED)
 *   - Giao thức: 1-wire NZR, điều khiển qua ESP32-S3 RMT peripheral
 *
 * Driver:
 *   - Dùng `espressif/led_strip` component (có sẵn trong ESP-IDF 5.x)
 *   - API: led_strip_new_rmt_device() → led_strip_set_pixel() → led_strip_refresh()
 *
 * Lưu ý cường độ sáng:
 *   - WS2812B rất sáng khi dùng giá trị 255.
 *   - Hàm set_color() áp dụng hệ số BRIGHTNESS (0–255) để giảm độ chói.
 *   - Mặc định BRIGHTNESS = 20 (≈ 8% công suất) - đủ nhìn thấy, không gây chói.
 */

#include "rgb_led.h"

#include <string.h>
#include "esp_log.h"
#include "led_strip.h"         /* espressif/led_strip - có trong ESP-IDF 5.x */
#include "led_strip_types.h"

static const char *TAG = "rgb_led";

/* ── Cấu hình phần cứng ────────────────────────────────────────────────── */
#define WS2812_GPIO_NUM     GPIO_NUM_48   /* GPIO 48 - WS2812B onboard LED   */
#define WS2812_LED_COUNT    1             /* Chỉ có 1 LED trên DevKitC-1     */
#define WS2812_RMT_RES_HZ   10000000     /* 10MHz resolution cho RMT         */
/* ──────────────────────────────────────────────────────────────────────── */

/* ── Cường độ sáng tổng thể (0–255) ────────────────────────────────────── */
/* 20 ≈ 8% → nhìn thấy rõ trong nhà, không gây chói.
 * Tăng lên 50–80 nếu dùng ngoài trời / ánh sáng mạnh.                    */
#define BRIGHTNESS          20
/* ──────────────────────────────────────────────────────────────────────── */

/* ── Con trỏ LED strip handle (static - nội bộ module) ─────────────────── */
static led_strip_handle_t s_led_strip = NULL;
/* ──────────────────────────────────────────────────────────────────────── */


/* ── Hàm nội bộ: scale màu theo hệ số BRIGHTNESS ──────────────────────── */
static inline uint8_t scale_brightness(uint8_t value)
{
    /* Nhân màu với hệ số (0–255)/255 để giảm độ sáng tổng thể             */
    return (uint8_t)(((uint16_t)value * BRIGHTNESS) / 255);
}


/* ══════════════════════════════════════════════════════════════════════════
 * PUBLIC: Khởi tạo LED WS2812B qua RMT
 * ══════════════════════════════════════════════════════════════════════════ */
esp_err_t rgb_led_init(void)
{
    /* Cấu hình LED Strip driver qua RMT */
    led_strip_config_t strip_cfg = {
        .strip_gpio_num   = WS2812_GPIO_NUM,
        .max_leds         = WS2812_LED_COUNT,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,  /* WS2812B dùng thứ tự GRB */
        .led_model        = LED_MODEL_WS2812,
        .flags = {
            .invert_out = false,                   /* Không đảo logic tín hiệu */
        },
    };

    /* Cấu hình backend RMT */
    led_strip_rmt_config_t rmt_cfg = {
        .clk_src       = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = WS2812_RMT_RES_HZ,
        .mem_block_symbols = 64,          /* Số symbol RMT trong 1 block DMA */
        .flags = {
            .with_dma = false,            /* Không cần DMA cho 1 LED          */
        },
    };

    esp_err_t ret = led_strip_new_rmt_device(&strip_cfg, &rmt_cfg, &s_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip RMT device (err=0x%x)", ret);
        return ret;
    }

    /* Tắt LED ngay sau khi khởi tạo (trạng thái sạch) */
    rgb_led_off();

    ESP_LOGI(TAG, "WS2812B LED initialized on GPIO%d (brightness=%d/255)",
             WS2812_GPIO_NUM, BRIGHTNESS);
    return ESP_OK;
}


/* ══════════════════════════════════════════════════════════════════════════
 * PUBLIC: Đặt màu LED (R, G, B) với hệ số BRIGHTNESS
 * ══════════════════════════════════════════════════════════════════════════ */
void rgb_led_set(uint8_t r, uint8_t g, uint8_t b)
{
    if (s_led_strip == NULL) {
        ESP_LOGW(TAG, "rgb_led_set() called before rgb_led_init()");
        return;
    }

    /* Áp dụng hệ số sáng để tránh LED quá chói */
    uint8_t sr = scale_brightness(r);
    uint8_t sg = scale_brightness(g);
    uint8_t sb = scale_brightness(b);

    /* Ghi màu vào pixel 0 (LED đầu tiên và duy nhất) */
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, sr, sg, sb));

    /* Refresh: truyền dữ liệu màu qua RMT đến LED WS2812B              */
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}


/* ══════════════════════════════════════════════════════════════════════════
 * PUBLIC: Tắt LED hoàn toàn
 * ══════════════════════════════════════════════════════════════════════════ */
void rgb_led_off(void)
{
    if (s_led_strip == NULL) {
        return;
    }
    /* led_strip_clear() đặt tất cả pixel về (0,0,0) và refresh ngay     */
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));
}


/* ══════════════════════════════════════════════════════════════════════════
 * PUBLIC: Các màu đặt sẵn (shortcut functions)
 * ══════════════════════════════════════════════════════════════════════════ */
void rgb_led_set_blue(void)
{
    rgb_led_set(0, 0, 255);   /* Xanh dương thuần - Wake Word detected    */
}

void rgb_led_set_green(void)
{
    rgb_led_set(0, 255, 0);   /* Xanh lá thuần - Idle / OK               */
}

void rgb_led_set_red(void)
{
    rgb_led_set(255, 0, 0);   /* Đỏ thuần - Listening / Error             */
}

void rgb_led_set_white(void)
{
    rgb_led_set(255, 255, 255); /* Trắng - Booting                        */
}

void rgb_led_set_yellow(void)
{
    rgb_led_set(255, 200, 0);  /* Vàng - OTA Upgrading                    */
}
