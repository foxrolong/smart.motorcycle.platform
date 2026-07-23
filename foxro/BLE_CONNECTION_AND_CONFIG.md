# BLE Connection & Runtime Configuration after Wake Word

## 1️⃣ Overview
After the wake‑word **"Hi Lily"** is detected the device no longer goes directly to deep‑sleep. Instead it:
1. **Initialises BLE (NimBLE)** and advertises a custom configuration service.
2. The user connects with a smartphone (e.g., nRF Connect).
3. Through GATT the phone can set:
   - **LED GPIO** (default `GPIO48`).
   - **Microphone I²S pins** (`WS`, `BCK`, `DIN`).
   - **Wake‑word phrase** (choose between the built‑in models).
4. Once the configuration is received the device stores the values in NVS, applies them, and proceeds to the normal **LED‑on‑wake → Deep‑Sleep** flow.

> This approach enables *field‑programmable* hardware mapping without reflashing firmware.

---
## 2️⃣ Required Components
| Component | ESP‑IDF module | Reason |
|-----------|----------------|--------|
| **NimBLE** | `esp-nimble` component (already part of IDF) | Lightweight BLE stack, supports custom GATT services. |
| **NVS** | `nvs_flash` | Persist configuration across resets. |
| **GPIO & I2S re‑initialisation** | Existing `rgb_led.c` / `wake_word.c` | Re‑use but with runtime‑chosen pins. |

---
## 3️⃣ File Structure
```
foxro/
├─ main/
│   ├─ ble.c / ble.h           # BLE init, advertising, GATT callbacks
│   ├─ config.c / config.h     # Helper to read/write config from NVS
│   ├─ main.c                  # Updated state machine to start BLE after wake
│   └─ …
```
All new files will be placed under `main/` so they are compiled as part of the `main` component.

---
## 4️⃣ GATT Service Design
- **Service UUID**: `0000F0A0-0000-1000-8000-00805F9B34FB`
- **Characteristics** (all **Write‑Only**, 4‑byte payload):
  1. **LED Pin** – UUID `0000F0A1-0000-1000-8000-00805F9B34FB`
  2. **Mic WS Pin** – UUID `0000F0A2-0000-1000-8000-00805F9B34FB`
  3. **Mic BCK Pin** – UUID `0000F0A3-0000-1000-8000-00805F9B34FB`
  4. **Mic DIN Pin** – UUID `0000F0A4-0000-1000-8000-00805F9B34FB`
  5. **Wake‑Word Model** – UUID `0000F0A5-0000-1000-8000-00805F9B34FB`
     - Payload: ASCII string matching a model folder (e.g., `wn9_hilili_tts`).
  6. **Commit** – UUID `0000F0A6-0000-1000-8000-00805F9B34FB`
     - Writing any value triggers the device to **save** the above values to NVS and **restart** the relevant subsystems.

---
## 5️⃣ Code Snippets
### 5.1 `ble.h`
```c
#pragma once
#include "esp_err.h"

void ble_init(void);
void ble_start_advertising(void);
void ble_stop(void);
```
### 5.2 `ble.c`
```c
#include "ble.h"
#include "config.h"
#include "esp_log.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

static const char *TAG = "ble";

/* UUID definitions (little‑endian) */
#define UUID_BASE   0x0000F0A0,0x0000,0x1000,0x8000,0x00805F9B34FB
static const ble_uuid128_t svc_uuid = BLE_UUID128_INIT(UUID_BASE);
static const ble_uuid128_t chr_led_uuid   = BLE_UUID128_INIT(0x0000F0A1,0x0000,0x1000,0x8000,0x00805F9B34FB);
static const ble_uuid128_t chr_ws_uuid    = BLE_UUID128_INIT(0x0000F0A2,0x0000,0x1000,0x8000,0x00805F9B34FB);
static const ble_uuid128_t chr_bck_uuid   = BLE_UUID128_INIT(0x0000F0A3,0x0000,0x1000,0x8000,0x00805F9B34FB);
static const ble_uuid128_t chr_din_uuid   = BLE_UUID128_INIT(0x0000F0A4,0x0000,0x1000,0x8000,0x00805F9B34FB);
static const ble_uuid128_t chr_model_uuid = BLE_UUID128_INIT(0x0000F0A5,0x0000,0x1000,0x8000,0x00805F9B34FB);
static const ble_uuid128_t chr_commit_uuid = BLE_UUID128_INIT(0x0000F0A6,0x0000,0x1000,0x8000,0x00805F9B34FB);

static int on_write(uint16_t conn_handle, uint16_t attr_handle,
                    struct ble_gatt_access_ctxt *ctxt, const void *arg)
{
    const struct os_mbuf *om = ctxt->om;
    uint8_t buf[32];
    int rc = OS_MBUF_READ(om, buf, OS_MBUF_PKTLEN(om));
    if (rc < 0) {
        ESP_LOGE(TAG, "Failed to read write payload");
        return BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    buf[rc] = 0; // NUL‑terminate for string use

    if (ble_uuid_cmp(ctxt->chr->uuid, &chr_led_uuid.u) == 0) {
        config_set_led_pin(*(uint8_t*)buf);
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &chr_ws_uuid.u) == 0) {
        config_set_mic_ws_pin(*(uint8_t*)buf);
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &chr_bck_uuid.u) == 0) {
        config_set_mic_bck_pin(*(uint8_t*)buf);
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &chr_din_uuid.u) == 0) {
        config_set_mic_din_pin(*(uint8_t*)buf);
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &chr_model_uuid.u) == 0) {
        config_set_wake_model((char*)buf);
    } else if (ble_uuid_cmp(ctxt->chr->uuid, &chr_commit_uuid.u) == 0) {
        config_commit();   // write all cached values to NVS
        ESP_LOGI(TAG, "Configuration committed – ready for restart");
        config_set_ready_flag(true);
    }
    return 0;
}

static const struct ble_gatt_chr_def gatt_chrs[] = {
    { .uuid = &chr_led_uuid.u,   .access_cb = on_write, .flags = BLE_GATT_CHR_F_WRITE },
    { .uuid = &chr_ws_uuid.u,    .access_cb = on_write, .flags = BLE_GATT_CHR_F_WRITE },
    { .uuid = &chr_bck_uuid.u,   .access_cb = on_write, .flags = BLE_GATT_CHR_F_WRITE },
    { .uuid = &chr_din_uuid.u,   .access_cb = on_write, .flags = BLE_GATT_CHR_F_WRITE },
    { .uuid = &chr_model_uuid.u, .access_cb = on_write, .flags = BLE_GATT_CHR_F_WRITE },
    { .uuid = &chr_commit_uuid.u, .access_cb = on_write, .flags = BLE_GATT_CHR_F_WRITE },
    { 0 },
};

static const struct ble_gatt_svc_def gatt_svc[] = {
    { .type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &svc_uuid.u, .characteristics = gatt_chrs },
    { 0 },
};

static void ble_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE host task started");
    nimble_port_run(); // This function will return only when NimBLE is stopped
    nimble_port_freertos_deinit();
}

void ble_init(void)
{
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();
    ble_sig_init();
    ble_gatts_register_svcs(gatt_svc);

    ble_gap_set_device_name("Foxro");
    // security parameters can stay default for this demo
    xTaskCreatePinnedToCore(ble_host_task, "nimble_host", 4096, NULL, 5, NULL, 0);
}

void ble_start_advertising(void)
{
    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(&adv_params, NULL, NULL);
    ESP_LOGI(TAG, "BLE advertising started");
}

void ble_stop(void)
{
    ble_gap_adv_stop();
    nimble_port_stop();
    ESP_LOGI(TAG, "BLE stopped");
}
```
### 5.3 `config.h / config.c`
```c
#pragma once
#include <stdbool.h>

void config_set_led_pin(uint8_t pin);
void config_set_mic_ws_pin(uint8_t pin);
void config_set_mic_bck_pin(uint8_t pin);
void config_set_mic_din_pin(uint8_t pin);
void config_set_wake_model(const char *model_name);
void config_commit(void);            // write to NVS
void config_set_ready_flag(bool ready);
bool config_is_ready(void);
```
```c
#include "config.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "config";
static nvs_handle_t nvs;
static bool ready_flag = false;

static void ensure_nvs(void)
{
    if (nvs != 0) return;
    esp_err_t err = nvs_open("foxro_cfg", NVS_READWRITE, &nvs);
    if (err != ESP_OK) ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
}

void config_set_led_pin(uint8_t pin)   { ensure_nvs(); nvs_set_u8(nvs, "led_pin", pin); }
void config_set_mic_ws_pin(uint8_t pin){ ensure_nvs(); nvs_set_u8(nvs, "mic_ws",  pin); }
void config_set_mic_bck_pin(uint8_t pin){ ensure_nvs(); nvs_set_u8(nvs, "mic_bck", pin); }
void config_set_mic_din_pin(uint8_t pin){ ensure_nvs(); nvs_set_u8(nvs, "mic_din", pin); }

void config_set_wake_model(const char *model_name)
{
    ensure_nvs();
    nvs_set_str(nvs, "wake_model", model_name);
}

void config_commit(void)
{
    ensure_nvs();
    esp_err_t err = nvs_commit(nvs);
    if (err != ESP_OK) ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    else ESP_LOGI(TAG, "Configuration saved to NVS");
}

void config_set_ready_flag(bool ready) { ready_flag = ready; }
bool config_is_ready(void) { return ready_flag; }
```
### 5.4 `main.c` – State‑machine cập nhật
```c
#include "ble.h"
#include "config.h"
#include "rgb_led.h"
#include "wake_word.h"
#include "esp_log.h"

enum {
    STATE_IDLE,
    STATE_WOKEN,
    STATE_CONFIGURE,   // BLE configuration state
    STATE_RUN,
};
static int state = STATE_IDLE;

void app_main(void)
{
    nvs_flash_init();
    rgb_led_init();          // default pins unless NVS overrides
    wake_word_init();        // default pins unless NVS overrides

    while (1) {
        switch (state) {
            case STATE_IDLE:
                esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
                esp_light_sleep_start();
                break;

            case STATE_WOKEN:
                rgb_led_set_color(0,0,255,255);
                vTaskDelay(pdMS_TO_TICKS(3000));
                rgb_led_off();
                state = STATE_CONFIGURE;   // move to BLE config
                break;

            case STATE_CONFIGURE:
                ble_init();
                ble_start_advertising();
                ESP_LOGI(TAG, "Waiting for phone configuration …");
                while (!config_is_ready()) {
                    vTaskDelay(pdMS_TO_TICKS(500));
                }
                ble_stop();
                // Re‑initialise peripherals with new settings from NVS
                rgb_led_deinit();
                rgb_led_init();   // reads LED pin from NVS
                wake_word_deinit();
                wake_word_init(); // reads I2S pins & model from NVS
                state = STATE_RUN;
                break;

            case STATE_RUN:
                // Normal operation – could keep listening or go deep‑sleep
                esp_deep_sleep_start();
                break;
        }
    }
}
```
> **Note**: `rgb_led_init()` and `wake_word_init()` need to be extended to read pin numbers and model name from NVS (fallback to defaults if not present).

---
## 6️⃣ Kiểm tra bằng điện thoại
1. Flash firmware (`idf.py build && idf.py -p com4 flash`).
2. Mở một BLE scanner (nRF Connect). Tìm thiết bị **"Foxro"**.
3. Kết nối → khám phá service `F0A0`.
4. Ghi giá trị cho các characteristic (ví dụ LED Pin = 48, Mic WS = 4, …).
5. Ghi bất kỳ giá trị nào vào **Commit**. Board sẽ log *"Configuration committed – ready"* và khởi động lại peripheral.
6. Nói wake‑word mới → LED bật, sau đó thiết bị vào deep‑sleep.

---
## 7️⃣ Build & flash lệnh
```powershell
# Xóa build cũ (đảm bảo file mới được biên dịch)
Remove-Item -Recurse -Force build
idf.py build
idf.py -p com4 flash monitor
```
Bạn sẽ thấy thêm log BLE khi vào trạng thái `STATE_CONFIGURE`.

---
## 8️⃣ Các tệp được thêm vào dự án
- `main/ble.c` & `main/ble.h`
- `main/config.c` & `main/config.h`
- `main/main.c` (cập nhật state‑machine)
- (cần cập nhật) `rgb_led.c` và `wake_word.c` để đọc cấu hình NVS
- **Tài liệu**: `BLE_CONNECTION_AND_CONFIG.md` (đây chính là file bạn đang xem).

---
### Tiếp theo
Nếu bạn muốn tôi cung cấp **các hàm đọc NVS** cho `rgb_led.c` và `wake_word.c` hoặc bất kỳ chi tiết nào khác, hãy cho biết nhé.   
