/**
 * @file main.c
 * @brief Foxro - Smart Motorcycle Platform v0.1.0
 *        Điểm khởi chạy ứng dụng và vòng lặp điều phối chính
 *
 * Luồng hoạt động (State Machine):
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                    KHỞI ĐỘNG (Boot)                        │
 *   │  Khởi tạo NVS → RGB LED → I2S Mic → AFE WakeNet           │
 *   └─────────────────────────────┬───────────────────────────────┘
 *                                 │
 *                                 ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │               TRẠNG THÁI: NGỦ CHỜ (Light Sleep / IDLE)    │
 *   │  LED tắt - Mic + AFE chạy nền lắng nghe từ khóa           │
 *   └─────────────────────────────┬───────────────────────────────┘
 *                                 │
 *                 Phát hiện Wake Word ("Nihao Xiaozhi")
 *                                 │
 *                                 ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │               TRẠNG THÁI: ĐÁNH THỨC (WOKEN)               │
 *   │  LED RGB bật XANH DƯƠNG 2–4 giây (hiển thị được đánh thức)│
 *   └─────────────────────────────┬───────────────────────────────┘
 *                                 │
 *                     Hết thời gian LED
 *                                 │
 *                                 ▼
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │               TRẠNG THÁI: NGỦ SÂU (Deep Sleep)            │
 *   │  LED tắt - Dừng I2S - Vào esp_deep_sleep_start()          │
 *   │  Đánh thức bởi: GPIO Wake (nút bấm) hoặc ULP Timer        │
 *   └─────────────────────────────────────────────────────────────┘
 *               ▲
 *               │ Chip reboot sau deep sleep
 *               └─────────────────────────────────────────────────
 *
 * Ghi chú:
 *   - Wake Word model được chứa trong partition "model" (SPIFFS, 1MB)
 *   - Partition table cần có entry "model" - xem partitions.csv
 *   - Cấu hình GPIO trong file này và rgb_led.c / wake_word.c
 */

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_sleep.h"
#include "esp_system.h"

#include "rgb_led.h"
#include "wake_word.h"

/* ── Tag log ────────────────────────────────────────────────────────────── */
static const char *TAG = "foxro_main";

/* ── Hằng số hành vi ───────────────────────────────────────────────────── */
/** Thời gian sáng LED sau khi đánh thức (ms) - nằm trong [2000, 4000] ms  */
#define LED_ON_DURATION_MS    3000

/* ── EventGroup giao tiếp giữa callback và app_main ───────────────────── */
#define WAKE_DETECTED_BIT   BIT0
static EventGroupHandle_t s_app_event_group = NULL;

/* ── Prototype hàm cục bộ ──────────────────────────────────────────────── */
static void on_wake_word_detected(void);
static void nvs_init(void);


/* ══════════════════════════════════════════════════════════════════════════
 * CALLBACK: Được gọi từ Task audio_detect khi Wake Word được phát hiện
 * ══════════════════════════════════════════════════════════════════════════ */
static void on_wake_word_detected(void)
{
    /* QUAN TRỌNG: Đây là callback từ Task FreeRTOS nội bộ (audio_detect).
     * Không thực hiện tác vụ nặng tại đây.
     * Chỉ set EventBit để app_main xử lý ở vòng lặp chính.                */
    ESP_LOGI(TAG, "[Callback] Wake word detected! Setting event bit...");
    if (s_app_event_group != NULL) {
        xEventGroupSetBitsFromISR(s_app_event_group, WAKE_DETECTED_BIT, NULL);
    }
}


/* ══════════════════════════════════════════════════════════════════════════
 * NVS: Khởi tạo bộ nhớ NVS Flash
 *   NVS cần cho nhiều component ESP-IDF (Wi-Fi, esp-sr, ...)
 * ══════════════════════════════════════════════════════════════════════════ */
static void nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* Xóa và khởi tạo lại nếu NVS bị đầy hoặc có phiên bản mới */
        ESP_LOGW(TAG, "NVS flash erasing and reinitializing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");
}


/* ══════════════════════════════════════════════════════════════════════════
 * APP_MAIN: Điểm khởi chạy chính của ứng dụng ESP-IDF
 * ══════════════════════════════════════════════════════════════════════════ */
void app_main(void)
{
    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, " Foxro - Smart Motorcycle Platform v0.1.0  ");
    ESP_LOGI(TAG, "=============================================");

    /* ── BƯỚC 1: Kiểm tra nguyên nhân reboot/wakeup ─────────────────────── */
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        ESP_LOGI(TAG, "Boot reason: Normal power-on / Reset");
    } else {
        ESP_LOGI(TAG, "Boot reason: Wakeup from deep sleep (cause=%d)", wakeup_reason);
    }

    /* ── BƯỚC 2: Khởi tạo NVS Flash ─────────────────────────────────────── */
    nvs_init();

    /* ── BƯỚC 3: Khởi tạo LED RGB ───────────────────────────────────────── */
    rgb_led_init();

    /* Nhấp nháy trắng 1 lần khi khởi động để báo hiệu hệ thống sẵn sàng   */
    rgb_led_set_white();
    vTaskDelay(pdMS_TO_TICKS(300));
    rgb_led_off();
    vTaskDelay(pdMS_TO_TICKS(200));

    /* ── BƯỚC 4: Tạo EventGroup cho giao tiếp app_main ← callback ──────── */
    s_app_event_group = xEventGroupCreate();
    if (s_app_event_group == NULL) {
        ESP_LOGE(TAG, "CRITICAL: Failed to create app EventGroup - rebooting...");
        esp_restart();
    }

    /* ── BƯỚC 5: Khởi tạo Wake Word Detection ───────────────────────────── */
    ESP_LOGI(TAG, "Initializing wake word detection pipeline...");
    esp_err_t ret = wake_word_init(on_wake_word_detected);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CRITICAL: Wake word init failed (err=0x%x) - rebooting...", ret);
        /* Chớp đèn đỏ 3 lần báo lỗi trước khi reboot */
        for (int i = 0; i < 3; i++) {
            rgb_led_set_red();
            vTaskDelay(pdMS_TO_TICKS(300));
            rgb_led_off();
            vTaskDelay(pdMS_TO_TICKS(300));
        }
        esp_restart();
    }

    /* ── BƯỚC 6: Bắt đầu lắng nghe từ khóa đánh thức ───────────────────── */
    wake_word_start();
    ESP_LOGI(TAG, "System ready. Listening for wake word...");

    /* Chớp xanh lá 2 lần → báo hiệu đang lắng nghe (IDLE state)           */
    for (int i = 0; i < 2; i++) {
        rgb_led_set_green();
        vTaskDelay(pdMS_TO_TICKS(150));
        rgb_led_off();
        vTaskDelay(pdMS_TO_TICKS(150));
    }

    /* ── VÒNG LẶP CHÍNH: Chờ sự kiện Wake Word ─────────────────────────── */
    while (true) {
        /* Chờ blocking vô thời hạn đến khi callback set WAKE_DETECTED_BIT  */
        EventBits_t bits = xEventGroupWaitBits(
            s_app_event_group,
            WAKE_DETECTED_BIT,
            pdTRUE,          /* Tự động clear bit sau khi nhận               */
            pdFALSE,         /* Chỉ cần 1 bit (không cần tất cả)             */
            portMAX_DELAY    /* Chờ vô thời hạn                              */
        );

        if (bits & WAKE_DETECTED_BIT) {
            /* ════════════════════════════════════════════════════════════
             * XỬ LÝ: Wake Word đã được phát hiện
             * ════════════════════════════════════════════════════════════ */
            ESP_LOGI(TAG, ">>> WAKE WORD DETECTED! Activating LED for %dms <<<",
                     LED_ON_DURATION_MS);

            /* --- HÀNH ĐỘNG 1: Bật LED XANH DƯƠNG (đang đánh thức) ------ */
            rgb_led_set_blue();

            /* Duy trì LED sáng trong LED_ON_DURATION_MS ms               */
            vTaskDelay(pdMS_TO_TICKS(LED_ON_DURATION_MS));

            /* --- HÀNH ĐỘNG 2: Tắt LED ----------------------------------- */
            rgb_led_off();
            ESP_LOGI(TAG, "LED OFF. Preparing to enter deep sleep...");

            /* --- HÀNH ĐỘNG 3: Dừng pipeline âm thanh -------------------- */
            wake_word_stop();
            vTaskDelay(pdMS_TO_TICKS(100));  /* Cho các Task có thời gian dừng hẳn */

            /* --- HÀNH ĐỘNG 4: Cấu hình nguồn đánh thức Deep Sleep ------- */
            /* Tùy chọn A: Không cấu hình nguồn đánh thức (deep sleep vô thời hạn)
             *             → Chỉ reset cứng (EN pin) mới wake up được
             *             → Dùng khi muốn tiết kiệm điện hoàn toàn        */

            /* Tùy chọn B: Timer 30 giây → tự động thức dậy để lắng nghe lại
             * esp_sleep_enable_timer_wakeup(30ULL * 1000 * 1000);         */

            /* Tùy chọn C: GPIO wakeup (nút bấm vật lý GPIO 0 - Active Low) */
            /* esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);                */

            /* Hiện tại: Timer 30 giây để demo vòng lặp liên tục           */
            ESP_LOGI(TAG, "Configuring wakeup timer: 30 seconds");
            esp_sleep_enable_timer_wakeup(30ULL * 1000ULL * 1000ULL);

            /* --- HÀNH ĐỘNG 5: Vào Deep Sleep ----------------------------- */
            ESP_LOGI(TAG, "Entering deep sleep now. Goodbye!");
            vTaskDelay(pdMS_TO_TICKS(50));  /* Flush log UART buffer         */
            esp_deep_sleep_start();

            /* *** Code sau đây KHÔNG BAO GIỜ thực thi ***
             * Sau deep_sleep_start(), chip bị tắt nguồn và chỉ boot lại
             * từ đầu khi có sự kiện wakeup (timer, GPIO, ...)              */
        }
    }

    /* Không bao giờ tới đây */
}
