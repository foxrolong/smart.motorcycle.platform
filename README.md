# BÓC TÁCH & PHÂN TÍCH TOÀN BỘ DỰ ÁN ESP32-S3 AI VOICE ASSISTANT (FOXROV / XIAOZHI)
## DÀNH CHO DỰ ÁN NỀN TẢNG XE MÁY THÔNG MINH (SMART MOTORCYCLE PLATFORM)

> **Mục đích**: Nhánh này chứa tài liệu và mã nguồn bóc tách 100% từ dự án **ESP32-S3 AI Voice Assistant** (các phiên bản `foxrov0.2.0` và `foxrov0.2.1`) phục vụ tham khảo, nghiên cứu và phát triển Nền tảng Trợ lý Giọng nói trên Xe máy Thông minh (**Smart Motorcycle Platform**).

---

## 📁 DANH MỤC TÀI LIỆU BÓC TÁCH (DOCUMENTATION INDEX)

1. 📄 [Tài liệu Bóc tách Phiên bản v0.2.0](file:///run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform/FOXROV_v0.2.0_BREAKDOWN.md)
   - Phân tích tổng quan kiến trúc Edge AI + Cloud AI Hybrid.
   - Luồng nén/giải mã Opus, giao tiếp I2S Micro INMP441 / Loa MAX98357.
   - Song song hóa 700ms tối ưu độ trễ với Octal PSRAM 80MHz & FreeRTOS.
   - Bóc tách chi tiết từng file `main.cc`, `Application.cc`, `AudioDevice.cc`, `SystemReset.cc`.

2. 📄 [Tài liệu Bóc tách Phiên bản v0.2.1 (Bản Nâng cấp)](file:///run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform/FOXROV_v0.2.1_BREAKDOWN.md)
   - Bổ sung Chế độ kiểm thử âm thanh ngoại tuyến (`kChatStateTesting`) thu/phát loa mic trực tiếp qua SPIRAM bằng nút bấm **GPIO 1**.
   - Cơ chế Reset linh hoạt: Khôi phục NVS Flash & Factory Reset phân vùng `otadata` qua nút **GPIO 2**.
   - Mở rộng Máy trạng thái `ChatState`, gửi JSON báo cáo trạng thái qua WebSocket Server.
   - Phân tích sơ đồ chuỗi Sequence Diagram và giao thức truyền tin WebSocket (Text JSON + Binary Opus Header).

3. 📄 [Phân tích Thư viện ESP-SR & WakeNet](file:///run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform/esp_sr.md)
   - Chi tiết cấu hình AFE (Audio Front-End), lọc nhiễu môi trường, khử tiếng vang (AEC) và nhận diện từ khóa đánh thức offline.

---

## 📐 BẢNG SO SÁNH NÂNG CẤP V0.2.0 ↔ V0.2.1

| Tính năng / Module | Phiên bản v0.2.0 | Phiên bản v0.2.1 (Nâng cấp) | Ứng dụng cho Xe máy Thông minh |
| :--- | :--- | :--- | :--- |
| **Offline Audio Test** | Chưa hỗ trợ | Nhấn giữ GPIO 1 để thu âm PCM vào SPIRAM và phát lại ra loa I2S | Giúp chẩn đoán lỗi Micro/Loa khi xe đang di chuyển hoặc ở vùng không có mạng |
| **System Reset** | Nhấn GPIO 1/2 xóa NVS đơn giản | Tách biệt: GPIO 1 (Reset NVS Wi-Fi), GPIO 2 (Factory Reset + Erase OTA Data) | Khôi phục thiết bị nhanh chóng khi gặp sự cố sóng yếu/treo OTA |
| **Máy trạng thái ChatState** | `kChatStateIdle`, `kChatStateConnecting`, `kChatStateListening`, `kChatStateSpeaking` | Bổ sung `kChatStateTesting` | Hiển thị màu LED RGB cảnh báo trạng thái rõ ràng cho người lái xe |
| **WebSocket Event Reporting** | Chỉ gửi audio packets | Tự động đóng gói JSON báo cáo `state` (`testing`, `listening`, `speaking`) về Server | Server theo dõi trạng thái thực tế của thiết bị trên xe theo thời gian thực |

---

## 🛠️ KIẾN TRÚC TỔNG QUAN HỆ THỐNG (SYSTEM ARCHITECTURE)

```mermaid
flowchart TD
    subgraph Hardware ["Thiết bị trên Xe máy (ESP32-S3 Board)"]
        MIC[Micro INMP441] -->|I2S Rx - GPIO 10,11,3| AFE_Feed[AudioFeedTask]
        BTN1[Nút GPIO 1] -->|Hold: Offline Mic Test / Boot: Reset NVS| RESET[SystemReset]
        BTN2[Nút GPIO 2] -->|Boot: Factory Reset| RESET
        SPK[Loa MAX98357] <--|I2S Tx - GPIO 4,5,6| PLAY[AudioPlayTask]
        LED[BuiltinLed RGB] <--|Cảnh báo trạng thái| APP[Application State]
    end

    subgraph Memory ["Octal PSRAM 80MHz"]
        PCM_BUF[Đệm PCM WakeWord 50 packets]
        OPUS_BUF[Đệm Opus WakeWord]
        TEST_PCM[Đệm Test Audio PSRAM]
    end

    subgraph CoreLogic ["FreeRTOS Tasks & Pipeline"]
        AFE_Feed -->|PCM Data| AFE_SR[AudioDetectionTask / WakeNet]
        AFE_Feed -->|PCM Data| AFE_VC[AudioCommunicationTask / SSP NS]
        
        AFE_SR -->|WakeWord Detected| PARALLEL{Song song hóa 700ms}
        PARALLEL -->|Task 1| ENC_TASK[wake_word_encode_task - Opus]
        PARALLEL -->|Task 2| WS_CONN[StartWebSocketClient]

        AFE_VC -->|PCM Audio| QUEUE_ENC[audio_encode_queue] --> ENC[AudioEncodeTask]
        WS_DATA[WebSocket Binary/JSON] --> QUEUE_DEC[audio_decode_queue] --> DEC[AudioDecodeTask]
    end

    subgraph CloudServer ["Hạ tầng Cloud AI Server"]
        ENC -->|Opus Packets| CLOUD((WebSocket Cloud AI))
        ENC_TASK -->|Encoded Opus| CLOUD
        CLOUD -->|Opus Packets / TTS JSON| WS_DATA
    end
```

---

## 🚀 HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY DỰ ÁN

```bash
# 1. Chuyển vào thư mục dự án
cd /run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform

# 2. Thiết lập target ESP32-S3 cho ESP-IDF v5.3+
idf.py set-target esp32s3

# 3. Cấu hình các tham số qua Menuconfig
idf.py menuconfig

# 4. Biên dịch dự án
idf.py build

# 5. Đóng gói file xiaozhi.img 4MB duy nhất
python3 pack.py
```
