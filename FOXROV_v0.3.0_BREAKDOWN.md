# BÓC TÁCH TOÀN BỘ DỰ ÁN ESP32-S3 AI VOICE ASSISTANT (FOXROV v0.3.0 / XIAOZHI)
## ỨNG DỤNG CHO NỀN TẢNG XE MÁY THÔNG MINH (SMART MOTORCYCLE PLATFORM)

> **Tài liệu bóc tách chuyên sâu v0.3.0**: Phân tích toàn bộ kiến trúc, tư duy thiết kế, nâng cấp phần cứng/phần mềm (Màn hình LCD Display, Nâng cấp OTA Firmware, SystemInfo), thư viện sử dụng và bóc tách 100% mã nguồn dự án tại `/run/media/long/48D4B274D4B263BA/idfiot/AI/foxrov0.3.0` để phục vụ làm lại dự án Nền tảng xe máy thông minh (Smart Motorcycle Platform) tại địa chỉ `/run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform`.

---

## 📌 TỔNG QUAN NÂNG CẤP THẾ HỆ v0.3.0

Thế hệ **v0.3.0** đánh dấu cột mốc quan trọng khi dự án bắt đầu tích hợp **màn hình hiển thị giao diện người dùng (Display GUI)** và **tự động kiểm tra/nâng cấp phần mềm qua mạng (OTA Firmware Upgrade)**. Các điểm cải tiến nổi bật:

1. **Module Màn hình LCD (`Display.cc` & `Display.h`)**:
   - Tích hợp driver hiển thị LCD `esp_lcd` và giao diện đồ họa **LVGL**.
   - Hiển thị văn bản phản hồi của AI, tin nhắn thông báo, trạng thái kết nối Wi-Fi/Server.
2. **Hệ thống Quản lý & Nâng cấp OTA Firmware (`FirmwareUpgrade.cc` & `FirmwareUpgrade.h`)**:
   - Đọc phiên bản từ Server, so sánh Semantic Versioning (X.Y.Z) để xác định có bản nâng cấp mới hay không.
   - Nạp firmware mới vào phân vùng OTA và tự động reboot.
3. **Module Thông tin Hệ thống (`SystemInfo.cc` & `SystemInfo.h`)**:
   - Thu thập thông tin phần cứng: Tên Chip, Số nhân CPU, Dung lượng Flash, Dung lượng SRAM/PSRAM còn trống, địa chỉ MAC Wi-Fi, và phiên bản Firmware hiện tại.
4. **Cấu hình Menuconfig mở rộng (`main/Kconfig.projbuild`)**:
   - Bổ sung menu cấu hình hiển thị, chân kết nối SPI/I2C cho màn hình, URL nâng cấp OTA và Access Token xác thực.

---

## 📐 PHẦN 1: TỔNG QUAN & TƯ DUY KIẾN TRÚC (ARCHITECTURAL PHILOSOPHY)

Dự án là hệ thống phần mềm nhúng (Firmware) cho vi điều khiển **ESP32-S3**, đóng vai trò là một **Trợ lý giọng nói AI (AI Voice Assistant)** truyền dữ liệu âm thanh hai chiều thời gian thực kết nối với hệ thống Server Cloud AI.

```mermaid
flowchart TD
    subgraph Hardware ["Phần cứng & Ngoại vi"]
        MIC[Micro INMP441] -->|I2S Rx| AUD_DEV[AudioDevice]
        SPK[Loa MAX98357] <--|I2S Tx| AUD_DEV
        LCD[Màn hình LCD ST7789] <--|SPI / I2C| DISP[Display Module LVGL]
        RESET_BTN[GPIO 1 / GPIO 2] --> RESET[SystemReset]
    end

    subgraph CoreApp ["Hạt nhân Application v0.3.0"]
        AUD_DEV -->|PCM Data| AFE[WakeNet9 / AFE]
        AFE -->|WakeWord Detected| PARALLEL{Song song hóa 700ms}
        PARALLEL -->|Encode Opus| OPUS[OpusEncoder Task]
        PARALLEL -->|Connect WS| WS[StartWebSocketClient]
        
        SYS_INFO[SystemInfo] -->|Báo cáo RAM/CPU| APP[Application Loop]
        APP -->|Cập nhật Text/Icon| DISP
        APP -->|Kích hoạt Check Version| OTA[FirmwareUpgrade]
    end

    subgraph Network ["Kết nối Mạng Cloud"]
        WS <-->|WebSocket Binary Opus / Text JSON| CLOUD((Cloud AI Server))
        OTA <--|Check Version & Download Binary| CLOUD
    end
```

---

## 🛠️ PHẦN 2: THƯ VIỆN & CÔNG CỤ ĐƯỢC SỬ DỤNG

Khởi tạo trong `main/idf_component.yml`:

| Thư viện / Component | Nguồn / Phiên bản | Chức năng chi tiết trong v0.3.0 |
| :--- | :--- | :--- |
| **`espressif/esp-sr`** | Espressif (`^1.9.0`) | **WakeNet9** nhận diện từ khóa đánh thức offline "你好小智" và lọc nhiễu AFE. |
| **`78/esp-opus-encoder`** | 3rd-party (`^1.0.0`) | Mã hóa và giải mã chuẩn nén âm thanh **Opus**. |
| **`espressif/lvgl`** | Espressif (`^8.3.11`) | Thiết kế và quản lý các Widget đồ họa màn hình (Text, Label, Notification). |
| **`espressif/esp_lcd`** | ESP-IDF Built-in | Driver giao tiếp phần cứng màn hình LCD (ST7789/SSD1306). |
| **`cJSON`** | ESP-IDF Built-in | Parse JSON phản hồi từ Server và tạo gói tin cấu hình. |

---

## 🔌 PHẦN 3: SƠ ĐỒ ĐẤU NỐI PHẦN CỨNG (GPIO MAP v0.3.0)

* **Loa MAX98357 (I2S TX)**: `BCLK` -> GPIO 5, `LRC (WS)` -> GPIO 4, `DIN` -> GPIO 6
* **Micro INMP441 (I2S RX)**: `SCK (BCLK)` -> GPIO 11, `WS` -> GPIO 10, `SD` -> GPIO 3
* **Màn hình LCD ST7789 (SPI)**: `SDA` -> GPIO 7, `SCL` -> GPIO 15, `DC` -> GPIO 4, `RST` -> GPIO 48
* **Nút bấm Reset Phần cứng**: `GPIO 1` (Reset Wi-Fi), `GPIO 2` (Factory Reset)

---

## ⚙️ PHẦN 4: MA TRẬN TRẠNG THÁI & HIỂN THỊ MÀN HÌNH

| Trạng thái (`ChatState`) | Đèn LED RGB | Giao diện Màn hình LCD | Trạng thái hệ thống |
| :--- | :--- | :--- | :--- |
| `kChatStateIdle` | Off | Icon Chờ + Tên thiết bị | Chờ từ khóa đánh thức WakeNet. |
| `kChatStateConnecting` | Xanh dương | "Đang kết nối Server..." | Mở kênh WebSocket Client. |
| `kChatStateListening` | Đỏ | Icon Micro + "Đang lắng nghe..." | Thu âm giọng nói truyền lên Server. |
| `kChatStateSpeaking` | Xanh lá | Cuộn văn bản câu trả lời | Phát âm thanh TTS nhận từ Cloud ra Loa. |
| `kChatStateUpgrading` | Vàng nháy | "Đang nâng cấp OTA..." | Đang tải và ghi Firmware mới vào Flash. |

---

## 💻 PHẦN 5: BÓC TÁCH CHI TIẾT MÃ NGUỒN C++ v0.3.0

### 1. `main/Display.h` & `Display.cc` - Quản lý giao diện LVGL
```cpp
// main/Display.cc
Display::Display(int sda_pin, int scl_pin) {
    // Khởi tạo bus I2C/SPI hiển thị và driver esp_lcd
    // Kích hoạt timer cập nhật LVGL (esp_timer)
    label_ = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(label_, 200);
    lv_obj_align(label_, LV_ALIGN_CENTER, 0, 0);
}

void Display::SetText(const std::string &text) {
    text_ = text;
    lv_async_call([](void* arg) {
        Display* d = static_cast<Display*>(arg);
        lv_label_set_text(d->label_, d->text_.c_str());
    }, this);
}
```

### 2. `main/FirmwareUpgrade.h` & `FirmwareUpgrade.cc` - Tải và Nâng cấp OTA
```cpp
// main/FirmwareUpgrade.cc
bool FirmwareUpgrade::IsNewVersionAvailable(const std::string& currentVersion, const std::string& newVersion) {
    auto current = ParseVersion(currentVersion);
    auto next = ParseVersion(newVersion);
    return next > current; // So sánh từng thành phần Major.Minor.Patch
}

void FirmwareUpgrade::Upgrade(const std::string& firmware_url) {
    esp_http_client_config_t config = {
        .url = firmware_url.c_str(),
        .timeout_ms = 10000,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    if (esp_https_ota(&ota_config) == ESP_OK) {
        esp_restart();
    }
}
```

### 3. `main/SystemInfo.h` & `SystemInfo.cc` - Thu thập thông tin phần cứng
```cpp
// main/SystemInfo.cc
std::string SystemInfo::GetMacAddress() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(mac_str);
}

int SystemInfo::GetFreeHeapSize() {
    return esp_get_free_heap_size();
}
```
