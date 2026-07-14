Yêu cầu :
Đăng nhập
Đăng ký
Email/Gmail/SĐT
Google Login
OTP
Quên mật khẩu
Điều khoản
Chính sách
Cần lưu:
Email/Gmail/SĐT
Google Login
Mật Khẩu
Các thực thể :
USER
OTP
SOCIAL_ACCOUNT
USER_AGREEMENT
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
Quy trình OTP:
Nhập Email
↓
Gửi OTP
↓
Nhập OTP
↓
Đăng ký thành công
| Tên cột      | Ý nghĩa                    |
| ------------ | -------------------------- |
| otp_id (PK)  | Mã OTP                     |
| user_id (FK) | Thuộc User nào             |
| otp_code     | 123456                     |
| purpose      | register / forgot_password |
| expired_at   | Hết hạn                    |
| status       | used / unused              |
