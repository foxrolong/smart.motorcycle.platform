Kiến trúc tổng thể

```
             Luôn chạy
                │
                ▼
        I2S Microphone
                │
                ▼
        Audio Front End (AFE)
    ┌────────────────────────┐
    │ Noise Suppression (NS) │
    │ Voice Activity (VAD)   │
    │ Echo Cancel (AEC)      │
    └────────────────────────┘
                │
                ▼
      WakeNet (ESP-SR)
      "Hi ESP" hoặc từ kích hoạt
                │
      phát hiện thành công
                ▼
     RGB LED sáng màu xanh
                │
                ▼
      MultiNet hoặc Record
       │                 │
       │                 ├── gửi âm thanh lên Server
       │                 │
       └── nhận lệnh offline
                │
                ▼
        AI xử lý / MQTT / HTTP
                │
                ▼
      Điều khiển LED, Relay...
```

Đây chính là kiến trúc mà Espressif thiết kế cho ESP-SR.

Bước 1. ESP32-S3 nhận âm thanh

Thông thường sẽ dùng microphone I2S như

INMP441
ICS43434
ES7210 (2 mic)

Pipeline:

Mic
 ↓
I2S Driver
 ↓
16kHz
 ↓
PCM Buffer
Bước 2. Audio Front End (AFE)

AFE là phần cực kỳ quan trọng.

Nó sẽ làm

Mic

↓

Noise Suppression

↓

Echo Cancellation

↓

Automatic Gain

↓

Voice Activity Detection

↓

WakeNet

Nếu bỏ AFE thì khả năng nhận sai rất cao.

AFE của ESP-SR đã tích hợp:

NS
VAD
AGC
AEC
WakeNet

Bước 3. Wake Word

Ví dụ:

Hi ESP

ESP sẽ luôn nghe.

Khi phát hiện:

wake == true

thì chuyển sang trạng thái

LISTENING
Bước 4. Bật LED RGB

Đây là phần bạn muốn.

Giả sử đang dùng WS2812.

Có thể làm state như:

WAIT_WAKE

↓

Wake detected

↓

RGB = BLUE

↓

Record Audio

↓

Send Server

↓

RGB = GREEN

↓

Receive Response

↓

RGB = WHITE

↓

Sleep

↓

RGB OFF

Ví dụ:

OFF
↓

Nghe

↓

"Hi ESP"

↓

LED xanh

↓

Đang ghi âm

↓

LED tím

↓

Đang gửi WIFI

↓

LED vàng

↓

Đã nhận phản hồi

↓

LED trắng

↓

OFF

Sau này nhìn LED là biết ESP đang ở bước nào.

Bước 5. Sau khi Wake

Có hai hướng.

Hướng 1 (Offline)

Wake

↓

MultiNet

↓

Turn on light

↓

GPIO ON

Không cần Internet.

Hướng 2 (AI)

Wake

↓

Record

↓

PCM

↓

Opus

↓

WiFi

↓

Python Server

↓

Whisper

↓

LLM

↓

TTS

↓

ESP phát loa

Đây là hướng XiaoZhi sử dụng.

Trong ESP-IDF sẽ có các Task

Thường chia như sau:

app_main()

↓

Create Task

├── Audio Task

├── Wake Task

├── RGB Task

├── Wifi Task

├── Recorder Task

└── Player Task

Các task giao tiếp bằng Queue/EventGroup thay vì gọi trực tiếp.

State Machine

Đây là phần rất nên làm ngay từ đầu.

IDLE

↓

LISTEN

↓

WAKE

↓

RECORD

↓

UPLOAD

↓

WAIT_AI

↓

PLAY_AUDIO

↓

IDLE

Mỗi state sẽ quyết định LED RGB.

Ví dụ

IDLE

LED OFF
LISTEN

LED DIM BLUE
WAKE

FLASH BLUE
UPLOAD

YELLOW
PLAY_AUDIO

GREEN
Cấu trúc project mình khuyên
main/

    app_main.c

components/

    audio/
        i2s_mic.c
        recorder.c

    sr/
        wake.c
        multinet.c
        afe.c

    rgb/
        rgb.c

    wifi/
        wifi.c

    network/
        websocket.c
        http.c

    player/
        speaker.c

    ai/
        protocol.c

    common/
        state.c

Đừng để toàn bộ logic trong main.c, vì sau này sẽ rất khó mở rộng.

Lộ trình học theo từng tuần
Giai đoạn 1 – Phần cứng
ESP-IDF project
I2S microphone
WS2812 RGB
Loa I2S (nếu có)
Giai đoạn 2 – Âm thanh
Đọc PCM
Buffer
Ring Buffer
FreeRTOS Queue
Giai đoạn 3 – ESP-SR
AFE
WakeNet
MultiNet
Voice Activity Detection
Giai đoạn 4 – AI
Gửi âm thanh lên server
Nhận kết quả
Phát lại âm thanh
Giai đoạn 5 – Hoàn thiện
OTA
MQTT
Web Config
Deep Sleep
Quản lý trạng thái và LED RGB