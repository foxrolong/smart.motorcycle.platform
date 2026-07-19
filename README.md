<h1 align="center">🚀 ĐIỀU KHIỂN XE MÁY TỪ XA 🚀</h1>
<p align="center">
  <b>🔒 Mở khoá </b>
  <br>
</p>
### tổng quan dự án 
về mặt kết nối:
- xe máy có cơ chế kết nối bluetooth giống như chết đọ tìm IP của  apple
- cơ chế hoạt động khi xe máy ko có mạng như sau.
- trên điện thoại điều khiển xe 
1. tải app () ứng dụng sẽ chạy quyền ngầm trên ip phê duyệt yêu cầu chuyền dữ liệu qua bluetooth
2. sau khi có quyền esp32 gửi BLE kết nối bluetooth với điện thoại
3. ip gần đó tự động gửi dữ liện lên claud ---> sau đó báo về web app <vị trí/thông số xe> 
- trên điện thoại vào web app +mở khóa xe
1. web app gửi lệnh lên cloud 
2. cloud --> gửi lệnh lên ip có mạng/app(ngầm) gần xe yêu cầu mở khóa xe.

### lưu ý ( bảo mật )
<p>
- chỉ có ip được cài app chạy ngầm mới có quyền kết nối vói xe để đảm bảo xe không bị nhận lệnh từ thiết bị khác đảm bảo bảo mật</p>


<p>sơ đồ đơn giản của giao thức kết nối/cơ chế hoạt động.</p>

```
ESP32 (không có WiFi/4G)
        │
        │ Quét BLE liên tục (BLE Scan)
        ▼
Tìm điện thoại đang phát BLE
        │
        ▼
Nhận được:
- MAC Bluetooth
- Tên thiết bị
- RSSI (cường độ tín hiệu)
- UUID dịch vụ
        │
        ▼
ESP32 kết nối BLE (lưu ý: phải có app mã xe riêng trên đt mới kết nối được )
        │
        ▼
Điện thoại nhận yêu cầu
        │
Nếu điện thoại có Internet
        │
        ▼
Điện thoại gửi HTTPS/MQTT lên Cloud
        │
        ▼
Cloud gửi lệnh mở xe
```

```
ESP32
   │
   ├── gửi ID xe
   ├── gửi Challenge
   └── gửi Token
         │
         ▼
Điện thoại
         │
Có Internet
         ▼
Cloud
         │
Cloud xác thực
         ▼
Trả kết quả
         │
         ▼
Điện thoại
         │
BLE
         ▼
ESP32
         │
Mở khóa
```
