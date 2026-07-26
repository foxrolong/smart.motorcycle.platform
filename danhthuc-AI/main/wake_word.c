/**
 * @file wake_word.c
 * @brief Thực thi phát hiện từ khóa đánh thức (Wake Word Detection)
 *
 * Kiến trúc 2 Task FreeRTOS (lấy tư duy từ foxrov/xiaozhi, viết lại C thuần):
 *
 *   ┌──────────────────────────────────────────────────────────────┐
 *   │ Task: audio_feed (Core 0, Priority 5)                       │
 *   │  • Đọc PCM liên tục từ I2S Mic (INMP441, 16kHz, 16-bit)    │
 *   │  • Đẩy vào AFE bằng esp_afe_sr_v1.feed()                   │
 *   └──────────────────────────┬───────────────────────────────────┘
 *                              │ PCM Data
 *                              ▼
 *   ┌──────────────────────────────────────────────────────────────┐
 *   │ AFE (Audio Front-End):                                      │
 *   │  • Noise Suppression (SSP/NSNet)                            │
 *   │  • AGC (Automatic Gain Control)                             │
 *   │  • WakeNet inference engine                                 │
 *   └──────────────────────────┬───────────────────────────────────┘
 *                              │ Processed frames
 *                              ▼
 *   ┌──────────────────────────────────────────────────────────────┐
 *   │ Task: audio_detect (Core 1, Priority 1)                     │
 *   │  • Gọi esp_afe_sr_v1.fetch() để lấy kết quả                │
 *   │  • Kiểm tra res->wakeup_state == WAKENET_DETECTED           │
 *   │  • Nếu phát hiện → gọi user callback → set EventBits       │
 *   └──────────────────────────────────────────────────────────────┘
 *
 * Cấu hình GPIO I2S trong Kconfig.projbuild (xem cuối file)
 */

#include "wake_word.h"

#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* esp-sr headers */
#include "esp_afe_sr_models.h"
#include "model_path.h"

static const char *TAG = "wake_word";

/* ── Cấu hình I2S Microphone (INMP441 hoặc tương đương) ─────────────────── */
/* Đổi các hằng số GPIO theo sơ đồ phần cứng board của bạn                   */
#define MIC_I2S_PORT         I2S_NUM_0
#define MIC_SAMPLE_RATE      16000           /* Hz - chuẩn ASR               */
#define MIC_BCLK_GPIO        GPIO_NUM_5      /* I2S Bit Clock  (SCK INMP441) */
#define MIC_WS_GPIO          GPIO_NUM_4      /* I2S Word Select (WS INMP441) */
#define MIC_DIN_GPIO         GPIO_NUM_6      /* I2S Data In    (DIN INMP441) */
/* ──────────────────────────────────────────────────────────────────────── */

/* ── EventGroup bits ───────────────────────────────────────────────────── */
#define DETECTION_RUNNING_BIT   BIT0    /* 1 = đang chạy phát hiện          */
/* ──────────────────────────────────────────────────────────────────────── */

/* ── Biến nội bộ (static - chỉ dùng trong file này) ────────────────────── */
static esp_afe_sr_data_t   *s_afe_data    = NULL;
static i2s_chan_handle_t    s_rx_handle   = NULL;
static EventGroupHandle_t   s_event_group = NULL;
static wake_word_callback_t s_user_cb     = NULL;

static TaskHandle_t s_feed_task_handle   = NULL;
static TaskHandle_t s_detect_task_handle = NULL;
/* ──────────────────────────────────────────────────────────────────────── */


/* ══════════════════════════════════════════════════════════════════════════
 * STATIC: Khởi tạo I2S channel RX cho Microphone
 * ══════════════════════════════════════════════════════════════════════════ */
static esp_err_t s_i2s_mic_init(void)
{
    /* 1. Tạo I2S channel RX (chỉ thu âm, không phát) */
    i2s_chan_config_t chan_cfg = {
        .id              = MIC_I2S_PORT,
        .role            = I2S_ROLE_MASTER,
        .dma_desc_num    = 6,
        .dma_frame_num   = 240,
        .auto_clear_after_cb  = false,
        .auto_clear_before_cb = false,
        .intr_priority   = 0,
    };
    /* Chỉ tạo kênh RX (tham số tx_handle = NULL) */
    ESP_RETURN_ON_ERROR(
        i2s_new_channel(&chan_cfg, NULL, &s_rx_handle),
        TAG, "Failed to create I2S RX channel"
    );

    /* 2. Cấu hình chuẩn I2S Standard Mode cho Mic INMP441
     *    - INMP441 xuất PCM 24-bit trong slot 32-bit
     *    - Chúng ta đọc 32-bit và dịch phải 16 bit khi đọc ra  */
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = MIC_SAMPLE_RATE,
            .clk_src        = I2S_CLK_SRC_DEFAULT,
            .ext_clk_freq_hz = 0,
            .mclk_multiple  = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_32BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode      = I2S_SLOT_MODE_MONO,
            .slot_mask      = I2S_STD_SLOT_LEFT,
            .ws_width       = I2S_DATA_BIT_WIDTH_32BIT,
            .ws_pol         = false,
            .bit_shift      = true,
            .left_align     = true,
            .big_endian     = false,
            .bit_order_lsb  = false,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = MIC_BCLK_GPIO,
            .ws   = MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,   /* RX only - không cần DOUT */
            .din  = MIC_DIN_GPIO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_RETURN_ON_ERROR(
        i2s_channel_init_std_mode(s_rx_handle, &std_cfg),
        TAG, "Failed to init I2S STD mode"
    );

    ESP_RETURN_ON_ERROR(
        i2s_channel_enable(s_rx_handle),
        TAG, "Failed to enable I2S RX channel"
    );

    ESP_LOGI(TAG, "I2S Mic initialized: %dHz, BCLK=GPIO%d WS=GPIO%d DIN=GPIO%d",
             MIC_SAMPLE_RATE, MIC_BCLK_GPIO, MIC_WS_GPIO, MIC_DIN_GPIO);
    return ESP_OK;
}


/* ══════════════════════════════════════════════════════════════════════════
 * TASK: audio_feed - Đọc PCM từ Mic và đẩy vào AFE
 *   Core 0, Priority 5 (cao hơn detect để đảm bảo dữ liệu luôn kịp feed)
 * ══════════════════════════════════════════════════════════════════════════ */
static void audio_feed_task(void *arg)
{
    /* Lấy kích thước chunk mà AFE yêu cầu mỗi lần feed (đơn vị: mẫu) */
    int feed_chunk_size = esp_afe_sr_v1.get_feed_chunksize(s_afe_data);

    /* Mỗi mẫu Mic thô là int32_t (32-bit từ I2S), sau đó dịch phải 16-bit
     * để có int16_t chuẩn 16-bit cho AFE                                    */
    int32_t *i2s_buf = heap_caps_malloc(feed_chunk_size * sizeof(int32_t), MALLOC_CAP_INTERNAL);
    int16_t *pcm_buf = heap_caps_malloc(feed_chunk_size * sizeof(int16_t), MALLOC_CAP_INTERNAL);

    if (!i2s_buf || !pcm_buf) {
        ESP_LOGE(TAG, "Failed to allocate audio buffers");
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "audio_feed_task started, chunk=%d samples", feed_chunk_size);

    while (true) {
        /* Chờ cho đến khi phát hiện đang được kích hoạt */
        xEventGroupWaitBits(s_event_group, DETECTION_RUNNING_BIT,
                            pdFALSE, pdTRUE, portMAX_DELAY);

        /* Đọc dữ liệu PCM từ I2S (blocking) */
        size_t bytes_read = 0;
        esp_err_t ret = i2s_channel_read(s_rx_handle,
                                         i2s_buf,
                                         feed_chunk_size * sizeof(int32_t),
                                         &bytes_read,
                                         portMAX_DELAY);
        if (ret != ESP_OK || bytes_read == 0) {
            continue;
        }

        /* Chuyển đổi I2S 32-bit → PCM 16-bit
         * INMP441 xuất dữ liệu âm thanh ở 24 bit cao của word 32-bit
         * Dịch phải 16 bit để lấy 16-bit có dấu chuẩn                      */
        int samples = (int)(bytes_read / sizeof(int32_t));
        for (int i = 0; i < samples; i++) {
            pcm_buf[i] = (int16_t)(i2s_buf[i] >> 16);
        }

        /* Đẩy PCM vào AFE pipeline */
        esp_afe_sr_v1.feed(s_afe_data, pcm_buf);
    }

    /* Không nên tới đây, nhưng giải phóng bộ nhớ nếu thoát */
    free(i2s_buf);
    free(pcm_buf);
    vTaskDelete(NULL);
}


/* ══════════════════════════════════════════════════════════════════════════
 * TASK: audio_detect - Lấy kết quả từ AFE và kiểm tra Wake Word
 *   Core 1, Priority 1 (thấp hơn feed để AFE pipeline không bị đói dữ liệu)
 * ══════════════════════════════════════════════════════════════════════════ */
static void audio_detect_task(void *arg)
{
    ESP_LOGI(TAG, "audio_detect_task started");

    while (true) {
        /* Chờ cho đến khi phát hiện đang được kích hoạt */
        xEventGroupWaitBits(s_event_group, DETECTION_RUNNING_BIT,
                            pdFALSE, pdTRUE, portMAX_DELAY);

        /* Lấy frame đã xử lý từ AFE (blocking - AFE tự block đến khi có frame) */
        afe_fetch_result_t *res = esp_afe_sr_v1.fetch(s_afe_data);
        if (res == NULL || res->ret_value == ESP_FAIL) {
            /* Lỗi từ AFE - bỏ qua frame này */
            continue;
        }

        /* Kiểm tra kết quả phát hiện Wake Word */
        if (res->wakeup_state == WAKENET_DETECTED) {
            ESP_LOGI(TAG, "*** WAKE WORD DETECTED! ***");

            /* Dừng vòng lặp phát hiện để tránh kích hoạt lại liên tục */
            xEventGroupClearBits(s_event_group, DETECTION_RUNNING_BIT);

            /* Gọi callback của người dùng (nếu đã đăng ký) */
            if (s_user_cb != NULL) {
                s_user_cb();
            }
            /* Task tự nghỉ sau khi gọi callback -
             * wake_word_start() sẽ kích hoạt lại nếu cần              */
        }
    }

    vTaskDelete(NULL);
}


/* ══════════════════════════════════════════════════════════════════════════
 * PUBLIC API
 * ══════════════════════════════════════════════════════════════════════════ */

esp_err_t wake_word_init(wake_word_callback_t cb)
{
    esp_err_t ret;

    /* Lưu callback người dùng */
    s_user_cb = cb;

    /* 1. Tạo EventGroup điều khiển vòng lặp */
    s_event_group = xEventGroupCreate();
    if (s_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create EventGroup");
        return ESP_ERR_NO_MEM;
    }

    /* 2. Khởi tạo I2S Microphone */
    ret = s_i2s_mic_init();
    if (ret != ESP_OK) {
        return ret;
    }

    /* 3. Tải danh sách model từ partition "model" trên Flash SPIFFS
     *    Tìm model WakeNet (tên bắt đầu với ESP_WN_PREFIX)                 */
    char *wakenet_model_name = NULL;
    srmodel_list_t *models = esp_srmodel_init("model");
    if (models == NULL) {
        ESP_LOGE(TAG, "No speech recognition models found in 'model' partition.");
        ESP_LOGE(TAG, "Ensure partition table has 'model' entry and you have flashed models.");
        return ESP_ERR_NOT_FOUND;
    }

    for (int i = 0; i < models->num; i++) {
        ESP_LOGI(TAG, "Found model [%d]: %s", i, models->model_name[i]);
        if (strstr(models->model_name[i], ESP_WN_PREFIX) != NULL) {
            wakenet_model_name = models->model_name[i];
            ESP_LOGI(TAG, "Using WakeNet model: %s", wakenet_model_name);
        }
    }

    if (wakenet_model_name == NULL) {
        ESP_LOGE(TAG, "No WakeNet model found! Check sdkconfig: CONFIG_USE_WAKENET=y");
        return ESP_ERR_NOT_FOUND;
    }

    /* 4. Cấu hình AFE (Audio Front-End):
     *    - 1 kênh Mic (Mono), không có kênh reference (không AEC)
     *    - Noise Suppression SSP bật
     *    - VAD bật để phát hiện giọng nói
     *    - WakeNet bật với model đã tìm thấy
     *    - Chạy trên Core 1 (AFE tự quản lý core của nó)                   */
    afe_config_t afe_cfg = {
        .aec_init                        = false,  /* Không AEC (không có loa reference) */
        .se_init                         = true,   /* Speech Enhancement (lọc nhiễu)     */
        .vad_init                        = true,   /* Voice Activity Detection            */
        .wakenet_init                    = true,   /* Bật Wake Word engine                */
        .voice_communication_init        = false,  /* Không dùng VC mode (chỉ detection)  */
        .voice_communication_agc_init    = false,
        .voice_communication_agc_gain    = 10,
        .vad_mode                        = VAD_MODE_3,
        .wakenet_model_name              = wakenet_model_name,
        .wakenet_model_name_2            = NULL,
        .wakenet_mode                    = DET_MODE_90,         /* Ngưỡng 90% (cân bằng FP/FN) */
        .afe_mode                        = SR_MODE_HIGH_PERF,   /* Hiệu năng cao               */
        .afe_perferred_core              = 1,                   /* AFE chạy Core 1             */
        .afe_perferred_priority          = 5,
        .afe_ringbuf_size                = 50,
        .memory_alloc_mode               = AFE_MEMORY_ALLOC_MORE_INTERNAL, /* Ưu tiên SRAM nội */
        .afe_linear_gain                 = 1.0,
        .agc_mode                        = AFE_MN_PEAK_AGC_MODE_2,
        .pcm_config = {
            .total_ch_num = 1,    /* 1 kênh mic */
            .mic_num      = 1,    /* 1 mic      */
            .ref_num      = 0,    /* 0 reference */
            .sample_rate  = MIC_SAMPLE_RATE,
        },
        .debug_init                      = false,
        .debug_hook                      = {{AFE_DEBUG_HOOK_MASE_TASK_IN, NULL},
                                            {AFE_DEBUG_HOOK_FETCH_TASK_IN, NULL}},
        .afe_ns_mode                     = NS_MODE_SSP,
        .afe_ns_model_name               = NULL,
        .fixed_first_channel             = true,
    };

    s_afe_data = esp_afe_sr_v1.create_from_config(&afe_cfg);
    if (s_afe_data == NULL) {
        ESP_LOGE(TAG, "Failed to create AFE instance");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "AFE initialized successfully");

    /* 5. Tạo Task feed (Core 0): đọc Mic → feed AFE */
    BaseType_t task_ret = xTaskCreatePinnedToCore(
        audio_feed_task,
        "audio_feed",
        4096 * 3,           /* Stack: 12KB - đủ cho buffer PCM + I2S         */
        NULL,
        5,                  /* Priority cao hơn detect để đảm bảo data flow  */
        &s_feed_task_handle,
        0                   /* Core 0                                         */
    );
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create audio_feed task");
        return ESP_ERR_NO_MEM;
    }

    /* 6. Tạo Task detect (Core 1): lấy kết quả AFE → kiểm tra WakeWord */
    task_ret = xTaskCreatePinnedToCore(
        audio_detect_task,
        "audio_detect",
        4096 * 2,           /* Stack: 8KB - chỉ xử lý logic kiểm tra        */
        NULL,
        1,                  /* Priority thấp hơn feed                         */
        &s_detect_task_handle,
        1                   /* Core 1 (cùng core với AFE để giảm latency)     */
    );
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create audio_detect task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Wake word detection initialized successfully");
    return ESP_OK;
}


void wake_word_start(void)
{
    if (s_event_group != NULL) {
        /* Set bit DETECTION_RUNNING → cả 2 Task đều thoát khỏi trạng thái chờ */
        xEventGroupSetBits(s_event_group, DETECTION_RUNNING_BIT);
        ESP_LOGI(TAG, "Wake word detection STARTED - listening for wake word...");
    }
}


void wake_word_stop(void)
{
    if (s_event_group != NULL) {
        /* Clear bit → cả 2 Task đều trở lại trạng thái chờ (block) */
        xEventGroupClearBits(s_event_group, DETECTION_RUNNING_BIT);
        ESP_LOGI(TAG, "Wake word detection STOPPED");
    }

    /* Vô hiệu hóa I2S channel để giảm tiêu thụ điện tối đa
     * trước khi hệ thống vào deep sleep                                      */
    if (s_rx_handle != NULL) {
        i2s_channel_disable(s_rx_handle);
    }
}
