# AI Reverse Engineering Prompt - ESP32-S3 Voice Assistant (ESP-IDF v6.0.2)
## Phiên bản 2 - Đã bổ sung các phần còn thiếu

---

## Role

Bạn là một Software Architect cấp cao, chuyên về Embedded Systems, ESP32, ESP-IDF, ESP-SR, FreeRTOS và Audio Processing.

Nhiệm vụ của bạn **không phải copy dự án**, mà là **bóc tách hoàn toàn kiến trúc của dự án gốc**, hiểu cách hoạt động, sau đó viết lại toàn bộ dự án theo chuẩn mới.

Mục tiêu là giúp tôi hiểu được toàn bộ dự án từ gốc đến ngọn.

---

## Mục tiêu cuối cùng

Tạo một dự án mới sử dụng:

* ESP32-S3
* ESP-IDF **v6.0.2**
* ESP-SR (phiên bản tương thích IDF v6.0.2 — xem Giai đoạn 0.4)
* CMake chuẩn của ESP-IDF
* Code sạch (Clean Architecture)
* Dễ mở rộng
* Không phụ thuộc cấu trúc dự án cũ

Không được giữ nguyên cách tổ chức code của dự án gốc nếu có thể thiết kế tốt hơn.

**Ghi chú kỹ thuật quan trọng:** Từ ESP-IDF v6.0 trở đi, driver I2S kiểu cũ (legacy, `driver/i2s.h`) đã bị loại bỏ hoàn toàn khỏi framework. Nếu dự án gốc dùng legacy I2S driver, bắt buộc phải port sang I2S driver mới (`driver/i2s_std.h`, `i2s_channel_t`, `i2s_new_channel`...). Đây là một breaking change bắt buộc, không phải tùy chọn.

---

## Giai đoạn 0 - Xác nhận Input (BẮT BUỘC, làm trước tiên)

Trước khi phân tích bất kỳ điều gì, AI phải thực hiện các bước xác nhận sau và **dừng lại chờ xác nhận từ người dùng** nếu có bất kỳ mục nào chưa rõ. Không được tự giả định hoặc tự bịa để tiếp tục.

### 0.1 Xác nhận danh sách file nhận được

* Liệt kê đầy đủ cây thư mục (tree) của dự án gốc đã nhận.
* Đối chiếu: file nào được include trong `CMakeLists.txt` / `idf_component.yml` nhưng không có source thực tế đi kèm → phải nêu rõ và hỏi lại, không được tự suy diễn nội dung file đó.
* Nếu dự án gốc là một repo GitHub, ghi rõ: tên repo, branch/tag, commit hash tại thời điểm phân tích.

### 0.2 Xác nhận phần cứng cụ thể

Bắt buộc phải xác định rõ (từ dự án gốc hoặc hỏi người dùng nếu không rõ):

* **Board**: tên chính xác (ví dụ: ESP32-S3-DevKitC-1, ESP32-S3-Korvo-2, board tự thiết kế...).
* **Microphone**: loại cảm biến (I2S chuẩn dạng analog như INMP441/SPH0645, hay PDM mic), số kênh (mono/stereo/multi-mic array), sample rate, bit depth.
* **Loa / DAC output**: chip khuếch đại (MAX98357A, NS4168, PCM5102...) hay dùng DAC nội bộ ESP32, cách kết nối I2S (chân GPIO cụ thể: BCLK, WS/LRCLK, DIN, DOUT).
* **RGB LED**: LED đơn dùng GPIO thường (3 chân R/G/B, PWM qua LEDC), hay LED địa chỉ hóa (WS2812/NeoPixel qua RMT hoặc SPI). Số lượng LED, chân GPIO điều khiển.

Nếu bất kỳ mục nào không xác định được từ dự án gốc, AI phải hỏi người dùng trước khi sang Giai đoạn 1.

### 0.3 Xác nhận nguồn âm thanh phản hồi ("Đã bật")

* Dự án gốc phát câu "Đã bật" (hoặc tương đương) bằng cách nào:
  * File âm thanh ghi sẵn (WAV/MP3/PCM thô) nhúng vào flash, SPIFFS, LittleFS, hay embed trực tiếp vào binary qua `EMBED_FILES`?
  * Hay dùng TTS engine chạy trên thiết bị?
* Nếu dùng file ghi sẵn: định dạng file, sample rate, bit depth, số kênh — phải khớp với cấu hình I2S output, nếu không khớp phải ghi rõ cần resample hoặc chuyển đổi.
* Nếu dùng TTS: xác nhận TTS engine đó có hỗ trợ tiếng Việt và có tương thích IDF v6.0.2 hay không.

### 0.4 Xác nhận phiên bản ESP-SR và WakeNet model

* Ghi rõ version ESP-SR component sẽ dùng (từ `idf_component.yml` của dự án gốc, hoặc submodule commit).
* Xác nhận version ESP-SR đó có chính thức hỗ trợ build trên ESP-IDF v6.0.2 hay không (ESP-SR gắn chặt theo từng release IDF, không phải mọi bản đều tương thích ngược/xuôi).
* Xác định tên Wake Word model cụ thể đang dùng (ví dụ "Hi ESP", "Xiaozhi", hay model custom train riêng) và ngôn ngữ của model đó.
* Nếu ESP-SR bản dự án gốc dùng không tương thích IDF v6.0.2, phải ghi rõ trong `docs/05_IDF6_Migration.md` cần nâng cấp lên ESP-SR version nào.

### 0.5 Xác nhận license dự án gốc

* Ghi rõ loại license của dự án gốc (MIT, Apache-2.0, GPL, không rõ license...).
* Nếu dự án dự định public/chia sẻ sau này, việc "tham khảo logic" phải tuân thủ điều khoản license đó — ghi chú rõ trong `docs/04_Logic_Extraction.md`.

**Chỉ khi tất cả các mục 0.1 → 0.5 đã được xác nhận đầy đủ (hoặc người dùng xác nhận bỏ qua mục nào có lý do), mới được chuyển sang Giai đoạn 1.**

---

## Giai đoạn 1 - Phân tích dự án gốc

Tạo file:

```text
docs/01_Project_Analysis.md
```

File này phải mô tả đầy đủ, bao gồm nhưng không giới hạn:

* Cấu trúc thư mục
* Các Component
* CMakeLists
* sdkconfig
* partitions
* main
* include
* Driver
* Audio
* Network
* FreeRTOS
* Event Loop
* State Machine
* Queue
* Ring Buffer
* Memory Allocation
* Task
* ISR
* Callback
* Timer
* GPIO
* I2S
* SPI
* UART
* WiFi
* ESP-SR

Không được bỏ sót bất kỳ module nào.

---

## Giai đoạn 2 - Reverse Engineering

Tạo:

```text
docs/02_Reverse_Engineering.md
```

Đối với từng file trong dự án gốc, phải ghi rõ:

### File

Ví dụ

```text
audio_recorder.c
```

Sau đó phân tích:

* Chức năng
* Luồng hoạt động
* Input
* Output
* Hàm nào gọi
* Hàm nào được gọi
* Dữ liệu truyền đi đâu
* Queue nào sử dụng
* Mutex nào sử dụng
* Semaphore nào sử dụng
* Có cấp phát heap không
* Có dùng DMA không
* Có dùng Ring Buffer không

Sau đó mới sang file tiếp theo. Không được bỏ sót file nào.

---

## Giai đoạn 3 - Truy vết toàn bộ logic

Tạo:

```text
docs/03_Code_Flow.md
```

Mô tả toàn bộ luồng chạy, ví dụ:

```
Boot
  ↓
app_main()
  ↓
init()
  ↓
wifi_init()
  ↓
audio_init()
  ↓
afe_init()
  ↓
wakenet_init()
  ↓
Task Create
  ↓
Event Loop
  ↓
Idle
  ↓
Wake
  ↓
Record
  ↓
Play
  ↓
Idle
```

Mỗi bước phải giải thích. Không được chỉ vẽ sơ đồ.

---

## Giai đoạn 4 - Trích xuất logic

Không copy nguyên file. Xác định:

* Logic nào đáng giữ
* Logic nào lỗi thời
* Logic nào có thể cải tiến
* Logic nào phụ thuộc ESP-IDF cũ
* Logic nào cần viết lại

Tạo:

```text
docs/04_Logic_Extraction.md
```

Trong mỗi phần ghi rõ:

**Lấy từ dự án gốc** — giải thích vì sao giữ.

**Viết mới** — giải thích vì sao không dùng code cũ.

### Định nghĩa ranh giới "giữ logic" (quan trọng, làm rõ mâu thuẫn giữa Giai đoạn 4 và Giai đoạn 6)

* "Giữ logic" nghĩa là giữ **ý tưởng/thuật toán/kiến trúc xử lý** — ví dụ: thứ tự cấu hình AFE, cách chọn tham số WakeNet, luồng chuyển trạng thái Idle → Wake → Record → Play → Idle, cách đồng bộ giữa task thu âm và task phát âm.
* **Không giữ** bất kỳ ký tự code nào: không copy tên biến, tên hàm, comment, format, cấu trúc file, thứ tự khai báo.
* Ghi rõ trong file: mỗi logic được "giữ" phải trích dẫn rõ tên file/hàm nguồn tương ứng ở dự án gốc và mô tả bằng lời (không phải bằng code) ý tưởng đó là gì.
* Nếu dự án gốc có license ràng buộc (xem mục 0.5), phần "giữ logic" phải tuân thủ điều khoản license đó.

---

## Giai đoạn 5 - Port sang ESP-IDF v6.0.2

Toàn bộ code phải được viết lại theo API của ESP-IDF v6.0.2. Không được sử dụng API đã deprecated.

Nếu API thay đổi, ghi rõ:

* API cũ
* API mới
* Lý do thay đổi
* Ảnh hưởng

**Các thay đổi bắt buộc phải rà soát cho dự án này (dựa trên breaking changes đã biết của IDF v6.0):**

* I2S: driver legacy (`driver/i2s.h`) → I2S driver mới (`driver/i2s_std.h`).
* ADC/DAC/Timer Group/PCNT/MCPWM/RMT/Temperature Sensor: nếu dự án gốc dùng driver legacy của các module này, phải xác nhận có dùng không và port sang driver mới tương ứng.
* Thư viện C chuẩn: IDF v6.0 chuyển mặc định từ Newlib sang Picolibc — kiểm tra nếu dự án gốc có phụ thuộc hành vi đặc thù của Newlib.
* Cảnh báo compiler được coi là lỗi (warnings-as-errors) mặc định trong v6.0 — code mới phải build sạch, không warning.

Tạo:

```text
docs/05_IDF6_Migration.md
```

---

## Giai đoạn 6 - Viết lại Clean Code

Sau khi hiểu logic, viết lại 100%, không copy. Yêu cầu:

* Clean Architecture
* SOLID
* Modular
* Component-based
* Low Coupling
* High Cohesion

Không copy tên biến, comment, format, cấu trúc cũ nếu không cần thiết.

### Cấu trúc thư mục bắt buộc cho dự án mới

```text
project-root/
├── CMakeLists.txt
├── partitions.csv
├── sdkconfig.defaults
├── main/
│   ├── CMakeLists.txt
│   └── app_main.c
├── components/
│   ├── audio_input/          # I2S mic + AFE + WakeNet
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   └── audio_input.c
│   ├── audio_output/         # I2S speaker playback
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   └── audio_output.c
│   ├── led_indicator/        # RGB LED control
│   │   ├── CMakeLists.txt
│   │   ├── include/
│   │   └── led_indicator.c
│   └── app_state/            # State machine: Idle/Wake/Record/Play
│       ├── CMakeLists.txt
│       ├── include/
│       └── app_state.c
└── docs/
    ├── 01_Project_Analysis.md
    ├── 02_Reverse_Engineering.md
    ├── 03_Code_Flow.md
    ├── 04_Logic_Extraction.md
    ├── 05_IDF6_Migration.md
    ├── 06_Comparison.md
    └── Development_Log.md
```

Mỗi component phải có ranh giới trách nhiệm rõ ràng, giao tiếp với `app_main`/`app_state` qua queue hoặc event, không gọi chéo trực tiếp giữa `audio_input` và `audio_output`.

---

## Giai đoạn 7 - So sánh

Tạo:

```text
docs/06_Comparison.md
```

Mỗi module phải có bảng:

| Module | Dự án gốc | Dự án mới |
|---|---|---|

Giải thích: cái gì giữ, cái gì bỏ, cái gì viết mới, lý do.

---

## Giai đoạn 8 - Nhật ký phát triển

Tạo:

```text
docs/Development_Log.md
```

Sau mỗi thay đổi đều ghi: Ngày, Module, File, Làm gì, Lý do, Ảnh hưởng, Các file liên quan.

Không được bỏ bất kỳ thay đổi nào.

---

## Giai đoạn 9 - Dự án đầu tiên (phạm vi triển khai)

Chỉ xây dựng các chức năng sau:

**Audio Input**
* I2S Microphone (theo phần cứng đã xác nhận ở Giai đoạn 0.2)
* ESP-SR AFE
* WakeNet (model đã xác nhận ở Giai đoạn 0.4)

**Audio Output**
* I2S Speaker

**RGB**

Khi Wake Word thành công:

1. RGB sáng.
2. Loa phát: "Đã bật." (nguồn âm thanh theo Giai đoạn 0.3)
3. Sau đó quay về trạng thái lắng nghe (Idle).

Không thêm AI. Không thêm LLM. Không thêm MQTT. Không thêm WebSocket. Không thêm OTA.

Chỉ hoàn thiện pipeline:

```
Microphone → AFE → WakeNet → RGB → Speaker → Idle
```

### Tiêu chí nghiệm thu (Definition of Done)

Dự án chỉ được coi là hoàn thành khi đáp ứng **tất cả** các điều kiện sau:

* [ ] Build bằng `idf.py build` thành công, **0 lỗi, 0 warning**.
* [ ] Flash lên board thực tế chạy được (`idf.py -p <PORT> flash monitor`).
* [ ] Nói đúng wake word → RGB sáng đúng thời điểm.
* [ ] Loa phát rõ "Đã bật" không bị rè/ngắt/lệch tốc độ.
* [ ] Sau khi phát xong, hệ thống tự quay lại trạng thái lắng nghe (Idle), có thể wake lại nhiều lần liên tục không bị treo.
* [ ] Không có memory leak: theo dõi `heap_caps_get_free_size(MALLOC_CAP_8BIT)` trước/sau nhiều chu kỳ wake-record-play, giá trị heap phải ổn định (không giảm dần theo thời gian).
* [ ] Không có crash/panic/watchdog reset trong tối thiểu 30 phút chạy liên tục với nhiều lần wake thử nghiệm.
* [ ] Tất cả các file `docs/*.md` được cập nhật đầy đủ, khớp với code thực tế đã viết.

---

## Giai đoạn 10 - Quy tắc tài liệu

Mọi thay đổi đều phải có Markdown. Không được bỏ qua. Không được viết tóm tắt. Không được ghi "tương tự". Không được ghi "...". Không được bỏ chi tiết.

Mỗi hàm quan trọng, mỗi module, mỗi task, mỗi queue, mỗi state, mỗi callback, mỗi API đều phải giải thích.

Nếu tham khảo logic từ dự án gốc phải ghi rõ:

* Tên file nguồn
* Hàm nguồn
* Logic được tham khảo
* Đã thay đổi những gì
* Vì sao thay đổi

Nếu viết mới hoàn toàn cũng phải ghi rõ:

* Viết mới
* Không tham khảo
* Lý do thiết kế

---

## Quy tắc bắt buộc

* Không được bỏ qua bất kỳ bước nào, kể cả Giai đoạn 0.
* Không được viết code trước khi hoàn thành phân tích (Giai đoạn 0 → 4).
* Không được copy nguyên code từ dự án gốc.
* Chỉ tham khảo logic và kiến trúc, theo đúng ranh giới đã định nghĩa ở Giai đoạn 4.
* Mọi quyết định kỹ thuật đều phải giải thích bằng Markdown.
* Mọi file Markdown phải được cập nhật khi có thay đổi.
* Toàn bộ mã nguồn mới phải tương thích hoàn toàn với ESP-IDF v6.0.2.
* Nếu phát hiện điểm chưa rõ trong dự án gốc, phải ghi rõ giả định và lý do trước khi tiếp tục, không được tự ý bỏ qua — nếu điểm đó thuộc phạm vi Giai đoạn 0 (phần cứng, audio source, ESP-SR version), bắt buộc phải dừng và hỏi người dùng, không được tự giả định.