// Lấy thẻ input có id là 'lightToggle' từ HTML
const toggleBtn = document.getElementById('lightToggle');

// Lắng nghe sự kiện 'change' (khi nút gạt thay đổi trạng thái)
toggleBtn.addEventListener('change', function() {
  
  if (this.checked) {
    // KHI BẬT ĐÈN (Nút gạt xanh)
    document.body.style.backgroundColor = '#ffffff'; // Nền sáng
    document.body.style.color = '#333333';           // Chữ tối màu
    console.log("Trạng thái: ĐÈN BẬT");
    
    // Nếu bạn muốn gửi lệnh tới thiết bị thật (ví dụ ESP8266), thêm API ở đây:
    // fetch('http://IP_CỦA_THIẾT_BỊ/turn-on');
    
  } else {
    // KHI TẮT ĐÈN (Nút gạt xám)
    document.body.style.backgroundColor = '#121212'; // Nền tối
    document.body.style.color = '#ffffff';           // Chữ sáng màu
    console.log("Trạng thái: ĐÈN TẮT");
    
    // Nếu bạn muốn gửi lệnh tới thiết bị thật:
    // fetch('http://IP_CỦA_THIẾT_BỊ/turn-off');
  }
});