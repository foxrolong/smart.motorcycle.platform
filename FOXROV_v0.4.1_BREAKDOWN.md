# BÓC TÁCH TOÀN BỘ DỰ ÁN ESP32-S3 AI VOICE ASSISTANT (FOXROV v0.4.1 / XIAOZHI)
## ỨNG DỤNG CHO NỀN TẢNG XE MÁY THÔNG MINH (SMART MOTORCYCLE PLATFORM)

> **Tài liệu bóc tách chuyên sâu v0.4.1**: Phân tích toàn bộ kiến trúc, tư duy thiết kế, nâng cấp phần cứng/phần mềm (Màn hình LVGL LCD, ESP32-S3-BOX Audio Codec ES8311/ES7210, Mô-đun 4G ML307, Nâng cấp OTA), thư viện sử dụng và bóc tách 100% mã nguồn dự án nâng cấp tại `/run/media/long/48D4B274D4B263BA/idfiot/AI/foxrov0.4.1` để phục vụ làm lại dự án Nền tảng xe máy thông minh (Smart Motorcycle Platform) tại địa chỉ `/run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform`.

---

## 📌 TỔNG QUAN NÂNG CẤP ĐỘT PHÁ Ở PHIÊN BẢN v0.4.1

Phiên bản **v0.4.1** đại diện cho bước chuyển mình từ một firmware demo sang một **hệ thống thương mại đa nền tảng (Enterprise-grade Embedded AI Platform)**. Các nâng cấp cốt lõi bao gồm:

1. **Kiến trúc Modul hóa Clean Architecture (Tách biệt trách nhiệm hoàn toàn)**:
   - Tách module nhận diện từ khóa thành `WakeWordDetect.cc`.
   - Tách module lọc nhiễu & thu âm thành `AudioProcessor.cc`.
   - Tách module quản lý phím bấm vật lý thành `Button.cc` (hỗ trợ Boot, Volume Up, Volume Down).
2. **Hỗ trợ Đa phần cứng Audio (Audio Hardware Abstraction Layer)**:
   - Bên cạnh driver I2S tiêu chuẩn (`AudioDevice` cho INMP441 + MAX98357), bổ sung `BoxAudioDevice` giao tiếp qua chuẩn I2C master & I2S TDM để điều khiển chip Codec chuyên dụng **ES8311 (DAC Loa)** và **ES7210 (ADC Micro)** trên board **ESP32-S3-BOX**.
3. **Màn hình hiển thị LCD & Đồ họa LVGL (`Display.cc`)**:
   - Sử dụng thư viện đồ họa **LVGL** kết hợp driver `esp_lcd` (I2C/SPI panel IO). Hiển thị trực quan trạng thái AI, cuộn văn bản câu trả lời thời gian thực, thông báo tin nhắn và độ mạnh sóng Wi-Fi / 4G.
4. **Kết nối mạng kép Dual-Network (Wi-Fi + Cat.1 4G ML307 Modem)**:
   - Hỗ trợ mô-đun 4G Cat.1 **ML307** giao tiếp AT Command qua UART (`Ml307AtModem`, `Ml307Http`), cho phép thiết bị xe máy thông minh hoạt động trực tiếp ngoài đường mà không phụ thuộc vào Wi-Fi nhà.
5. **Hệ thống Nâng cấp Phần mềm qua mạng OTA nâng cao (`FirmwareUpgrade.cc`)**:
   - Tự động kiểm tra phiên bản mới từ Server qua HTTPS POST JSON, tải file binary nâng cấp theo khối (chunked download), tính toán tốc độ tải (KB/s) và % tiến trình để hiển thị lên màn hình LCD trước khi ghi vào phân vùng `ota_x`.
6. **Mở rộng giao thức Binary Protocol v2**:
   - Nâng cấp `BinaryProtocol` lên version 2 hỗ trợ truyền tải dữ liệu đa ngôn ngữ (SenseVoice: Việt, Anh, Trung, Nhật, Hàn) và nhận phản hồi cấu hình TTS động từ Cloud AI Server.

---

## 📐 PHẦN 1: TỔNG QUAN & TƯ DUY KIẾN TRÚC (ARCHITECTURAL PHILOSOPHY)

Dự án là hệ thống phần mềm nhúng (Firmware) cho vi điều khiển **ESP32-S3**, đóng vai trò là một **Trợ lý giọng nói AI (AI Voice Assistant)** truyền dữ liệu âm thanh hai chiều thời gian thực (real-time voice streaming) kết nối với hệ thống Server Cloud AI.

```mermaid
flowchart TD
    subgraph Hardware ["Phần cứng Xe máy & ESP32-S3 Hardware Layer"]
        MIC_STD[Micro I2S INMP441] -->|I2S Rx| AUD_DEV[AudioDevice Standard]
        BOX_CODEC[Codec ES8311/ES7210] -->|I2C + I2S TDM| BOX_DEV[BoxAudioDevice]
        SPK_STD[Loa I2S MAX98357] <--|I2S Tx| AUD_DEV
        LCD[Màn hình ST7789 / SSD1306] <--|SPI / I2C| DISP[Display LVGL Task]
        BTNS[Boot / Vol+ / Vol-] -->|GPIO Interupt| BTN_MOD[Button Class]
        MODEM_4G[Mô-đun 4G Cat.1 ML307] <-->|UART AT Cmd| ML307_MOD[Ml307AtModem / Ml307Http]
    end

    subgraph AudioProcessing ["Xử lý âm thanh & Edge AI (ESP-SR)"]
        AUD_DEV -->|PCM 16kHz Mono| PROC[AudioProcessor Task]
        BOX_DEV -->|PCM 16kHz Mono| PROC
        PROC -->|Feed PCM| WAKE[WakeWordDetect Task / WakeNet9]
        WAKE -->|WakeWord Detected| APP_EVT{Song song hóa 700ms}
    end

    subgraph CoreApplication ["Hạt nhân ứng dụng (Application Singleton & FreeRTOS)"]
        APP_EVT -->|Encode Wakeword| OPUS_ENC[OpusEncoder Task]
        APP_EVT -->|Start Net Client| NET_CONN{Kết nối Mạng}
        BTN_MOD -->|Short / Long Press| APP_MAIN[Application Main Loop]
        APP_MAIN -->|Update UI Text/Icon| DISP
        APP_MAIN -->|Check & Flash Binary| OTA[FirmwareUpgrade Manager]
    end

    subgraph NetworkLayer ["Hạ tầng Truyền thông & Cloud AI"]
        NET_CONN -->|Wi-Fi Station| ESP_NET[EspHttp / WebSocket Client]
        NET_CONN -->|4G Cat.1 Cellular| ML307_MOD
        ESP_NET <-->|Opus Voice Stream / JSON| CLOUD((Server Cloud AI: NLU, Qwen-72B, TTS))
        ML307_MOD <-->|Opus Voice Stream / JSON| CLOUD
        OTA <--|Download Binary Firmware| CLOUD
    end

    CLOUD -->|Opus Packets| DECODER[OpusDecoder] -->|PCM Data| AUD_DEV
    CLOUD -->|Opus Packets| DECODER -->|PCM Data| BOX_DEV
```

### 💡 Các nguyên lý thiết kế hệ thống cấp cao trong v0.4.1:

1. **Abstraction Layer cho thiết bị ngoại vi (Hardware Abstraction Layer - HAL)**:
   - Lop `AudioDevice` kế thừa làm base class cho `BoxAudioDevice`. Hệ thống có thể chuyển đổi giữa board tự chế (INMP441 + MAX98357) và board thương mại (ESP32-S3-BOX) chỉ qua cờ biên dịch `CONFIG_AUDIO_CODEC_ES8311_ES7210` trong `sdkconfig`.
   - Lớp `Http` kế thừa làm base class cho `EspHttp` (Wi-Fi) và `Ml307Http` (4G LTE). Hệ thống OTA không cần quan tâm mạng đang dùng là Wi-Fi hay 4G.

2. **Giao diện người dùng hướng sự kiện với LVGL (Event-driven GUI)**:
   - Module `Display` chạy trên nền thư viện đồ họa **LVGL**. Khi ứng dụng đổi trạng thái (`ChatState`), `Display` lập tức cập nhật label, thanh trạng thái notification và hiệu ứng cuộn tự động.

3. **Xử lý âm thanh đa luồng cách ly (Isolated Audio Pipeline)**:
   - `WakeWordDetect` chạy trên một Task riêng biệt, chịu trách nhiệm quản lý đệm nhẫn PCM (Ring Buffer) 50 gói âm thanh trước khi bật wakeword, giúp giữ lại 2s âm thanh người dùng nói trước khi từ khóa được phát hiện.

---

## 🛠️ PHẦN 2: CÁC THƯ VIỆN & CÔNG CỤ NÂNG CẤP (THIRD-PARTY DEPENDENCIES)

Khai báo trong `main/idf_component.yml`:

| Thư viện / Component | Nguồn / Phiên bản | Chức năng chi tiết trong v0.4.1 |
| :--- | :--- | :--- |
| **`espressif/esp-sr`** | Espressif (`^1.9.0`) | Xử lý AI giọng nói: **WakeNet9** ("Nihao Xiaozhi"), lọc nhiễu **AFE** (Audio Front-End) SSP. |
| **`78/esp-opus-encoder`** | Component 3rd-party (`^1.0.0`) | Mã hóa và giải mã nén âm thanh **Opus** cực thấp độ trễ. |
| **`espressif/lvgl`** | Espressif (`^8.3.11`) | Thư viện thiết kế giao diện đồ họa chuyên nghiệp cho màn hình nhúng. |
| **`espressif/esp_lcd`** | ESP-IDF Built-in | Trình điều khiển phần cứng màn hình LCD (ST7789, ILI9341, SSD1306). |
| **`espressif/esp_codec_dev`** | Espressif (`^1.2.0`) | Driver điều khiển chip âm thanh phần cứng chuyên dụng ES8311 & ES7210. |
| **`78/esp-ml307`** | Component 3rd-party (`^1.0.0`) | Trình điều khiển Mô-đun 4G Cat.1 ML307 qua UART AT Commands. |
| **`espressif/cJSON`** | ESP-IDF Built-in | Đóng gói & phân tích cú pháp JSON giao thức giao tiếp Server. |

---

## 🔌 PHẦN 3: SƠ ĐỒ ĐẤU NỐI PHẦN CỨNG & GPIO MAP (NÂNG CẤP v0.4.1)

Dự án v0.4.1 hỗ trợ 2 cấu hình phần cứng linh hoạt:

### 1. Cấu hình Standard (Board ESP32-S3 DevKit + Micro & Loa rời + Màn hình ST7789)
* **Loa MAX98357 (I2S TX)**:
  - `BCLK` -> GPIO 5
  - `LRC (WS)` -> GPIO 4
  - `DIN` -> GPIO 6
* **Micro INMP441 (I2S RX)**:
  - `SCK (BCLK)` -> GPIO 11
  - `WS` -> GPIO 10
  - `SD` -> GPIO 3
* **Màn hình ST7789 SPI**:
  - `SDA (MOSI)` -> GPIO 7
  - `SCL (CLK)` -> GPIO 15
  - `CS` -> GPIO 6 / None
  - `DC` -> GPIO 4
  - `RST` -> GPIO 48
* **Phím bấm vật lý (Button)**:
  - `BOOT Button` -> GPIO 0
  - `VOLUME UP` -> GPIO 1
  - `VOLUME DOWN` -> GPIO 2

### 2. Cấu hình ESP32-S3-BOX (Chip Audio ES8311 & ES7210 TDM Codec)
* **I2C Master Control**: `SDA` -> GPIO 8, `SCL` -> GPIO 18 (Điều khiển thanh ghi ES8311 / ES7210).
* **I2S TDM Duplex**: `BCLK` -> GPIO 13, `WS` -> GPIO 14, `DOUT` -> GPIO 15, `DIN` -> GPIO 16.

---

## ⚙️ PHẦN 4: MA TRẬN TRẠNG THÁI & HIỂN THỊ MÀN HÌNH (CHAT STATE & DISPLAY)

| Trạng thái (`ChatState`) | Mã Enum | Đèn LED RGB | Giao diện Màn hình LCD (LVGL) | Hành vi hệ thống |
| :--- | :--- | :--- | :--- | :--- |
| `kChatStateIdle` | 0 | Off | Hiển thị Icon Chờ + Tên thiết bị | Chờ từ khóa đánh thức, chạy `WakeWordDetect`. |
| `kChatStateConnecting` | 1 | Xanh dương | Hiển thị "Đang kết nối Server..." | Mở kết nối WebSocket Server qua Wi-Fi / 4G. |
| `kChatStateListening` | 2 | Đỏ | Icon Micro + "Đang lắng nghe..." | Đang thu âm giọng nói người dùng truyền lên Server. |
| `kChatStateSpeaking` | 3 | Xanh lá | Cuộn văn bản câu trả lời của AI | Phát âm thanh TTS nhận từ Cloud AI ra Loa. |
| `kChatStateWakeWordDetected` | 4 | Xanh dương | Icon Sét + "Đã phát hiện từ khóa" | Đã đóng gói xong đệm PCM Wakeword và gửi lên Server. |
| `kChatStateUpgrading` | 5 | Vàng nháy | Thanh tiến trình OTA (% + KB/s) | Đang nạp Firmware mới từ OTA Server vào Flash. |

---

## 💻 PHẦN 5: BÓC TÁCH CHI TIẾT 100% MÃ NGUỒN C++ (12 FILE MÃ NGUỒN CHÍNH)

---

### 🟢 1. `main/main.cc` - Điểm khởi chạy hệ thống
```cpp
extern "C" void app_main(void)
{
    // 1. Kiểm tra nút bấm reset phần cứng NVS / Factory
    SystemReset system_reset;
    system_reset.CheckButtons();

    // 2. Khởi tạo Event Loop mặc định
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 3. Khởi tạo bộ nhớ NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#ifndef CONFIG_USE_ML307
    // 4. Nếu không dùng 4G ML307 -> Kiểm tra Wi-Fi NVS
    nvs_handle_t nvs_handle;
    ret = nvs_open("wifi", NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        auto& builtin_led = BuiltinLed::GetInstance();
        builtin_led.SetBlue();
        builtin_led.Blink(1000, 500);

        WifiConfigurationAp::GetInstance().Start("Xiaozhi");
        return;
    }
    nvs_close(nvs_handle);
#endif

    // 5. Khởi chạy ứng dụng chính Application Singleton
    Application::GetInstance().Start();

    // 6. Vòng lặp giám sát RAM định kỳ 10 giây
    while (true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        int free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGI("MAIN", "Free heap: %u, min internal SRAM: %u", SystemInfo::GetFreeHeapSize(), free_sram);
    }
}
```

---

### 🔵 2. `main/Application.h` & `Application.cc` - Trái tim ứng dụng (Core App)

#### `Application.h` - Khai báo lớp điều phối:
- Quản lý các Task FreeRTOS: `AudioEncodeTask`, `AudioPlayTask`, `CheckNewVersionTask`.
- Quản lý 3 hàng đợi (Queue): `audio_encode_queue_`, `audio_decode_queue_`, `audio_play_queue_`.
- Quản lý trạng thái AI `chat_state_` và các đối tượng phần cứng `audio_device_`, `display_`, `wake_word_detect_`, `audio_processor_`, `firmware_upgrade_`.

#### `Application.cc` - Thực thi logic điều phối:

1. **Khởi động ứng dụng (`Start`)**:
   ```cpp
   void Application::Start() {
       // Khởi tạo phím bấm Boot và Volume
       boot_button_.OnPress([this]() {
           Schedule([this]() {
               if (chat_state_ == kChatStateSpeaking) {
                   break_speaking_ = true; // Nhấn BOOT để ngắt AI đang nói
               } else if (chat_state_ == kChatStateIdle) {
                   StartWebSocketClient(); // Nhấn BOOT để gọi AI thủ công
               }
           });
       });

       // Khởi tạo Audio Device (Duplex I2S hoặc ES8311 Codec)
       audio_device_.Initialize();

       // Khởi tạo Display LCD LVGL
   #ifdef CONFIG_USE_DISPLAY
       display_.SetText("Đang khởi động...");
   #endif

       // Khởi tạo WakeWordDetect & AudioProcessor
   #ifdef CONFIG_USE_AFE_SR
       wake_word_detect_.Initialize(audio_device_.input_channels(), audio_device_.input_reference());
       wake_word_detect_.OnWakeWordDetected([this]() {
           Schedule([this]() {
               SetChatState(kChatStateConnecting);
               EncodeWakeWordData(); // Mã hóa 2s PCM trước đó
               StartWebSocketClient(); // Kết nối WebSocket song song
           });
       });
       wake_word_detect_.StartDetection();

       audio_processor_.Initialize(audio_device_);
       audio_processor_.Start();
   #endif

       // Tạo Task mã hóa và phát âm thanh
       xTaskCreatePinnedToCore([](void* arg) {
           static_cast<Application*>(arg)->AudioEncodeTask();
       }, "audio_encode", 4096, this, 5, &audio_encode_task_, 1);
   }
   ```

2. **Xử lý gói tin nhị phân BinaryProtocol v2 (`HandleAudioPacket`)**:
   ```cpp
   void Application::HandleAudioPacket(AudioPacket* packet) {
       switch (packet->type) {
           case kAudioPacketTypeSentenceStart:
               // Bắt đầu một câu trả lời mới từ AI -> Xóa hàng đợi cũ
               skip_to_end_ = false;
               break;
           case kAudioPacketTypeData:
               // Gói dữ liệu âm thanh Opus -> Đẩy vào hàng đợi giải mã
               if (!skip_to_end_ && !break_speaking_) {
                   audio_play_queue_.push_back(packet);
               } else {
                   delete packet;
               }
               break;
           case kAudioPacketTypeSentenceEnd:
               // Kết thúc câu
               break;
       }
   }
   ```

---

### 🟡 3. `main/BoxAudioDevice.h` & `BoxAudioDevice.cc` - Trình điều khiển Codec phần cứng ES8311/ES7210

Được kích hoạt khi cấu hình board **ESP32-S3-BOX**:

```cpp
void BoxAudioDevice::Initialize() {
    // 1. Khởi tạo Bus I2C Master để truyền lệnh cấu hình thanh ghi
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_8,
        .scl_io_num = GPIO_NUM_18,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    i2c_new_master_bus(&i2c_bus_cfg, &i2c_master_handle_);

    // 2. Cấu hình Chip ES8311 (DAC Output cho Loa)
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = CODEC_MODE_DECODE,
        .i2c_bus = i2c_master_handle_,
        .i2c_addr = ES8311_CODEC_DEFAULT_ADDR,
    };
    out_codec_if_ = es8311_codec_create(&es8311_cfg);

    // 3. Cấu hình Chip ES7210 (ADC Input cho 2-Micro Array)
    es7210_codec_cfg_t es7210_cfg = {
        .i2c_bus = i2c_master_handle_,
        .i2c_addr = ES7210_CODEC_DEFAULT_ADDR,
    };
    in_codec_if_ = es7210_codec_create(&es7210_cfg);

    // 4. Mở kênh I2S TDM Duplex 16kHz 16-bit
    CreateDuplexChannels();
}
```

---

### 🟣 4. `main/AudioProcessor.h` & `AudioProcessor.cc` - Tách biệt xử lý âm thanh đầu vào

Lớp `AudioProcessor` nhận luồng PCM thô từ `AudioDevice`, thực hiện cân bằng âm lượng (AGC), lọc nhiễu (NS) và chuyển dữ liệu cho `WakeWordDetect`:

```cpp
void AudioProcessor::AudioProcessorTask() {
    std::vector<int16_t> input_pcm(samples_per_frame_);
    while (running_) {
        // Đọc dữ liệu PCM từ Micro
        int samples_read = audio_device_.Read(input_pcm.data(), samples_per_frame_);
        if (samples_read > 0) {
            // Chuyển dữ liệu sang WakeWordDetect để quét từ khóa đánh thức
            wake_word_detect_.Feed(input_pcm);
            
            // Nếu đang trong phiên hội thoại -> Đẩy âm thanh vào Queue mã hóa Opus
            if (Application::GetInstance().IsListening()) {
                Application::GetInstance().PushAudioEncodeData(input_pcm);
            }
        }
    }
}
```

---

### 🔴 5. `main/Display.h` & `Display.cc` - Hiển thị giao diện đồ họa LVGL LCD

```cpp
Display::Display(int sda_pin, int scl_pin) {
    // 1. Khởi tạo bus I2C/SPI hiển thị
    // 2. Khởi tạo Driver Panel esp_lcd (ST7789 / SSD1306)
    // 3. Khởi tạo Thư viện LVGL (lv_init & lv_port_disp_init)
    
    // Tạo Label hiển thị văn bản AI
    label_ = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label_, LV_LABEL_LONG_SCROLL_CIRCULAR); // Cuộn tròn văn bản
    lv_obj_set_width(label_, 220);
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

---

### 🟠 6. `main/FirmwareUpgrade.h` & `FirmwareUpgrade.cc` - Nâng cấp Phần mềm OTA

```cpp
void FirmwareUpgrade::StartUpgrade(std::function<void(int progress, size_t speed)> callback) {
    upgrade_callback_ = callback;
    
    esp_http_client_config_t config = {
        .url = firmware_url_.c_str(),
        .cert_pem = NULL,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    // Bắt đầu quá trình nạp OTA vào phân vùng ota_0 / ota_1
    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    
    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        // Tính toán % tiến trình và tốc độ KB/s báo về Display
        int image_len = esp_https_ota_get_image_len_read(https_ota_handle);
        int total_len = esp_https_ota_get_image_size(https_ota_handle);
        int progress = (image_len * 100) / total_len;
        upgrade_callback_(progress, current_speed);
    }

    if (esp_https_ota_finish(https_ota_handle) == ESP_OK) {
        esp_restart(); // Nạp thành công -> Khởi động lại
    }
}
```

---

### 🟤 7. `main/Button.h` & `Button.cc` - Quản lý Phím bấm Chống rung (Debounce)

Trình quản lý nút bấm nâng cao hỗ trợ phát hiện Nhấn ngắn (Short Press), Nhấn giữ (Long Press) và Chống rung phần cứng (Debounce):

```cpp
void Button::CheckState() {
    int level = gpio_get_level(gpio_num_);
    uint32_t now = esp_log_timestamp();

    if (level == active_level_) {
        if (!is_pressed_) {
            is_pressed_ = true;
            press_time_ = now;
        } else if (now - press_time_ > long_press_time_ms_ && !long_press_triggered_) {
            long_press_triggered_ = true;
            if (long_press_callback_) long_press_callback_();
        }
    } else {
        if (is_pressed_) {
            is_pressed_ = false;
            if (!long_press_triggered_ && (now - press_time_ > debounce_ms_)) {
                if (press_callback_) press_callback_();
            }
            long_press_triggered_ = false;
        }
    }
}
```

---

## 📋 PHẦN 6: BẢNG SO SÁNH TIẾN HÓA TOÀN DIỆN (v0.2.0 vs v0.2.1 vs v0.4.1)

| Tiêu chí phân tích | Phiên bản v0.2.0 | Phiên bản v0.2.1 | Phiên bản v0.4.1 (Mới nhất) |
| :--- | :--- | :--- | :--- |
| **Kiến trúc Code** | Đơn khối (Monolithic) | Đơn khối + Kiểm thử | **Clean Architecture (Modul hóa 100%)** |
| **Hỗ trợ Phần cứng Audio** | I2S Standard (INMP441/MAX98357) | I2S Standard | **Dual HAL: Standard I2S & ES8311/ES7210 Codec** |
| **Giao diện Người dùng** | LED RGB đơn giản | LED RGB đơn giản | **Đồ họa LVGL LCD (ST7789) + LED RGB** |
| **Phương thức Kết nối Mạng**| Wi-Fi Station / AP | Wi-Fi Station / AP | **Dual Net: Wi-Fi Station/AP + Cat.1 4G LTE ML307** |
| **Cập nhật Firmware** | Nạp qua cáp USB | Nạp qua cáp USB | **Tự động Nâng cấp OTA qua HTTPS HTTP Chunked** |
| **Xử lý Nút bấm** | GPIO Polling trực tiếp | Interupt cơ bản | **Button Class (Short/Long Press + Debounce)** |
| **Tiến trình Âm thanh** | Gom chung trong Application | Gom chung trong Application | **Tách riêng AudioProcessor & WakeWordDetect Task** |

---

## 🚀 HƯỚNG DẪN ỨNG DỤNG CHO DỰ ÁN XE MÁY THÔNG MINH (SMART MOTORCYCLE PLATFORM)

Khi chuyển giao kiến trúc **v0.4.1** sang dự án Xe máy thông minh (`smart.motorcycle.platform`):

1. **Sử dụng kết nối 4G Cat.1 ML307**: Xe máy di chuyển ngoài đường không có Wi-Fi, việc tích hợp `Ml307AtModem` giúp thiết bị liên tục giữ kết nối Cloud AI qua SIM 4G.
2. **Hiển thị Màn hình LCD Đồng hồ xe (Cluster Display)**: Tận dụng module `Display.cc` với LVGL để đưa thông tin trợ lý AI, chỉ đường (Navigation text) và trạng thái xe trực tiếp lên màn hình xe máy.
3. **Phím bấm trên Ghi-đông (Handlebar Control)**: Sử dụng module `Button.cc` kết nối với các nút bấm vật lý trên ghi-đông (nhấn BOOT/Mic button để gọi trợ lý hoặc hủy phát câu thoại AI).
