/**
 * @file rgb_led.h
 * @brief Điều khiển LED RGB WS2812B tích hợp trên mạch ESP32-S3-DevKitC-1
 *
 * ESP32-S3-DevKitC-1 có 1 LED RGB WS2812B gắn sẵn trên GPIO 48.
 * WS2812B là LED địa chỉ hóa (addressable) dùng giao thức 1-wire NZR,
 * được điều khiển qua ngoại vi RMT (Remote Control Transceiver) của ESP32-S3.
 *
 * Driver sử dụng: espressif/led_strip (ESP-IDF 5.x built-in component)
 *
 * Màu sắc được biểu diễn bằng: (R, G, B) mỗi thành phần 0–255
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Khởi tạo LED WS2812B trên GPIO 48 qua RMT
 *        Phải gọi trước khi dùng các hàm rgb_led_*
 * @return ESP_OK nếu thành công
 */
esp_err_t rgb_led_init(void);

/**
 * @brief Đặt màu LED theo giá trị R, G, B
 * @param r  Cường độ đỏ   (0–255)
 * @param g  Cường độ xanh lá (0–255)
 * @param b  Cường độ xanh dương (0–255)
 */
void rgb_led_set(uint8_t r, uint8_t g, uint8_t b);

/** @brief Tắt LED (đen hoàn toàn) */
void rgb_led_off(void);

/** @brief XANH DƯƠNG - Đang đánh thức / kết nối (Wake Word detected) */
void rgb_led_set_blue(void);

/** @brief XANH LÁ - Sẵn sàng / bình thường (Idle / OK) */
void rgb_led_set_green(void);

/** @brief ĐỎ - Đang lắng nghe / cảnh báo (Listening / Error) */
void rgb_led_set_red(void);

/** @brief TRẮNG - Đang khởi động (Booting) */
void rgb_led_set_white(void);

/** @brief VÀNG - Đang nâng cấp OTA */
void rgb_led_set_yellow(void);

#ifdef __cplusplus
}
#endif
