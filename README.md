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
Các thông tin của USER:
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
Quy trình OTP:<br>
Nhập Email<br>
↓<br>
Gửi OTP<br>
↓<br>
Nhập OTP<br>
↓<br>
Đăng ký thành công<br>
| Tên cột      | Ý nghĩa                    |
| ------------ | -------------------------- |
| otp_id (PK)  | Mã OTP                     |
| user_id (FK) | Thuộc User nào             |
| otp_code     | 123456                     |
| purpose      | register / forgot_password |
| expired_at   | Hết hạn                    |
| status       | used / unused              |
