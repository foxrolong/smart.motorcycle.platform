<h1 align=center> 🏍️ Thiết Kế Web App Điều Khiển Xe Máy Từ Xa </h1>

xem trước giao diện thiết kế <span style="color: red;">tại đây --></span> [giao diện](https://www.figma.com/proto/lz2H8DAKdBCGt89Gml0XGS/UX-UI?node-id=0-1&t=juMv1ijA6PWIwJA4-0&scaling=scale-down&content-scaling=fixed&page-id=0%3A1&starting-point-node-id=2007%3A114&fuid=1645916119517119414)

## 1. Mục tiêu

Web app cho phép chủ xe:
- Xem trạng thái xe (khóa <span style="color: red">[mở/tắt]</span>, vị trí, tín hiệu kết nối)
- Gửi lệnh mở/khóa xe từ xa
- Xem lịch sử thao tác
- Quản lý thiết bị (thêm/xóa điện thoại được ủy quyền làm "cầu nối" BLE)
- cho mượn xe ( mục quản lý cho mượn xe. phải được chủ xe cấp quyền và chỉ có thể <span style="color: red"> [tắt/mở] </span>, chủ xe có thể khóa bất kì lúc nào khi có vấn đề)

Kiến trúc tổng thể (dựa theo sơ đồ trong README):<br>
**ESP32 (BLE) ⇄ App điện thoại (cầu nối) ⇄ Cloud ⇄ Web App**.

---

## 2. Sơ đồ luồng dữ liệu

```
[Web App] --HTTPS--> [Cloud/Backend] --Push/MQTT--> [App điện thoại gần xe]
                                                          │ BLE
                                                          ▼
                                                       [ESP32 trên xe]
```

Chiều ngược lại (báo trạng thái):
```
[ESP32] --BLE--> [App điện thoại] --HTTPS/MQTT--> [Cloud] --WebSocket/Poll--> [Web App]
```

---

## 3. Các trang (pages) của Web App

### Trang đăng nhập / đăng ký
- Đăng nhập bằng số điện thoại/email + mật khẩu, nên có OTP hoặc 2FA vì đây là app điều khiển thiết bị vật lý.

### Trang Dashboard (trang chính)
- Trạng thái xe: 🔒 Đã khóa / 🔓 Đã mở
- Nút bấm lớn "Mở khóa" / "Khóa xe" (có xác nhận lại trước khi gửi lệnh)
- Hiển thị: thời gian cập nhật trạng thái gần nhất, cường độ tín hiệu BLE, pin điện thoại đang làm cầu nối (nếu có)
- Vị trí xe trên bản đồ (nếu có GPS trên điện thoại cầu nối hoặc module GPS riêng)

### Trang Lịch sử hoạt động
- Danh sách: thời gian, hành động (mở/khóa), thiết bị thực hiện, kết quả (thành công/thất bại)

### Trang Quản lý thiết bị
- Danh sách các điện thoại đăng nhập [chủ xe]
- Thêm thiết bị mới ( cho mượn ) (ghép đôi bằng mã QR hoặc mã xe)
- Thu hồi quyền truy cập của thiết bị (cho mượn)

### Trang Cài đặt tài khoản
- Đổi mật khẩu, bật/tắt 2FA, thông báo đẩy (push notification) khi có ai mở khóa xe

---

## 4. Thiết kế API (Backend)

| Chức năng | Method | Endpoint |
|---|---|---|
| Đăng nhập | POST | `/api/auth/login` |
| Lấy trạng thái xe | GET | `/api/vehicle/{id}/status` |
| Gửi lệnh mở/khóa | POST | `/api/vehicle/{id}/command` |
| Lịch sử thao tác | GET | `/api/vehicle/{id}/logs` |
| Danh sách thiết bị cầu nối | GET | `/api/vehicle/{id}/bridges` |
| Thêm thiết bị cầu nối | POST | `/api/vehicle/{id}/bridges` |
| Nhận báo cáo trạng thái từ app cầu nối | POST | `/api/bridge/report` |

Realtime trạng thái nên dùng **WebSocket** hoặc **MQTT over WebSocket** thay vì poll liên tục, để cập nhật ngay khi có thay đổi.

---

## 5. Cơ chế xác thực & bảo mật

Vì đây là hệ thống điều khiển thiết bị vật lý, cần nhiều lớp bảo mật hơn app thông thường:

1. **Challenge–response giữa ESP32 và Cloud**: ESP32 không tin tưởng bất kỳ điện thoại nào tuyệt đối — mỗi lệnh mở khóa cần một token có chữ ký (signed token) mà chỉv Cloud biết cách tạo, ESP32 xác minh bằng khóa bí mật đã nạp sẵn (không truyền qua kênh không mã hóa).
2. **Whitelist thiết bị cầu nối**: chỉ điện thoại đã được đăng ký (MAC BLE + app key) mới được ESP32 chấp nhận kết nối.
3. **Mã hóa đầu cuối**: dữ liệu giữa App ↔ Cloud dùng HTTPS/TLS; giữa App ↔ ESP32 dùng BLE có mã hóa (bonding + encryption, không dùng "Just Works" pairing).
4. **Chống replay attack**: mỗi lệnh có timestamp + nonce, ESP32 từ chối lệnh cũ bị gửi lại.
5. **Giới hạn tốc độ (rate limit)** trên API để tránh bị dò lệnh hàng loạt.
6. **Thông báo tức thời**: mỗi lần mở khóa thành công, gửi push notification cho chủ xe để phát hiện truy cập trái phép.

> Lưu ý: vấn đề bảo mật

---

## 6. Luồng use-case chính: "Mở khóa xe"

1. Người dùng mở Web App → bấm "Mở khóa"
2. Web App gọi `POST /api/vehicle/{id}/command` kèm JWT xác thực người dùng
3. Cloud kiểm tra quyền sở hữu xe → tạo lệnh có chữ ký + đẩy xuống app cầu nối đang online gần xe nhất (qua MQTT/push)
4. App cầu nối nhận lệnh → gửi qua BLE cho ESP32
5. ESP32 xác minh chữ ký lệnh → nếu hợp lệ, kích relay mở khóa → gửi ACK ngược lại
6. App cầu nối báo kết quả về Cloud → Cloud cập nhật trạng thái → đẩy realtime về Web App
7. Web App hiển thị "Đã mở khóa thành công" + ghi log

---

<!-- ## Gợi ý wireframe cấu trúc thư mục Frontend

```
src/
├── pages/
│   ├── login/
│   ├── dashboard/
│   ├── history/
│   ├── devices/
│   └── settings/
├── components/
│   ├── VehicleStatusCard/
│   ├── UnlockButton/
│   ├── MapView/
│   └── DeviceList/
├── hooks/
│   └── useVehicleSocket.ts   // kết nối realtime
├── services/
│   └── api.ts
└── store/                    // state management (Zustand/Redux)
```

-->
## 7. tham khảo

1. Vẽ wireframe UI chi tiết (Figma) cho từng trang
2. Định nghĩa schema database (bảng `users`, `vehicles`, `bridges`, `commands`, `logs`)
3. Xây dựng backend API + hạ tầng MQTT trước, mock dữ liệu ESP32 để test luồng
4. Sau khi luồng App ↔ Cloud ↔ Web ổn định, tích hợp firmware ESP32 thật
5. Kiểm thử bảo mật (pentest) trước khi lắp lên xe thật


