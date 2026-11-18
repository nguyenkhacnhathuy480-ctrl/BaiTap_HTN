# RobotLine Keil Final

Project độc lập cho STM32F103C8T6, FreeRTOS, TB6612FNG và 9 đầu dò analog
(8 mắt PA0–PA7 dùng điều khiển; PB0 chỉ đọc chẩn đoán).
Tầng ứng dụng và điều khiển được viết mới. BSP, startup, driver phần cứng,
CMSIS và FreeRTOS được giữ lại để dùng đúng mạch hiện tại.

## Mở trong Keil

Mở `MDK-ARM/RobotLine.uvprojx` bằng Keil µVision/ARM Compiler 5.06.
Chỉ có một project vận hành, không kèm target tự chạy để thử motor.
**Config → robot_config.h** nằm ngay trong cây Project.
Rebuild tạo `MDK-ARM/Objects/RobotLine.hex`. Build không tự nạp.
Chạy `Tools/Test.ps1` để kiểm thử mô-đun trên Windows.

## Các phần chính

- **Application**: `main.c` khởi tạo; `robot.c` đọc ADC, nút và điều khiển
  motor mỗi 5 ms trong cùng task; `monitor.c` quản lý OLED, UART và LED.
- **Control**: `tracker.c` chuẩn hóa từng mắt và chọn cụm vạch liền nhau;
  `drive.c` quản lý trạng thái, PWM, tốc độ và giới hạn lái.
- **Config**: chân phần cứng, tham số điều khiển, thứ tự mắt, preset và RTOS.
- **Board / Drivers / Startup / FreeRTOS**: tầng thấp và thư viện nền.

## Vận hành và tuning

Giữ độ cao cảm biến, nền/vạch và nguồn như lúc lấy mẫu. Preset trắng–đen
nằm trong `sensor_calibration.h`; nút không thể xóa preset. PA4 là tâm.
Bật nguồn: motor không tự chạy. Đặt xe gần tâm vạch hẹp, đợi OLED hiện
READY, nhấn PB5. PB5 dừng; PB8/PB9 tăng/giảm 2,5 điểm phần trăm PWM;
giữ nút không lặp, nhấn đồng thời PB8+PB9 để dừng. Có còi khi nhận nút.

Khi vạch mất ngắn, xe báo GAP, giữ tỷ lệ PWM gần nhất ở tối đa 28% trong
35 ms. Hai khung hợp lệ liên tiếp trước hạn cho phép bám lại và tăng tốc
dần; hết hạn báo LOST và dừng. Vạch rộng, mơ hồ hoặc lỗi vẫn dừng ngay.
Nếu PA7 hoặc PA0 nhận vạch ở cùng một mép trong ít nhất hai khung liên tiếp
ngay trước khi mất vạch, xe thử ôm góc vuông về phía đó. Bốn khung liên tiếp
cùng mép cho phép rẽ sớm khi vẫn thấy vạch. Bánh trong dừng, bánh ngoài chạy
tối đa 36% PWM. OLED báo 90 LEFT hoặc 90 RIGHT. Vạch về
tâm ổn định thì tăng dần tốc trở lại; quá 450 ms báo 90 TIMEOUT và dừng.
Vùng đen rộng, vạch mơ hồ, lỗi ADC hoặc lỗi chu kỳ vẫn cắt lệnh ngay.
Một dãy mắt không phân biệt chắc chắn góc vuông với xe văng khỏi vạch;
nhánh khôi phục này có giới hạn và không chọn hướng ở giao lộ.
Cắt PWM không phanh hết quán tính. Giữ công tắc nguồn trong tầm tay.

Tuning trong `Config/robot_config.h`: CTRL_BASE_PWM=520 (52%), PB8 có thể
tăng dần tới CTRL_MAX_PWM=620 (62%). CTRL_CORNER_PWM và
CTRL_CORNER_TIMEOUT_MS chỉnh riêng cho góc vuông;
CTRL_KP điều chỉnh lực trả lái; CTRL_KD giảm dao động, có bộ lọc;
CTRL_TRIM_DIV là số chia của bù lệch (số nhỏ hơn cho bù mạnh hơn).
Chỉ chỉnh một tham số mỗi lần; không cần hiệu chuẩn khi chỉnh hệ số.
Mặc định không đảo bánh: xe rẽ bằng cách giảm duty bánh trong.
Khi độ lệch tăng từ `CTRL_CURVE_START` tới `CTRL_CURVE_FULL`, gain P
tăng dần tới `CTRL_CURVE_KP` và nền giảm thêm theo
`CTRL_CURVE_SLOW_EXTRA`. Sàn tương đối `CTRL_CURVE_PWM_FLOOR` được tính
trước khi áp trần `CTRL_CURVE_APPROACH_PWM`; trần giảm dần về 36% khi đạt
độ lệch CTRL_CURVE_FULL để tốc độ thẳng không kéo qua cua gắt.
Xe suy ra nhu cầu rẽ từ độ lệch mắt, không đo bán kính cua.

UART 115200 8N1, RX adapter→PB10 và TX→PB11, GND chung, logic 3,3 V.
Không nối VCC adapter khi kit dùng pin riêng. Lệnh `a` đọc trạng thái,
`x` dừng, `?` trợ giúp. Không hỗ trợ Start từ UART.
`E>0` nghĩa là vạch ở bên phải xe; `L/R` là PWM theo phần nghìn.
Q=0 hợp lệ, 1 không thấy vạch, 2 vùng rộng, 3 hai cụm, 4 preset sai.

Trước khi thử trên sàn, kê hai bánh và kiểm tra vạch bên phải tạo lệnh
L>R, rồi xác nhận bánh trái vật lý được điều khiển mạnh hơn. Thử ngược lại
cho bên trái. Nếu không khớp, kiểm tra đấu dây và vị trí các mắt trước khi
tuning. Chưa có phản hồi RPM hoặc watchdog độc lập; phần mềm không thay
thế công tắc cắt nguồn. Build/test trên máy không chứng minh xe bám thật.
