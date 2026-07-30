#include "led.h"

#include "led_strip.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h"

// Chân GPIO nối với đèn LED
#define LED_GPIO_NUM 46

// Số lượng LED trên strip
#define LED_COUNT 1

// Độ sáng của LED theo phần trăm
#define LED_BRIGHTNESS_PERCENT 20

// Tần số chạy của RMT, dùng để điều khiển LED WS2812
#define LED_RMT_RESOLUTION_HZ 10000000

// Handle để giữ trạng thái của dải LED
static led_strip_handle_t strip = NULL;

// Khởi tạo LED strip một lần duy nhất
static void rgb_led_init(void)
{
    if (strip != NULL) {
        return;
    }

    // Cấu hình chung cho LED strip
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO_NUM,
        .max_leds = LED_COUNT,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = 0,
        },
    };

    // Cấu hình cho RMT, là phần cứng ESP-IDF dùng để điều khiển LED
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_RMT_RESOLUTION_HZ,
        .mem_block_symbols = 0,
        .flags = {
            .with_dma = true,
        },
    };

    // Tạo thiết bị LED strip
    if (led_strip_new_rmt_device(&strip_config, &rmt_config, &strip) != ESP_OK) {
        return;
    }

    // Xóa trạng thái cũ để LED bắt đầu ở trạng thái tắt
    led_strip_clear(strip);
}

// Đặt màu cho LED đơn
static void led_set_color(uint32_t red, uint32_t green, uint32_t blue)
{
    if (strip == NULL) {
        return;
    }

    // Nhân với độ sáng để màu không quá gắt
    led_strip_set_pixel(strip, 0, red * LED_BRIGHTNESS_PERCENT / 100, green * LED_BRIGHTNESS_PERCENT / 100, blue * LED_BRIGHTNESS_PERCENT / 100);

    // Gửi dữ liệu màu mới ra phần cứng
    led_strip_refresh(strip);
}

void rgb_led_off(void)
{
    rgb_led_init();
    led_set_color(0, 0, 0);
}

void rgb_led_on(void)
{
    rgb_led_init();
    led_set_color(0, 255, 0);
}