# BÓC TÁCH TOÀN BỘ DỰ ÁN ESP32-S3 AI VOICE ASSISTANT (FOXROV v0.2.1 / XIAOZHI)
## ỨNG DỤNG CHO NỀN TẢNG XE MÁY THÔNG MINH (SMART MOTORCYCLE PLATFORM)

> **Tài liệu bóc tách chuyên sâu v0.2.1**: Phân tích toàn bộ kiến trúc, tư duy thiết kế, nâng cấp phần cứng/phần mềm, thư viện sử dụng và bóc tách 100% mã nguồn dự án nâng cấp tại `/run/media/long/48D4B274D4B263BA/idfiot/AI/foxrov0.2.1` để phục vụ làm lại dự án Nền tảng xe máy thông minh (Smart Motorcycle Platform) tại địa chỉ `/run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform`.

---

## 📌 TỔNG QUAN NÂNG CẤP PHIÊN BẢN v0.2.1

Phiên bản **v0.2.1** bổ sung các cải tiến quan trọng về kiểm thử phần cứng tại chỗ (offline testing) và cơ chế Reset phần cứng linh hoạt qua các nút bấm vật lý:

1. **Chế độ kiểm thử âm thanh ngoại tuyến (`kChatStateTesting`)**:
   - Cho phép người dùng nhấn giữ nút bấm trên **GPIO 1** để thu âm trực tiếp vào bộ nhớ SPIRAM mà không cần kết nối mạng/WebSocket Server.
   - Khi nhả nút **GPIO 1**, thiết bị tự động khởi tạo Task FreeRTOS `play_test_audio` để resample và phát lại đoạn âm thanh vừa thu ra loa I2S, giúp kỹ thuật viên kiểm tra phần cứng Micro INMP441 và Loa MAX98357 nhanh chóng.
2. **Cơ chế quản lý Nút bấm Phần cứng (`SystemReset`)**:
   - **GPIO 1 (Khởi động)**: Nhấn khi cấp nguồn để xóa cấu hình Wi-Fi trong NVS Flash (`ResetNvsFlash()`).
   - **GPIO 2 (Khởi động)**: Nhấn khi cấp nguồn để xóa NVS Flash và phân vùng `otadata` (`ResetToFactory()`), khôi phục firmware gốc và khởi động lại sau 3 giây.
3. **Mở rộng Trạng thái Trợ lý Giọng nói (`ChatState`)**:
   - Bổ sung trạng thái `kChatStateTesting` và cập nhật LED RGB hiển thị màu Đỏ khi đang test micro.
   - Tự động báo cáo trạng thái `state` dưới dạng JSON qua WebSocket Server khi có thay đổi.

---

## 📐 PHẦN 1: TỔNG QUAN & TƯ DUY KIẾN TRÚC (ARCHITECTURAL PHILOSOPHY)

Dự án là hệ thống phần mềm nhúng (Firmware) cho vi điều khiển **ESP32-S3**, đóng vai trò là một **Trợ lý giọng nói AI (AI Voice Assistant)** truyền dữ liệu âm thanh hai chiều thời gian thực (real-time voice streaming) kết nối với hệ thống Server Cloud AI (sử dụng mô hình ASR SenseVoice, LLM Qwen-72B và TTS).

```mermaid
flowchart TD
    subgraph Hardware ["Phần cứng Xe máy & Vi điều khiển"]
        MIC[Micro INMP441] -->|I2S Rx - GPIO 10,11,3| AFE_Feed[AudioFeedTask]
        BTN1[Nút bấm GPIO 1] -->|Hold: Mic Test / Boot: NVS Reset| RESET[SystemReset / CheckTestButton]
        BTN2[Nút bấm GPIO 2] -->|Boot: Factory Reset| RESET
        SPK[Loa MAX98357] <--|I2S Tx - GPIO 4,5,6| PLAY[AudioPlayTask]
        LED[BuiltinLed RGB] <--|Hiển thị trạng thái| APP
    end

    subgraph Memory ["Bộ nhớ Octal PSRAM 80MHz"]
        PCM_BUF[Đệm PCM WakeWord 50 packets]
        OPUS_BUF[Đệm Opus WakeWord]
        TEST_PCM[Đệm Test PCM Audio]
    end

    subgraph Tasks ["Các FreeRTOS Tasks & Event Group"]
        AFE_Feed -->|PCM Data| AFE_SR[AudioDetectionTask / WakeNet]
        AFE_Feed -->|PCM Data| AFE_VC[AudioCommunicationTask / SSP NS]
        
        AFE_SR -->|WakeWord Detected| PARALLEL{Song song hóa 700ms}
        PARALLEL -->|Luồng 1| ENC_TASK[wake_word_encode_task - Opus]
        PARALLEL -->|Luồng 2| WS_CONN[StartWebSocketClient]

        AFE_VC -->|PCM Audio| QUEUE_ENC[audio_encode_queue] --> ENC[AudioEncodeTask]
        WS_DATA[WebSocket Audio/JSON] --> QUEUE_DEC[audio_decode_queue] --> DEC[AudioDecodeTask]
    end

    subgraph Network ["Hạ tầng Mạng & Cloud AI"]
        ENC -->|Opus Packets| CLOUD((WebSocket Server Cloud AI))
        ENC_TASK -->|Encoded Opus| CLOUD
        CLOUD -->|Opus Packets / TTS JSON| WS_DATA
    end

    DEC -->|Decoded PCM| PLAY
```

### 💡 Tư duy thiết kế cốt lõi:

1. **Kiến trúc Lai (Edge AI + Cloud AI Hybrid)**:
   * **Xử lý tại biên (Edge AI)**: Nhận diện từ khóa đánh thức ("你好小智" - Nihao Xiaozhi) hoàn toàn offline trên ESP32-S3 bằng mô hình nơ-ron lượng hóa (`WakeNet` trong `esp-sr`). Giúp tiết kiệm băng thông và năng lượng khi đứng chờ.
   * **Xử lý trên mây (Cloud AI)**: Sau khi đánh thức, thiết bị mới mở kết nối WebSocket truyền giọng nói để AI hiểu ngôn ngữ tự nhiên và phản hồi.

2. **Song song hóa tối ưu độ trễ (Parallel Latency Optimization)**:
   * Ngay khi phát hiện từ khóa đánh thức, thiết bị kích hoạt 2 công việc chạy song song:
     - **Luồng 1**: Mã hóa 2 giây đệm âm thanh PCM trước đó sang Opus (`EncodeWakeWordData`).
     - **Luồng 2**: Mở kết nối WebSocket tới Server (`StartWebSocketClient`).
   * Giúp tiết kiệm khoảng **700ms độ trễ**, tạo cảm giác phản hồi tức thì.

3. **Quản lý bộ nhớ Octal PSRAM trên ESP32-S3**:
   * Chuẩn nén Opus và AFE tốn nhiều RAM. Hệ thống cấu hình **Octal PSRAM 80MHz**, buộc cấp phát các đệm âm thanh lớn vào PSRAM qua `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`, giải phóng Internal SRAM cho I2S DMA và Wi-Fi stack.

4. **Đa nhiệm bất đồng bộ với FreeRTOS & Event Group**:
   * Sử dụng `EventGroupHandle_t` kiểm soát các bit cờ: `DETECTION_RUNNING`, `COMMUNICATION_RUNNING`, `DETECT_PACKETS_ENCODED`.

---

## 🛠️ PHẦN 2: THƯ VIỆN & PHỤ THUỘC (DEPENDENCIES)

Khai báo trong `main/idf_component.yml`:

| Thư viện / Component | Nguồn / Phiên bản | Chức năng chi tiết |
| :--- | :--- | :--- |
| **`espressif/esp-sr`** | Espressif (`^1.9.0`) | Xử lý âm thanh AI: **WakeNet9** (nhận diện từ khóa), **AFE** (Audio Front-End) lọc nhiễu (SSP Noise Suppression), điều chỉnh âm lượng tự động (AGC). |
| **`78/esp-opus-encoder`** | 3rd-party (`^1.0.0`) | Thư viện mã hóa/giải mã chuẩn nén âm thanh **Opus** tốc độ cao. |
| **`espressif/cJSON`** | Espressif (`^1.7.15`) | Parse và khởi tạo chuỗi JSON gửi/nhận dữ liệu điều khiển qua WebSocket. |

---

## 🔌 PHẦN 3: SƠ ĐỒ ĐẤU NỐI PHẦN CỨNG & GPIO MAP

Dự án tương thích với ESP32-S3 DevKitC-1 và phần cứng xe máy thông minh:

### 1. MAX98357 (Mạch khuếch đại Loa I2S)
* **LRC (WS)** -> GPIO 4
* **BCLK** -> GPIO 5
* **DIN (DOUT)** -> GPIO 6
* **GAIN** -> GND (hoặc 3.3V nếu cần tăng âm lượng)
* **SD** -> 3.3V
* **VIN / GND** -> 5V / GND

### 2. INMP441 (Micro I2S)
* **WS** -> GPIO 10
* **SCK (BCLK)** -> GPIO 11
* **SD (DIN)** -> GPIO 3
* **L/R** -> GND (Chọn kênh Trái)
* **VDD / GND** -> 3.3V / GND

### 3. Nút bấm vật lý & Đèn LED (Nâng cấp v0.2.1)
* **GPIO 1 (Input Pull-up)**:
  - **Khi Khởi động**: Nhấn giữ để Reset Wi-Fi NVS Flash.
  - **Khi Đang chạy**: Nhấn giữ để ghi âm test ngoại tuyến (`kChatStateTesting`), thả ra để phát lại.
* **GPIO 2 (Input Pull-up)**:
  - **Khi Khởi động**: Nhấn giữ để Factory Reset (Xóa NVS Flash & `otadata` partition).
* **RGB LED**: Đèn LED hiển thị trạng thái hệ thống.

---

## ⚙️ PHẦN 4: MA TRẬN TRẠNG THÁI & HIỂN THỊ LED (CHAT STATE)

| Trạng thái (`ChatState`) | Mã Enum | Đèn LED RGB | Hành vi hệ thống |
| :--- | :--- | :--- | :--- |
| `kChatStateIdle` | 0 | Tắt (Off) | Chờ từ khóa đánh thức, chạy `AudioDetectionTask`. |
| `kChatStateConnecting` | 1 | Xanh dương (Blue On) | Phát hiện Wakeword, đang mở WebSocket Server. |
| `kChatStateListening` | 2 | Đỏ (Red On) | Đã kết nối, đang thu âm giọng nói người dùng gửi lên Server. |
| `kChatStateSpeaking` | 3 | Xanh lá (Green On) | Loa đang phát âm thanh phản hồi từ Cloud AI (TTS). |
| `kChatStateWakeWordDetected` | 4 | Xanh dương (Blue On) | Đã mã hóa xong gói âm thanh Wakeword và gửi lên Server. |
| `kChatStateTesting` *(v0.2.1)* | 5 | Đỏ (Red On) | Đang nhấn giữ GPIO 1 để thu âm test ngoại tuyến vào SPIRAM. |

---

## 💻 PHẦN 5: BÓC TÁCH CHI TIẾT 100% MÃ NGUỒN C++

### 1. `main/main.cc` - Khởi tạo & Vòng lặp giám sát
- **`app_main()`**:
  1. Khởi tạo `SystemReset system_reset` và gọi `CheckButtons()` kiểm tra nút bấm GPIO 1/2.
  2. Khởi tạo Event Loop mặc định `esp_event_loop_create_default()`.
  3. Khởi tạo NVS Flash (`nvs_flash_init()`). Nếu lỗi trang đầy/phiên bản mới -> `nvs_flash_erase()`.
  4. Đọc namespace `"wifi"`. Nếu chưa cấu hình -> Bật LED xanh dương nhấp nháy và phát Hotspot AP `Xiaozhi` (`WifiConfigurationAp::GetInstance().Start("Xiaozhi")`).
  5. Nếu đã có Wi-Fi -> Chạy ứng dụng chính `Application::GetInstance().Start()`.
  6. Vòng lặp `while(true)` mỗi 10 giây in dung lượng bộ nhớ Heap còn lại và bộ nhớ Internal SRAM tối thiểu.

```cpp
// main/main.cc
extern "C" void app_main(void) {
    SystemReset system_reset;
    system_reset.CheckButtons();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

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
    
    Application::GetInstance().Start();

    while (true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        int free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        ESP_LOGI(TAG, "Free heap size: %u minimal internal: %u", SystemInfo::GetFreeHeapSize(), free_sram);
    }
}
```

---

### 2. `main/SystemReset.h` & `SystemReset.cc` - Quản lý Reset & Nút bấm
Nâng cấp trong **v0.2.1** cho phép kiểm tra cấu hình GPIO 1 và GPIO 2 làm Input Pull-up:
- **`CheckButtons()`**:
  - GPIO 2 = Low: Gọi `ResetNvsFlash()` và `ResetToFactory()`. Phân vùng `otadata` bị xóa trắng, thiết bị khởi động lại sau 3 giây (`esp_restart()`).
  - GPIO 1 = Low (khi Boot): Gọi `ResetNvsFlash()` xóa thông tin Wi-Fi.

```cpp
// main/SystemReset.cc
SystemReset::SystemReset() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_1) | (1ULL << GPIO_NUM_2);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
}

void SystemReset::CheckButtons() {
    if (gpio_get_level(GPIO_NUM_2) == 0) {
        ESP_LOGI(TAG, "Button is pressed, reset to factory");
        ResetNvsFlash();
        ResetToFactory();
    }
    if (gpio_get_level(GPIO_NUM_1) == 0) {
        ESP_LOGI(TAG, "Button is pressed, reset NVS flash");
        ResetNvsFlash();
    }
}
```

---

### 3. `main/Application.h` & `Application.cc` - Trung tâm điều khiển chính

#### A. Khởi tạo & Cấu hình Task tĩnh (Static Task Allocation)
Các Task nén/giải nén Opus chiếm tới **32KB Stack** mỗi Task. Để tránh vỡ bộ nhớ, ứng dụng tự tạo bộ đệm tĩnh cho Stack:

```cpp
// main/Application.cc
const size_t opus_stack_size = 4096 * 8; // 32KB
audio_encode_task_stack_ = (StackType_t*)malloc(opus_stack_size);
xTaskCreateStatic([](void* arg) {
    Application* app = (Application*)arg;
    app->AudioEncodeTask();
}, "opus_encode", opus_stack_size, this, 1, audio_encode_task_stack_, &audio_encode_task_buffer_);

audio_decode_task_stack_ = (StackType_t*)malloc(opus_stack_size);
xTaskCreateStatic([](void* arg) {
    Application* app = (Application*)arg;
    app->AudioDecodeTask();
}, "opus_decode", opus_stack_size, this, 1, audio_decode_task_stack_, &audio_decode_task_buffer_);
```

#### B. Chế độ Kiểm thử Nút bấm GPIO 1 (`CheckTestButton` & `PlayTestAudio`)
Thu âm và phát lại ngoại tuyến khi nhấn giữ GPIO 1:

```cpp
// main/Application.cc
void Application::CheckTestButton() {
    if (gpio_get_level(GPIO_NUM_1) == 0) {
        if (chat_state_ == kChatStateIdle) {
            SetChatState(kChatStateTesting);
            test_resampler_.Configure(CONFIG_AUDIO_INPUT_SAMPLE_RATE, CONFIG_AUDIO_OUTPUT_SAMPLE_RATE);
        }
    } else {
        if (chat_state_ == kChatStateTesting) {
            SetChatState(kChatStateIdle);
            xTaskCreate([](void* arg) {
                Application* app = static_cast<Application*>(arg);
                app->PlayTestAudio();
                vTaskDelete(NULL);
            }, "play_test_audio", 4096, this, 1, NULL);
        }
    }
}

void Application::PlayTestAudio() {
    auto packet = new AudioPacket();
    packet->type = kAudioPacketTypeStart;
    audio_device_.QueueAudioPacket(packet);

    for (auto& pcm : test_pcm_) {
        packet = new AudioPacket();
        packet->type = kAudioPacketTypeData;
        packet->pcm.resize(test_resampler_.GetOutputSamples(pcm.iov_len / 2));
        test_resampler_.Process((int16_t*)pcm.iov_base, pcm.iov_len / 2, packet->pcm.data());
        audio_device_.QueueAudioPacket(packet);
        heap_caps_free(pcm.iov_base);
    }
    test_pcm_.clear();

    packet = new AudioPacket();
    packet->type = kAudioPacketTypeStop;
    audio_device_.QueueAudioPacket(packet);
}
```

#### C. Các FreeRTOS Tasks trong `Application`

1. **`AudioFeedTask`** (Priority 5, 8KB Stack):
   - Liên tục đọc dữ liệu PCM thô từ Micro I2S qua `audio_device_.Read(buffer, chunk_size)`.
   - Nếu cờ `DETECTION_RUNNING` bật -> Nạp PCM vào AFE Wakenet (`esp_afe_sr_v1.feed`).
   - Nếu cờ `COMMUNICATION_RUNNING` bật -> Nạp PCM vào AFE Lọc nhiễu giọng nói (`esp_afe_vc_v1.feed`).

2. **`AudioDetectionTask`** (Priority 5, 8KB Stack):
   - Đọc kết quả xử lý từ AFE (`esp_afe_sr_v1.fetch`).
   - Gọi `StoreWakeWordData` lưu đệm 50 gói âm thanh gần nhất (khoảng 2 giây) vào PSRAM.
   - Kiểm tra `CheckTestButton()`. Nếu đang ở `kChatStateTesting` -> Lưu dữ liệu PCM vào `test_pcm_`.
   - Khi phát hiện từ khóa `WAKENET_DETECTED`:
     - Tắt cờ `DETECTION_RUNNING`.
     - Đổi trạng thái sang `kChatStateConnecting`.
     - Kích hoạt song song: `EncodeWakeWordData()` & `StartWebSocketClient()`.
     - Đợi cờ `DETECT_PACKETS_ENCODED`, sau đó gửi các gói Wakeword Opus lên Server qua WebSocket và chuyển cờ `COMMUNICATION_RUNNING`.

3. **`AudioCommunicationTask`** (Priority 5, 8KB Stack):
   - Lấy dữ liệu PCM đã lọc nhiễu qua `esp_afe_vc_v1.fetch`.
   - Kiểm tra trạng thái WebSocket. Nếu mất kết nối -> Ngắt loa (`audio_device_.Break()`), chuyển về `kChatStateIdle`, bật lại `DETECTION_RUNNING`.
   - Nếu ở trạng thái `kChatStateListening` -> Đẩy PCM vào `audio_encode_queue_`.

4. **`AudioEncodeTask`** (Priority 1, 32KB Static Stack):
   - Nhận dữ liệu PCM từ `audio_encode_queue_`.
   - Mã hóa sang chuẩn Opus bằng `opus_encoder_.Encode()`.
   - Gửi dữ liệu nén nhị phân (Binary Opus) qua WebSocket Server.

5. **`AudioDecodeTask`** (Priority 1, 32KB Static Stack):
   - Nhận `AudioPacket` từ `audio_decode_queue_`.
   - Giải mã dữ liệu Opus thành PCM bằng `opus_decode()`.
   - Thực hiện Resampling nếu tần số mẫu của Server khác tần số xuất của phần cứng Loa (`24kHz` vs `CONFIG_AUDIO_OUTPUT_SAMPLE_RATE`).
   - Đẩy PCM vào hàng chờ phát của Loa (`audio_device_.QueueAudioPacket`).

---

### 4. `main/AudioDevice.h` & `AudioDevice.cc` - Quản lý Phần cứng Âm thanh I2S
Quản lý luồng I2S Duplex (hoặc Simplex):
- Khởi tạo kênh I2S Tx (Loa) và Rx (Micro) bằng driver `driver/i2s_std.h` của ESP-IDF 5.x.
- **`QueueAudioPacket(AudioPacket* packet)`**: Đẩy gói âm thanh nhận từ WebSocket hoặc từ bài test vào `audio_play_queue_`.
- **`AudioPlayTask`**: Lấy gói âm thanh từ hàng chờ, ghi dữ liệu PCM ra I2S Tx Channel (`i2s_channel_write`). Khi bắt đầu/kết thúc phát âm thanh, tự động gọi callback `OnStateChanged` để chuyển đổi trạng thái LED giữa `kChatStateSpeaking` và `kChatStateListening`.

---

## 🌐 PHẦN 6: GIAO THỨC WEBSOCKET & CẤU TRÚC GÓI DỮ LIỆU

Kết nối WebSocket sử dụng SSL (`wss://`) với hai loại gói tin:

### 1. Gói tin Điều khiển (Text / JSON Packets)
* **Client Hello** (Thiết bị gửi ngay khi kết nối thành công):
```json
{
  "type": "hello",
  "version": "1.0",
  "wakeup_model": "wn9_nihaoxiaozhi",
  "audio_params": {
    "format": "opus",
    "sample_rate": 16000,
    "channels": 1
  }
}
```
* **Client State Report** (Tự động gửi khi đổi `chat_state_`):
```json
{
  "type": "state",
  "state": "listening"
}
```
* **Server TTS Control** (Server gửi điều khiển phát âm thanh):
```json
{
  "type": "tts",
  "state": "start",
  "sample_rate": 24000
}
```

### 2. Gói tin Dữ liệu Âm thanh (Binary Packets)
* **Cấu trúc Header nén nhị phân** (`AudioDataHeader` - 16 bytes packed):
```cpp
struct AudioDataHeader {
    uint32_t version;       // Phiên bản giao thức
    uint32_t reserved;      // Bộ nhớ dự phòng
    uint32_t timestamp;     // Nhãn thời gian (Network Byte Order - ntohl)
    uint32_t payload_size;  // Dung lượng gói Opus phía sau
} __attribute__((packed));
```

---

## 🛠️ PHẦN 7: SCRIPT ĐÓNG GÓI & PHÁT HÀNH FIRMWARE

### 1. `pack.py` - Đóng gói Firmware đơn (Single Binary File)
Script Python tự động gộp tất cả các phân vùng (Bootloader, Partition Table, OTADATA, Application Firmware, Storage/Models) thành 1 file duy nhất `xiaozhi.img` (dung lượng 4MB) phục vụ nạp nhanh qua Web Flash Tool:

```python
# pack.py
# Gộp các phân vùng Flash theo địa chỉ offset:
# 0x00000 -> bootloader.bin
# 0x08000 -> partition-table.bin
# 0x0e000 -> ota_data_initial.bin
# 0x10000 -> xiaozhi.bin (App Firmware)
```

### 2. `publish.py` - Xuất bản OTA Firmware
Tự động trích xuất thông số phiên bản, tạo file nén và tính toán mã MD5/SHA256 phục vụ Server OTA kiểm tra phiên bản nâng cấp tự động.

---

## 🚀 PHẦN 8: HƯỚNG DẪN BIÊN DỊCH VÀ ĐÓNG GÓI CHO DỰ ÁN SMART MOTORCYCLE PLATFORM

### 1. Yêu cầu môi trường
* **ESP-IDF**: Version 5.3 trở lên.
* **Target Board**: ESP32-S3 với Flash >= 8MB, PSRAM Octal >= 8MB.

### 2. Các bước cấu hình & Biên dịch (`idf.py`)
```bash
# 1. Thiết lập target ESP32-S3
idf.py set-target esp32s3

# 2. Cấu hình các tham số dự án qua Menuconfig
idf.py menuconfig
# Config tại: "Xiaozhi Assistant"
# - Websocket URL: wss://api.tenclass.net/xiaozhi/v1/
# - Access Token: test-token
# - GPIO Pinout cho Loa/Micro/Nút bấm

# 3. Biên dịch dự án
idf.py build

# 4. Nạp firmware lên thiết bị
idf.py -p /dev/ttyUSB0 flash monitor
```

### 3. Đóng gói file Binary duy nhất
```bash
python3 pack.py
# Kết quả xuất ra file xiaozhi.img sẵn sàng nạp cho thiết bị trên xe máy.
```

---

## 📝 TỔNG KẾT BÓC TÁCH KIẾN TRÚC CHO XE MÁY THÔNG MINH

Mã nguồn phiên bản **v0.2.1** đã hoàn thiện toàn bộ các module cần thiết cho hệ thống Trợ lý Giọng nói trên Xe máy Thông minh:
1. **Khả năng làm việc offline**: Thu âm & phát test nhanh qua nút bấm GPIO 1 mà không cần mạng.
2. **Khôi phục sự cố**: Nút Factory Reset GPIO 2 bảo đảm thiết bị trên xe máy luôn có thể phục hồi về bản gốc khi mất sóng hoặc lỗi cấu hình NVS.
3. **Độ trễ thấp & Âm thanh chất lượng**: Sự kết hợp giữa AFE Lọc nhiễu, Opus Encoder 60ms, Octal PSRAM và kết nối WebSocket bất đồng bộ đảm bảo khả năng tương tác mượt mà trong môi trường di chuyển trên đường.
