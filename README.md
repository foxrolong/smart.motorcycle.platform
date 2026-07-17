````md
## 1. Đăng nhập / Đăng ký

### Màn hình đăng nhập

**Chức năng:**

- Đăng nhập
- Đăng ký tài khoản

### Phương thức đăng nhập

- Email
- Gmail
- Số điện thoại
- Đăng nhập trực tiếp bằng **Google (OAuth)**

### Đăng ký tài khoản

**Cho phép đăng ký bằng:**

- Email
- Gmail
- Số điện thoại

**Yêu cầu:**

- Gửi mã xác thực (OTP) qua Email/Gmail/SĐT.
- Nhập mã xác thực để hoàn tất đăng ký.
- Bắt buộc tích chọn:

  - ☑ Tôi đồng ý với **Thỏa thuận người dùng**
  - ☑ Tôi đồng ý với **Chính sách bảo mật**

### Quên mật khẩu

- Hỗ trợ lấy lại mật khẩu qua:
  - Email
  - Gmail
  - Số điện thoại

---

# 🏠 Dashboard (Sau khi đăng nhập thành công)

## Trang 1 — Danh sách Agent

### Chức năng

- Hiển thị danh sách Agent
- Thêm thiết bị
- Tạo Agent mới
- Chỉnh sửa Agent
- Xóa Agent

---

## Trang 2 — Cấu hình giọng nói

### Ngôn ngữ

- 🇻🇳 Tiếng Việt
- 🇺🇸 English

### Tùy chọn

- Chọn giọng nói
- Điều chỉnh tốc độ nói
- Điều chỉnh cao độ
- Nghe thử

---

## Trang 3 — Lịch sử trò chuyện

### Chức năng

- Danh sách cuộc hội thoại
- Xem chi tiết
- Tìm kiếm
- Xóa lịch sử
- Lọc theo thời gian

---

## Trang 4 — Cài đặt

### Cấu hình Agent

- Ngôn ngữ giao diện
- Tên AI
- Mô tả / Giới thiệu Agent
- Prompt vai trò của Agent

### Chọn mô hình AI

- GPT
- Qwen
- DeepSeek
- Gemini

### Thao tác

- 💾 Lưu cấu hình
- ❌ Hủy thay đổi

---

# 🧩 Kiến trúc chức năng

```text
Đăng nhập
        │
        ▼
Dashboard
│
├── Agent
│   ├── Danh sách Agent
│   ├── Thêm thiết bị
│   └── Tạo Agent
│
├── Voice
│   ├── Chọn ngôn ngữ
│   ├── Chọn giọng nói
│   └── Nghe thử
│
├── Chat History
│   ├── Danh sách hội thoại
│   ├── Tìm kiếm
│   └── Xóa lịch sử
│
└── Settings
    ├── Tên AI
    ├── Prompt Agent
    ├── Model AI
    ├── Ngôn ngữ
    ├── Lưu
    └── Hủy
```

---

# 👥 Phân chia hạng mục phát triển

| Mảng | Công việc |
|------|-----------|
| 🎨 UX/UI Design | Thiết kế giao diện, Prototype, Design System |
| 🧪 Tester / QA | Kiểm thử chức năng, UI, API, báo lỗi |
| 🗄 Database | Thiết kế CSDL, lưu trữ User, Agent, Chat History, Settings |
| 💻 Frontend | Xây dựng giao diện Web, Dashboard, Responsive |
| ⚙ Backend | API, Authentication, Agent, AI, Database |
| 🔒 Security | *Tạm thời chưa triển khai* |

---

# 🗄 Đề xuất Database

## User

- id
- name
- email
- phone
- password
- avatar
- role
- created_at

## Agent

- id
- user_id
- name
- description
- prompt
- model
- language
- voice
- created_at

## Device

- id
- user_id
- device_name
- device_type
- status

## Chat History

- id
- user_id
- agent_id
- question
- answer
- created_at

## Settings

- id
- user_id
- language
- theme
- ai_name
- default_model

---

# 🎯 Mục tiêu giao diện

- Giao diện hiện đại (Modern Dashboard)
- Thiết kế tối giản (Minimal UI)
- Responsive cho Desktop, Tablet và Mobile
- Hỗ trợ Dark Mode / Light Mode
- Hiệu ứng chuyển trang mượt mà
- Dashboard trực quan, dễ sử dụng
- Tối ưu trải nghiệm người dùng (UX)
- Dễ dàng mở rộng thêm tính năng trong tương lai
````
