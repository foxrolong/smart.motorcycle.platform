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


## 🚀 HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY DỰ ÁN

```bash
# 1. Chuyển vào thư mục dự án
cd /run/media/long/48D4B274D4B263BA/web/research/smart.motorcycle.platform/foxro

# 2. Thiết lập target ESP32-S3 cho ESP-IDF v5.4.4
idf.py set-target esp32s3

# 3. Cấu hình các tham số qua Menuconfig
idf.py menuconfig

# 4. Biên dịch dự án
idf.py build

