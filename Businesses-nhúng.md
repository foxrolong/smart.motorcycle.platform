<h1 align=center> Đối Với Dự Án Thực Tế "Lập Trình Nhúng" </h1>

## 1. Nghiên cứu Tính khả thi & Tối ưu Chi phí (Feasibility & BOM Analysis)

* **Tài liệu PRD (Product Requirement Document):** Chốt toàn bộ tính năng, môi trường hoạt động (chịu nhiệt, kháng nước IP67...), thời lượng pin.
* **Tối ưu BOM (Bill of Materials - Giá thành linh kiện):**
* Ví dụ: Bán ra thị trường giá $50 thì tổng tiền linh kiện trên bo mạch chỉ được phép rơi vào tầm $10 - $12.
* **Đánh giá Chuỗi cung ứng (Supply Chain Audit):** Chip chọn có dễ mua không? Có nguy cơ bị đứt hàng hay ngừng sản xuất (EOL) trong 3–5 năm tới không?

## 2. Vẽ Sơ Đồ Khối & Sơ Đồ Nguyên Lý (Block Diagram & Schematics)

Bước này thay thế cho việc vẽ sơ đồ Database ở dự án phần mềm:

* **Hardware Team (Phần cứng):** Vẽ sơ đồ nguyên lý (Schematic) và thiết kế mạch in nhiều lớp (PCB Layout 4-8 lớp) trên các phần mềm chuyên nghiệp như Altium Designer, Cadence Allegro.
* **Mechanical Team (Cơ khí):** Thiết kế vỏ hộp 3D (SolidWorks/NX), tính toán tản nhiệt, chống nước và khả năng lắp ráp.
* **Firmware Team (Phần mềm):** Xây dựng kiến trúc hệ điều hành thời gian thực (FreeRTOS / Embedded Linux), thiết kế luồng bảo mật (Secure Boot, Mã hóa dữ liệu) và tính năng cập nhật phần mềm từ xa (OTA).

* **Vẽ Sơ đồ khối:** Khối Nguồn $\rightarrow$ Khối Xử lý trung tâm $\rightarrow$ Khối Đầu vào (Cảm biến) $\rightarrow$ Khối Đầu ra (Relay/Màn hình).
* **Quy định Sơ đồ chân (Pinout Mapping):** Viết rõ ra giấy hoặc phần mềm (Fritzing, Proteus, KiCad) xem **chân nào của cảm biến nối vào chân nào của vi điều khiển**.
* **Sơ đồ trạng thái phần mềm (State Machine):** Mô tả luồng chạy của chương trình (Ví dụ: *Khởi tạo $\rightarrow$ Đọc cảm biến mỗi 2 giây $\rightarrow$ Nếu nhiệt độ > 30°C thì Bật quạt $\rightarrow$ Gửi dữ liệu*).

## 3. Lắp Mạch Nháp & Test Linh Kiện Lẻ (Breadboard & Unit Test)

1. Cắm linh kiện lên **Breadboard** (testboard) theo sơ đồ chân ở Bước 2.
2. Viết các đoạn code ngắn **test riêng từng linh kiện**:
* Code 1: Chỉ đọc và in giá trị cảm biến ra màn hình Serial.
* Code 2: Chỉ điều khiển bật/tắt Relay.
* Code 3: Chỉ test phát sóng Bluetooth/WiFi.
3. Khi chắc chắn linh kiện nào cũng sống và đọc đúng dữ liệu, bạn mới chuyển sang bước tiếp theo.

## 4. Viết Firmware Cốt Lõi & Tích Hợp (Integration)

Khi phần cứng nháp đã chạy ổn định:

* Ghép các đoạn code lẻ ở Bước 3 lại thành một chương trình hoàn chỉnh.
* Quản lý luồng bằng **ngắt (Interrupt)**, **Timer**, hoặc các trạng thái tĩnh (Finite State Machine) để tránh dùng quá nhiều hàm `delay()` làm đơ hệ thống.

## 5. Hàn Mạch & Đóng Vỏ (PCB / Hardware Assembly)

Nội dung này tương đương với bước "Deploy" trong phần mềm:

* Bỏ mạch breadboard cồng kềnh, chuyển sang hàn cố định lên **mạch lỗ (Perfboard)** hoặc vẽ board in **PCB** (bằng EasyEDA/KiCad) rồi đặt mạch về hàn.
* Thiết kế hộp / vỏ bảo vệ (in 3D hoặc dùng hộp nhựa) để bảo vệ linh kiện khỏi va đập, chập mạch.


## 6. Các Giai đoạn Thử nghiệm Chuẩn Công nghiệp (EVT $\rightarrow$ DVT $\rightarrow$ PVT)

Đây là điểm khác biệt lớn nhất giữa công ty và sinh viên. Công ty phải làm qua 3 vòng kiểm thử:

* **Giai đoạn EVT (Engineering Validation Test - Kiểm thử Kỹ thuật):**
* Làm khoảng 10–20 mẫu thử đầu tiên để xác minh phần cứng và phần mềm có chạy đúng chức năng cơ bản không.

* **Giai đoạn DVT (Design Validation Test - Kiểm thử Thiết kế):**
* Làm 50–100 mẫu thử có vỏ hộp hoàn chỉnh.
* Đưa vào phòng lab ép tải: Thả rơi, rung lắc, cho vào lò hấp nhiệt, đưa vào phòng băng, xịt nước, đo nhiễu điện từ (EMC/EMI).

* **Giai đoạn PVT (Production Validation Test - Kiểm thử Sản xuất):**
* Chạy thử dây chuyền sản xuất thật với số lượng nhỏ (200–500 sản phẩm).
* Mục đích: Kiểm tra xem công nhân nhà máy có dễ lắp ráp không, dây chuyền có bị nghẽn ở khâu nào không.

## 7. Xin Chứng nhận Tiêu chuẩn (Certifications & Compliance)

Trước khi được phép bán ra thị trường, sản phẩm bắt buộc phải thông qua các bài kiểm định quốc tế tại các trung tâm đo lường:

* **FCC / CE:** Chứng nhận thiết bị không phát ra sóng nhiễu gây ảnh hưởng thiết bị khác và an toàn cho người dùng.
* **RoHS / REACH:** Chứng nhận linh kiện không chứa chất độc hại (chì, thủy ngân...).

## 8. Sản xuất Hàng loạt & Làm Jig Kiểm thử Tự động (Mass Production)

* **Làm Jig nạp & Test (Test Fixture):** Đội ngũ kỹ sư phải làm ra một thiết bị kiểm thử riêng (Jig test) dành cho nhà máy. Khi công nhân đặt bo mạch vào Jig, máy sẽ tự động nạp Firmware, kiểm tra dòng điện, test Wi-Fi/Bluetooth trong vòng **3 - 5 giây** và báo ĐẠT / LỖI.
* Bắt đầu dán linh kiện tự động (SMT Line) và đóng gói hàng loạt.

## 9. Vận hành, Cập nhật OTA & Bảo trì (Lifecycle Management)

* Quản lý các thiết bị đã bán ra ngoài thị trường thông qua Server Cloud.
* **Cập nhật OTA (Over-The-Air):** Sửa lỗi hoặc thêm tính năng mới cho hàng triệu thiết bị từ xa qua Internet mà không cần thu hồi sản phẩm.

xem cách làm dự án của sinh viên <span style="color: red;">tại link này --></span> [Students](https://github.com/foxrolong/smart.motorcycle.platform/blob/master/Students-nhúng.md)