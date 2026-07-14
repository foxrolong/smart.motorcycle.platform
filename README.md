Các thông tin đối tượng và thuộc tính
1. USERS
Thuộc tính	Kiểu dữ liệu	Mô tả<br>
user_id	INT	Mã người dùng<br>
user_name VARCHAR Tên người dùng<br>
fullname	VARCHAR	Họ tên<br>
email	VARCHAR	Email<br>
phone	VARCHAR	Số điện thoại<br>
password_hash	VARCHAR	Mật khẩu đã mã hóa<br>
google_id	VARCHAR	ID Google<br>
avatar	VARCHAR	Ảnh đại diện<br>
created_at	DATETIME	Ngày tạo<br>
3. AGENT<br>
Thuộc tính	Kiểu dữ liệu	Mô tả<br>
agent_id	INT	Mã Agent<br>
user_id	INT	Chủ sở hữu<br>
model_id	INT	Mô hình AI<br>
agent_name	VARCHAR	Tên Agent<br>
prompt	TEXT	Vai trò của Agent<br>
temperature	FLOAT	Độ sáng tạo<br>
created_at	DATETIME	Ngày tạo<br>
4. AI_MODEL<br>
Thuộc tính	Kiểu dữ liệu<br>
model_id	INT<br>
model_name	VARCHAR<br>
provider	VARCHAR<br>
version	VARCHAR<br>
5. CHAT_MESSAGE<br>
Thuộc tính<br>
message_id<br>
session_id<br>
sender<br>
content<br>
created_at<br>
6. VOICE_SETTING<br>
Thuộc tính<br>
voice_id<br>
user_id<br>
language<br>
speaker<br>
speed<br>
pitch<br>
7. USER_SETTING<br>
Thuộc tính<br>
setting_id<br>
user_id<br>
ai_name<br>
language<br>
default_model<br>
theme<br>
8. OTP<br>
Thuộc tính<br>
otp_id<br>
user_id<br>
otp_code<br>
expired_at<br>
status<br>
