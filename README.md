Các thông tin đối tượng và thuộc tính
1. USERS
Thuộc tính	Kiểu dữ liệu	Mô tả
user_id	INT	Mã người dùng
user_name VARCHAR Tên người dùng
fullname	VARCHAR	Họ tên
email	VARCHAR	Email
phone	VARCHAR	Số điện thoại
password_hash	VARCHAR	Mật khẩu đã mã hóa
google_id	VARCHAR	ID Google
avatar	VARCHAR	Ảnh đại diện
created_at	DATETIME	Ngày tạo
3. AGENT
Thuộc tính	Kiểu dữ liệu	Mô tả
agent_id	INT	Mã Agent
user_id	INT	Chủ sở hữu
model_id	INT	Mô hình AI
agent_name	VARCHAR	Tên Agent
prompt	TEXT	Vai trò của Agent
temperature	FLOAT	Độ sáng tạo
created_at	DATETIME	Ngày tạo
4. AI_MODEL
Thuộc tính	Kiểu dữ liệu
model_id	INT
model_name	VARCHAR
provider	VARCHAR
version	VARCHAR
5. CHAT_MESSAGE
Thuộc tính
message_id
session_id
sender
content
created_at
6. VOICE_SETTING
Thuộc tính
voice_id
user_id
language
speaker
speed
pitch
7. USER_SETTING
Thuộc tính
setting_id
user_id
ai_name
language
default_model
theme
8. OTP
Thuộc tính
otp_id
user_id
otp_code
expired_at
status
