# TÀI LIỆU GHI CHÚ QUÁ TRÌNH PHÁT TRIỂN & GIẢI THÍCH CODE (FOXRO v0.1.0)

> **Mục tiêu dự án**: Xây dựng một dự án ESP32-S3 (dùng ESP-IDF 5.4.4) hoàn toàn mới, tối giản và sạch. Thiết bị liên tục lắng nghe từ khóa đánh thức ("Nihao Xiaozhi") qua micro I2S INMP441. Khi phát hiện từ khóa, bật đèn LED RGB WS2812B tích hợp trên mạch (GPIO 48) màu Xanh Dương trong vòng 3 giây, sau đó chuyển chip vào chế độ Ngủ sâu (Deep Sleep) để tiết kiệm năng lượng.

---

## 🔍 PHẦN 1: THAM KHẢO CODE TỪ DỰ ÁN GỐC (XIAOZHI/FOXROV)

Để viết lại toàn bộ mã nguồn mà không gây lỗi và đảm bảo đúng luồng xử lý AI, mình đã tham khảo và trích xuất các đoạn logic cốt lõi từ 2 phiên bản cũ trong thư mục `F:\idfiot\AI\`:

### 1. Tham khảo cấu hình I2S cho Micro INMP441
* **Nguồn**: `foxrov0.2.0/main/AudioDevice.cc` (Hàm `CreateDuplexChannels` / `CreateSimplexChannels`).
* **Logic lấy ra**: Cách thiết lập `i2s_std_config_t` cho chuẩn I2S.
* **Áp dụng vào code mới**:
  - Trong dự án gốc, I2S thu âm được đọc dưới dạng số nguyên 32-bit (`I2S_DATA_BIT_WIDTH_32BIT`), mặc dù INMP441 chỉ xuất 24-bit.
  - Mình áp dụng logic dịch bit `pcm_buf[i] = (int16_t)(i2s_buf[i] >> 16);` vào file `wake_word.c` của chúng ta để chuyển đổi I2S 32-bit thô thành chuẩn PCM 16-bit mà bộ lọc nhiễu AFE yêu cầu.

### 2. Tham khảo luồng xử lý Wake Word (esp-sr)
* **Nguồn**: `foxrov0.4.1/main/WakeWordDetect.cc` (Các hàm `Initialize`, `Feed`, `AudioDetectionTask`).
* **Logic lấy ra**:
  - Việc nhận diện cần chia làm 2 luồng (Task): 1 luồng đẩy dữ liệu vào mạng nơ-ron (Feed) và 1 luồng lấy kết quả ra (Fetch/Detect).
  - Khởi tạo cấu hình `afe_config_t` (Audio Front-End) với chế độ VAD (Voice Activity Detection), Wakeword (WakeNet), và SSP (Noise Suppression).
* **Áp dụng vào code mới**:
  - Thay vì dùng C++ Class phức tạp, mình đã viết lại thành C thuần trong `wake_word.c`.
  - Giữ nguyên kiến trúc chia 2 Task:
    - `audio_feed_task` (Core 0): Liên tục đọc I2S và gọi `esp_afe_sr_v1.feed()`.
    - `audio_detect_task` (Core 1): Liên tục gọi `esp_afe_sr_v1.fetch()` và kích hoạt Callback khi `wakeup_state == WAKENET_DETECTED`.

---

## 💻 PHẦN 2: CHI TIẾT QUÁ TRÌNH VIẾT CODE & GIẢI THÍCH

Dưới đây là trình tự các bước mình đã thực hiện để xây dựng dự án `foxro` từ con số 0.

### Bước 1: Khởi tạo và cấu hình dự án (Project Config)
* **File đã tạo**: `CMakeLists.txt`, `sdkconfig.defaults`, `partitions.csv`, `main/idf_component.yml`.
* **Giải thích**:
  - Khai báo dependency tối thiểu trong `idf_component.yml`: `espressif/esp-sr` (để dùng AI nhận diện giọng nói) và `espressif/led_strip` (để điều khiển LED WS2812B).
  - Cấu hình phân vùng `partitions.csv`: Bắt buộc tạo một phân vùng tên là `model` dung lượng 1MB định dạng `spiffs` để chứa bộ trọng số AI (WakeNet model).
  - Tinh chỉnh `sdkconfig.defaults`: Tối ưu hóa CPU lên 240MHz, bật PSRAM (Octal 80MHz) vì thuật toán nhận diện giọng nói tốn rất nhiều RAM (không đủ nếu chỉ dùng RAM nội bộ của chip).

### Bước 2: Viết Module điều khiển LED WS2812B (`rgb_led.c`)
* **Bối cảnh**: Bạn cung cấp chân LED là GPIO 48. Trên ESP32-S3-DevKitC-1, GPIO 48 được đấu vào một con LED WS2812B (LED địa chỉ hóa thông minh). Loại LED này không thể bật/tắt bằng lệnh mức cao/thấp (1/0) thông thường mà phải truyền dữ liệu theo giao thức 1-wire.
* **Giải pháp**:
  - Sử dụng ngoại vi **RMT** (Remote Control) của chip thông qua driver `espressif/led_strip`.
  - Cấu hình độ sáng mặc định (BRIGHTNESS) xuống mức 20/255 (khoảng 8%) để ánh sáng dễ chịu, không gây chói mắt.
  - Viết các hàm tiện ích: `rgb_led_set_blue()`, `rgb_led_set_white()`, v.v.

### Bước 3: Viết Module nhận diện từ khóa (`wake_word.c`)
* **Giải thích kiến trúc**:
  - **Khởi tạo I2S (`s_i2s_mic_init`)**: Chân SCK=GPIO5, WS=GPIO4, DIN=GPIO6 (như bạn cung cấp). Kênh âm thanh được set là Left-channel Mono.
  - **Khởi tạo AFE**: AFE (Audio Front End) đóng vai trò như màng lọc tạp âm trước khi đưa vào mô hình AI. Thiết lập hệ thống chỉ có 1 mic đầu vào, không có echo cancellation (do không dùng loa lúc lắng nghe).
  - **Task `audio_feed`**: Chạy trên nhân CPU 0 (Core 0), mức ưu tiên 5 (cao). Nhiệm vụ là "bơm" (feed) dữ liệu âm thanh từ Mic vào màng lọc AFE nhanh nhất có thể.
  - **Task `audio_detect`**: Chạy trên nhân CPU 1 (Core 1), mức ưu tiên 1 (thấp). Nhiệm vụ là chờ kết quả từ màng lọc. Nếu màng lọc báo `WAKENET_DETECTED`, nó sẽ kích hoạt hàm Callback.

### Bước 4: Lắp ghép Logic vòng lặp chính (`main.c`)
Đây là "trái tim" của hệ thống, điều phối chu trình sống của thiết bị theo mô hình State Machine:

1. **Boot**: Khởi tạo NVS Flash (bộ nhớ trong). Nháy đèn LED Trắng 1 lần báo hiệu có điện.
2. **Setup**: Gọi `wake_word_init()`. Nếu thành công, nháy đèn Xanh Lá 2 lần báo hiệu bắt đầu lắng nghe.
3. **Chờ đợi (Wait)**: Lệnh `xEventGroupWaitBits()` khóa (block) vòng lặp `while(1)` lại, giúp tiết kiệm CPU. Nó chỉ mở khóa khi module `wake_word` phát hiện có người gọi "Nihao Xiaozhi".
4. **Phản hồi (Action)**: Khi phát hiện từ khóa, bật đèn LED sang màu Xanh Dương bằng lệnh `rgb_led_set_blue()`. Giữ nguyên trong 3 giây (`vTaskDelay(3000)`).
5. **Vào Deep Sleep**:
   - Tắt đèn LED.
   - Gọi `wake_word_stop()` để ngắt ngoại vi I2S (cắt điện mic).
   - Thiết lập thời gian ngủ sâu 30 giây bằng `esp_sleep_enable_timer_wakeup()`.
   - Gọi `esp_deep_sleep_start()`: Lệnh này cắt điện toàn bộ chip (chỉ chừa lại bộ đếm thời gian). Sau 30 giây, chip sẽ khởi động lại từ đầu như vừa được cấp nguồn.

### Bước 5: Khắc phục lỗi Build của ESP-IDF trên Windows
* **Vấn đề**: Khi bạn chạy `idf.py build`, hệ thống báo lỗi `UnicodeDecodeError: 'charmap' codec can't decode byte 0x81`.
* **Nguyên nhân**: Script `movemodel.py` (nhiệm vụ copy model AI vào thư mục build) của ESP-IDF cố đọc file `sdkconfig` (file mà mình đã viết tiếng Việt UTF-8 vào) nhưng lại sử dụng bảng mã mặc định của Windows (ANSI/cp1252). Do không hiểu tiếng Việt, Python ném ra lỗi crash.
* **Cách mình đã xử lý**: Dùng công cụ code can thiệp sâu vào mã nguồn của ESP-IDF tại `F:\web\research\smart.motorcycle.platform\foxro\managed_components\espressif__esp-sr\model\movemodel.py`. Mình đã sửa tất cả các dòng lệnh `open()` thành `open(..., encoding="utf-8")`. Điều này giúp hệ thống build chạy mượt mà ngay cả khi dự án có ký tự tiếng Việt.

### Bước 6: Khắc phục lỗi Build lần 2 — Thiếu header khai báo kiểu & macro
* **Lỗi 1**: `unknown type name 'esp_err_t'` tại `wake_word.h:42`.
  - **Nguyên nhân**: File `wake_word.h` sử dụng kiểu trả về `esp_err_t` trong khai báo hàm `wake_word_init()`, nhưng kiểu này được định nghĩa trong header `esp_err.h` của ESP-IDF mà mình quên include.
  - **Cách sửa**: Thêm `#include "esp_err.h"` vào phần đầu file `wake_word.h`.

* **Lỗi 2**: `implicit declaration of function 'ESP_RETURN_ON_ERROR'` tại `wake_word.c:88`.
  - **Nguyên nhân**: Macro `ESP_RETURN_ON_ERROR` là một tiện ích kiểm tra lỗi do ESP-IDF cung cấp trong file `esp_check.h`. Macro này thực hiện: gọi hàm → nếu trả về lỗi thì in log và `return` ngay lập tức. Mình dùng nó để kiểm tra kết quả `i2s_new_channel()` và `i2s_channel_init_std_mode()` nhưng quên include header chứa macro đó.
  - **Cách sửa**: Thêm `#include "esp_check.h"` vào phần đầu file `wake_word.c`.

* **Bài học rút ra**: Trong ESP-IDF, mỗi kiểu dữ liệu và macro đều nằm trong một header riêng biệt. Khác với Arduino (nơi `Arduino.h` include gần hết mọi thứ), ESP-IDF yêu cầu khai báo rõ ràng từng dependency:
  - `esp_err.h` → kiểu `esp_err_t`, hằng `ESP_OK`, `ESP_FAIL`
  - `esp_check.h` → macro `ESP_RETURN_ON_ERROR`, `ESP_GOTO_ON_ERROR`
  - `esp_log.h` → macro `ESP_LOGI`, `ESP_LOGE`, `ESP_LOGW`

### Bước 7: Khắc phục lỗi Flash lần 3 — Overlap partition (model AI quá lớn)
* **Lỗi**: `Detected overlap at address: 0x200000 for file: foxro.bin`.
* **Nguyên nhân gốc rễ**: Khi build, hệ thống esp-sr tự động đóng gói tất cả model AI (WakeNet + VAD + NSNet) thành file `srmodels.bin`. File này có dung lượng thực tế **~3.9MB** (build log báo `Recommended model partition size: 3902K`), trong khi mình chỉ cấp 1MB cho partition `model` trong `partitions.csv`. Kết quả là `srmodels.bin` (1MB–4.9MB) tràn sang vùng nhớ của `factory` (bắt đầu tại 2MB), gây xung đột.

* **Cách sửa**: Mở rộng partition `model` từ **1MB → 4MB** và dịch toàn bộ partition phía sau ra:
  ```
  # Trước (lỗi):                      # Sau (đã sửa):
  model,  spiffs, 0x100000, 1M         model,  spiffs, 0x100000, 4M
  factory, app,   0x200000, 2M         factory, app,   0x500000, 2M
  ota_0,   app,   0x400000, 2M         ota_0,   app,   0x700000, 2M
  ota_1,   app,   0x600000, 2M         ota_1,   app,   0x900000, 2M
  ```
  Tổng dung lượng sử dụng: 0x900000 + 2MB = 0xB00000 = **11MB** < Flash 16MB → vẫn đủ.

* **Bài học rút ra**: Luôn kiểm tra dòng log `Recommended model partition size` khi build lần đầu với `esp-sr`. Mỗi model AI chiếm từ vài trăm KB đến hơn 1MB, và khi bật cả WakeNet + VAD + NSNet cộng lại có thể vượt 3MB. Nếu dùng Flash 4MB (thay vì 16MB), cần tắt bớt model trong `sdkconfig` hoặc dùng bảng phân vùng nhỏ hơn `partitions_4M.csv`.

---

## 🎯 TỔNG KẾT
Toàn bộ mã nguồn đã được tối ưu hóa cho ESP-IDF 5.4.x (từ bỏ các hàm I2S cũ đã bị deprecate trong phiên bản 0.2.x của dự án gốc). Kiến trúc được thiết kế dạng Module rời rạc, rất sạch sẽ để bạn có thể mở rộng tiếp (như thêm Bluetooth, 4G, hoặc Màn hình) trong tương lai.

