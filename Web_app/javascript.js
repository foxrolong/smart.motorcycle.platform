// alert('Hello, World!');
// let long;
// long='hello';

// alert(long);
// let admin, name;
// admin = 'john';
// name = admin;
// alert(name);

// let HTcuachungta;
// HTcuachungta = 'Hello, World!';

// alert (NaN ** 0);  ngoại lệ ('**'=1)
// alert (NaN ** 1);  bình thường ('+'/'-'*')(=NaN)

// let str = 'string';
// Biểu thức bên trong ${…}được đánh giá và kết quả trở thành một phần của chuỗi. Chúng ta có thể đặt bất cứ thứ gì vào đó: một biến như namehoặc một biểu thức số học như 1 + 2hoặc thứ gì đó phức tạp hơn.
// let phrase = `can embed another ${str}`;
// alert(phrase);

// console.log(q=9007199254740991 + 2);
// console.log(9007199254740991 + 2);
// alert(q);
// typeof null;
// alert(typeof null); // "object"  (thực tế là null không phải là một đối tượng, đây là một lỗi trong JavaScript)

// kiểu dữ liệu trong JavaScript 
// typeof undefined // "undefined"
// typeof 0 // "number" 
// typeof 10n // "bigint"
// typeof true // "boolean"
// typeof "foo" // "string"
// typeof Symbol("id") // "symbol"
// typeof Math // "object"  (1)
// typeof null // "object"  (2)
// typeof alert // "function"  (3)

// let age = prompt('How old are you?', 100);
// alert(`You are ${age} years old!`); // You are 100 years old!

// bài tập hãy viết ra một đoạn code để hỏi người dùng về tên của họ và sau đó hiển thị tên đó trong một thông báo. 
// 'use strict';
// let name = prompt("my name is?", "");
// alert(name);

//"Chuyển đổi chuỗi" (String Conversion)
// let long = true;
// long = String(long); // ép kiểu
// alert(typeof long);
// let long = 4<1;
// long = String(long);
// alert(typeof long);

// "Chuyển đổi số" (Number Conversion)
// let long = '123';
// long = Number(long);
// alert(typeof long);

// let long = prompt("Nhập vào một số","");
// alert(`Bạn vừa nhập vào số: ${long}`);
// alert(typeof long);

// alert( 2 ** 2 ); // 2² = 4
// alert( 5 % 2 ); // 1, the remainder of 5 divided by 2

// alert( '1' - '10' + 4 - 2 );
// alert( '1' + 4 - 2 + '10');
// '14'-2=12 12+'10'='1210'
//lưu ý: khi gặp toán tử +, JavaScript sẽ thực hiện nối chuỗi.
// khi chuỗi + với số, số sẽ được chuyển đổi thành chuỗi.
// khi chuỗi - với số, chuỗi sẽ được chuyển đổi thành số.

// thứ tự ưu tiên của các toán tử trong JavaScript
// Thứ tự ưu tiên	Tên	Dấu hiệu
// …	…	…
// 14	đơn cộng	+
// 14	phủ định đơn nhất	-
// 13	lũy thừa	**
// 12	phép nhân	*
// 12	phân công	/
// 11	phép cộng	+
// 11	phép trừ	-
// …	…	…
// 2	phân công	=
// …	…	…

// let n = 2;
// n += 5; // now n = 7 (same as n = n + 5)
// n *= 2; // now n = 14 (same as n = n * 2)
// alert( n ); // 14

// Các toán tử ++AND --có thể được đặt trước hoặc sau biến.

// Khi toán tử đứng sau biến, nó ở dạng "hậu tố": counter++.
// "Dạng tiền tố" là khi toán tử đứng trước biến: ++counter.

//Toán tử bitwise
// Các toán tử bitwise coi các đối số là các số nguyên 32 bit và hoạt động ở cấp độ biểu diễn nhị phân của chúng.

// Các toán tử này không chỉ dành riêng cho JavaScript. Chúng được hỗ trợ trong hầu hết các ngôn ngữ lập trình.

// Danh sách các nhà mạng:

// VÀ ( &)
// HOẶC ( |)
// XOR ( ^)
// KHÔNG ( ~)
// DỊCH CHUYỂN SANG TRÁI ( <<)
// PHÍM PHẢI ( >>)
// ZERO-FILL RIGHT SHIFT ( >>>)

// "" + 1 + 0 // '10'
// "" - 1 + 0 // -1
// true + false // 1
// 6 / "3" // 2
// "2" * "3" //6
// 4 + 5 + "px" // "9px"
// "$" + 4 + 5 // "$45"
// "4" - 2 // 2
// "4px" - 2 // NaN
// "  -9  " + 5 // "  -9  5"
// "  -9  " - 5 // -14
// null + 1 // 1
// undefined + 1 //NaN
// " \t \n" - 2 // -2

