Yêu cầu :<br>
Đăng nhập<br>
Đăng ký<br>
Email/Gmail/SĐT<br>
Google Login<br>
OTP<br>
Quên mật khẩu<br>
Điều khoản<br>
Chính sách<br>
Cần lưu:<br>
Email/Gmail/SĐT<br>
Google Login<br>
Mật Khẩu<br>
Các thực thể :<br>
USER<br>
OTP<br>
SOCIAL_ACCOUNT<br>
USER_AGREEMENT<br>
Các thông tin của USER:<br>
| Tên cột       | Kiểu         | Ý nghĩa            |
| ------------- | ------------ | ------------------ |
| user_id       | INT (PK)     | Mã người dùng      |
| full_name     | VARCHAR(100) | Họ tên             |
| email         | VARCHAR(100) | Email              |
| phone         | INT(15)      | SĐT                |
| password_hash | VARCHAR(255) | Mật khẩu đã mã hóa |
| status        | ENUM         | active / inactive  |
| created_at    | DATETIME     | Ngày tạo           |
| updated_at    | DATETIME     | Ngày cập nhật      |
<br>
Quy trình OTP:<br>
Nhập Email<br>
↓<br>
Gửi OTP<br>
↓<br>
Nhập OTP<br>
↓<br>
Đăng ký thành công<br>
<br>
| otp_id (PK)  | Mã OTP                     |<br>
| user_id (FK) | Thuộc User nào             |<br>
| otp_code     | 123456                     |<br>
| purpose      | register / forgot_password |<br>
| expired_at   | Hết hạn                    |<br>
| status       | used / unused              |<br>
Việc tạo tài khoản bằng Google :<br>
Google sẽ gửi thông báo xác nhận hoặc ID(áp dụng với việc liên kết mọi tk khác) <br>
Điều khoản:<br>
Điều khoản<br>
Chính sách(cuộn xuống dưới cùng)<br>
☑ Tôi đồng ý<br>
Điều này giúp người dùng sẽ biết được thông tin tài khoản đã lập <br>

