/**
 * @file wake_word.h
 * @brief Module phát hiện từ khóa đánh thức (Wake Word Detection)
 *
 * Lấy logic từ dự án foxrov/xiaozhi, viết lại sạch hoàn toàn bằng C thuần
 * cho ESP-IDF 5.4.x + esp-sr v1.9.x
 *
 * Pipeline:
 *   I2S Mic (INMP441) → AFE (Noise Suppression + AGC) → WakeNet → Callback
 *
 * Khi phát hiện từ khóa, callback đã đăng ký sẽ được gọi từ Task FreeRTOS
 * chuyên dụng (không chặn luồng chính).
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Prototype hàm callback khi phát hiện Wake Word
 *        Hàm này được gọi từ Task nội bộ (không phải app_main)
 */
typedef void (*wake_word_callback_t)(void);

/**
 * @brief Khởi tạo toàn bộ pipeline Wake Word Detection
 *
 * Thực hiện:
 *   1. Khởi tạo I2S RX channel (Mic INMP441) theo cấu hình Kconfig
 *   2. Khởi tạo AFE (Audio Front-End) với WakeNet
 *   3. Tạo 2 FreeRTOS Task: audio_feed và audio_detect
 *   4. Đăng ký callback được gọi khi phát hiện từ khóa
 *
 * @param cb   Hàm callback sẽ được gọi khi Wake Word được phát hiện
 * @return     ESP_OK nếu thành công, mã lỗi esp_err_t nếu thất bại
 */
esp_err_t wake_word_init(wake_word_callback_t cb);

/**
 * @brief Bắt đầu quá trình lắng nghe Wake Word
 *        Kích hoạt sau khi wake_word_init() thành công
 */
void wake_word_start(void);

/**
 * @brief Dừng toàn bộ pipeline (để thiết bị chuẩn bị vào deep sleep)
 *        Giải phóng I2S để tiết kiệm điện tối đa trước khi gọi esp_deep_sleep_start()
 */
void wake_word_stop(void);

#ifdef __cplusplus
}
#endif
