<h1 align=center> 🏍️ AI-integrated motorcycle </h1>

xem giao diện đã được thiết kế <span style="color: red;">tại đây --></span> [giao diện](https://www.figma.com/make/4PKpnnqDbTJug1DW5MgsBG/Gmail-Login-Registration?fullscreen=1&t=JW5dcm2HD73Htj33-1&code-node-id=0-6)

## 1. Giao diện (interface).

Web app cho phép chủ xe:
- Xem trạng thái xe (khóa <span style="color: red">[mở/tắt]</span>, tín hiệu kết nối)
- Quản lý thiết bị (thêm/xóa thiết bị)

Kiến trúc tổng thể:<br>
**ESP32 ⇄ Cloud ⇄ web app**

## Trang đăng nhập / đăng ký
- sử dụng phương thức đăng nhập đăng ký bằng gmail/email.
1. Phương thức đăng nhập

   **Cho phép đăng nhập bằng:**
      - Email
      - Gmail

3. Đăng ký tài khoản

   **Cho phép đăng ký bằng:**

      - Email
      - Gmail

   **Yêu cầu:**

      - Gửi mã xác thực (OTP) qua Email/Gmail.
      - Nhập mã xác thực để hoàn tất đăng ký.
      <!-- - Bắt buộc tích chọn:
      - ☑ Tôi đồng ý với **Thỏa thuận người dùng**
      - ☑ Tôi đồng ý với **Chính sách bảo mật** -->

4. Quên mật khẩu

   **Chức năng:**
      - Hỗ trợ lấy lại mật khẩu qua:
      1. Email/Gmail
      2. gửi mã xác nhận tài khoản


## Trang Dashboard (trang chính)

- Trạng thái xe: 🔒 Đã khóa / 🔓 Đã mở
- Nút bấm lớn "Mở khóa" / "Khóa xe"
- Hiển thị: thời gian cập nhật trạng thái gần nhất, cường độ tín hiệu kết nối.

<!-- 1. Trang 1

2. Trang 2 — Cấu hình giọng nói

   - Ngôn ngữ
      - 🇻🇳 Tiếng Việt
      - 🇺🇸 English
   - Tùy chọn
3. Trang 3
   - Chức năng

4. Trang 4 — Cài đặt
   - Thao tác

      - 💾 Lưu cấu hình
      - ❌ Hủy thay đổi -->
<!-- 
## Trang Lịch sử hoạt động
- Danh sách: thời gian, hành động (mở/khóa), thiết bị thực hiện, kết quả (thành công/thất bại)

## Trang Quản lý thiết bị
- Danh sách các điện thoại đăng nhập [chủ xe]
- Thêm thiết bị mới ( cho mượn ) (ghép đôi bằng mã QR hoặc mã xe)
- Thu hồi quyền truy cập của thiết bị (cho mượn)

## Trang Cài đặt tài khoản
- Đổi mật khẩu, bật/tắt 2FA, thông báo đẩy (push notification) khi có ai mở khóa xe



# 🏠 Dashboard (Sau khi đăng nhập thành công)

 -->

---
## 🎯 Mục tiêu giao diện

<!-- - Giao diện hiện đại (Modern Dashboard)
- Thiết kế tối giản (Minimal UI)
- Responsive cho Desktop, Tablet và Mobile
- Hỗ trợ Dark Mode / Light Mode
- Dashboard trực quan, dễ sử dụng
- Tối ưu trải nghiệm người dùng (UX)
- Dễ dàng mở rộng thêm tính năng trong tương lai -->

<!--## 5. Cơ chế xác thực & bảo mật

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
## 7. tham khảo

1. Vẽ wireframe UI chi tiết (Figma) cho từng trang
2. Định nghĩa schema database (bảng `users`, `vehicles`, `bridges`, `commands`, `logs`)
3. Xây dựng backend API + hạ tầng MQTT trước, mock dữ liệu ESP32 để test luồng
4. Sau khi luồng App ↔ Cloud ↔ Web ổn định, tích hợp firmware ESP32 thật
5. Kiểm thử bảo mật (pentest) trước khi lắp lên xe thật-->


