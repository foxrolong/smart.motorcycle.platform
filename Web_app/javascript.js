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

// "Chuyển đổi chuỗi" (String Conversion)
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
// lưu ý: khi gặp toán tử +, JavaScript sẽ thực hiện nối chuỗi.
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

// Toán tử bitwise
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

// toán tử điều kiện (conditional operator) 
// hay còn gọi là toán tử 3 ngôi (ternary operator) 
// là toán tử duy nhất trong JavaScript có 3 toán hạng.
// Nó thường được sử dụng như một cách rút gọn cho câu lệnh if...else.
// Cú pháp: condition ? expr1 : expr2
// giống như if else; nếu condition (điều kiện) đúng(true) & sai(false)
// thì toán tử trả về giá trị của expr1, ngược lại trả về giá trị của expr2.
// Ví dụ:
// let accessAllowed;
// let age = prompt('How old are you?', 18);
// accessAllowed = (age > 18) ? true : false;
// alert(accessAllowed);

// let age = prompt('How old are you?', 18);
// let message = (age < 3) ? 'Hi, baby!' : //đúng thì trả về 'Hi, baby!' sai thì tiếp tục kiểm tra điều kiện tiếp theo
//   (age < 18) ? 'Hello!' : // cũng như trên.
//   (age < 100) ? 'Greetings!' : // cũng như trên.
//   'What an unusual age!'; // sai tức là >100 thì trả về 'What an unusual age!'
// alert(message);

// trong truong hợp này, lưu ý: Trong ví dụ trên, 
// bạn có thể tránh sử dụng toán tử dấu chấm hỏi 
// vì bản thân phép so sánh đã trả về giá trị true/false

// let age = prompt('How old are you?', 18);
// let accessAllowed = (age > 18) || confirm('Do you have permission from your parents?');
// alert(accessAllowed);

// Web_app/image/image.png
// cơ bản
// let a = prompt("Tên gọi “chính thức” của JavaScript là gì?", '');
// if (a == 'ECMAScript') {
//   alert('Đúng rồi!');
// } else {
//   alert('Bạn không biết? “ECMAScript”!');
// }
// nâng cao
// let a = prompt("Tên gọi “chính thức” của JavaScript là gì?", '');
// (a == 'ECMAScript') ? alert('Đúng rồi!') : alert('Bạn không biết? “ECMAScript”!

// Web_app/image/1image.png
// nâng cao
// let a = prompt("Nhập số bất kỳ", '');
// (a == 0) ? alert(0) : (a > 0) ? alert(1) : alert(-1);
// // cơ bản
// let a = prompt("Nhập số bất kỳ", '');
// if (a > 0) {
//   alert(1);
// } else if (a < 0) {
//   alert(-1);
// } else {
//   alert(0);
// } 

// toán tuer or (||) và and (&&)
// toán tử or:
// let hour = 9;
// if (hour < 10 || hour > 18) {
//   alert('The office is closed.');
// } else {
//   alert('The office is open.');
// } 
// lưu ý khi toán tử or 
// giá trị sai đầu tiên thì được trả về ngay lập tức.
// result = value1 || value2 || value3;
// Nếu value1 là true, nó sẽ được trả về ngay lập tức,
// nếu không, value2 sẽ được kiểm tra, v.v.

// toán tử and:
// let hour = 12;
// let minute = 30; 
// if (hour == 12 && minute == 30) {
//   alert('The time is 12:30.');
// } else {
//   alert('The time is not 12:30.');
// }
// lưu ý khi toán tử and
// result = value1 && value2 && value3;
// Nếu value1 là 'false', nó sẽ được trả về ngay lập tức,
// nếu không, value2 sẽ được kiểm tra, v.v.'

// toán tử NOT:
// let hour = 9; 
// if (!(hour >= 10 && hour <= 18)) {
//   alert('The office is closed.');
// } else {
//   alert('The office is open.');
// }
// lưu ý khi toán tử NOT
// Toán tử NOT (!) chuyển đổi giá trị sang kiểu boolean trước khi đảo ngược nó. 
// Do đó, !0 trở thành true, và !1 trở thành false.

// // bài tập:
// "use strict";
// let a = prompt("nhập tài khoản","");
// if(a == null || a == ''){
//   alert("đã hủy");
// }else if(a !='Admin'){
//   alert('sai tài khoản');
// }else{
//   let n = prompt("nhập mật khẩu","");
//   if(n == null || n == ''){
//     alert("đã hủy");
//   }else if(n !='long'){
//     alert('sai tài khoản');
//   }else{
//     alert("welcome!");
//   }
// }
// bài tập giải:
// let userName = prompt("Who's there?", '');

// if (userName === 'Admin') {

//   let pass = prompt('Password?', '');

//   if (pass === 'TheMaster') {
//     alert( 'Welcome!' );
//   } else if (pass === '' || pass === null) {
//     alert( 'Canceled' );
//   } else {
//     alert( 'Wrong password' );
//   }

// } else if (userName === '' || userName === null) {
//   alert( 'Canceled' );
// } else {
//   alert( "I don't know you" );
// }

//toán tủ hợp nhất rỗng ??
// toán tử hợp nhất rỗng (??) 
// trả về toán hạng bên phải khi toán hạng bên trái là null,undefined,
// ngược lại trả về toán hạng bên trái.
// let height = null;
// let width = null;

// // important: use parentheses
// let area = (height ?? 100) * (width ?? 50);

// alert(area); // 5000