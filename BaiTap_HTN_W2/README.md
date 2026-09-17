# STM32F103 với HAL - Bài tập Week 02

## Thông tin nhóm

**Lớp: 02**

| STT | Họ và tên | Mã sinh viên |
|---:|---|---|
| 1 | Nguyễn Khắc Nhật Huy | B23DCDT123 |
| 2 | Nguyễn Trung Đán | B23DCDT043 |
| 3 | Hoàng Hải Đăng | B23DCDT303 |

## Phạm vi và công nghệ

Dự án triển khai đủ 5 bài trong Week 02 cho STM32F103x6 low-density
(32 KiB Flash, 10 KiB SRAM):

- Dùng **STM32 HAL**, đúng yêu cầu cho phép dùng HAL hoặc STD.
- Build và nạp bằng ARM GNU Toolchain, GNU Make và `stlink-tools` trên Linux.
- Không phụ thuộc KeilC hoặc STM32CubeIDE khi build.
- Clock hệ thống dùng HSI 8 MHz để không phụ thuộc thạch anh ngoài.
- Mã ứng dụng được viết ngắn, đặt tên theo chức năng và tách khỏi thư viện.

Phần `Drivers/` được lấy từ gói chính thức **STM32CubeF1 v1.8.6** của
STMicroelectronics, sau đó chỉ giữ các thành phần cần cho STM32F103x6.
Giấy phép gốc nằm ngay trong các thư mục driver.

## Cấu trúc dự án

```text
BaiTap_HTN_W2/
├── Common/
│   ├── Inc/                 # Cấu hình HAL và khai báo dùng chung
│   ├── Src/                 # HAL tick, clock HSI và Error_Handler
│   └── Makefile.common      # Quy trình build dùng chung
├── Drivers/
│   ├── CMSIS/               # CMSIS Core + device STM32F103x6
│   └── STM32F1xx_HAL_Driver/
├── bai-01-uart-buffer/
│   ├── Core/Src/main.c
│   └── Makefile
├── bai-02-systick-leds/
├── bai-03-adc-uart/
├── bai-04-pwm-4ch/
└── bai-05-uart-pwm-control/
```

Mỗi bài chỉ chứa logic riêng trong `Core/Src/main.c`. Startup, linker,
CMSIS, HAL và cấu hình build được dùng chung nên không có 5 bản sao khó
bảo trì.

## Build và nạp trên Ubuntu

Ví dụ với bài 01:

```bash
cd ~/BaiTap_HTN/BaiTap_HTN_W2/bai-01-uart-buffer
make clean
make
st-info --probe
make flash
```

Thay tên thư mục để build bài khác. File nạp nằm trong `build/` và có
đuôi `.bin`. Khi nạp thành công, `st-flash` báo
`Flash written and verified!`.

## Bài 01 - UART nhận chuỗi và phản hồi

### Kết nối

| STM32F103 | USB-UART |
|---|---|
| PA9 - USART1_TX | RX |
| PA10 - USART1_RX | TX |
| GND | GND |

Terminal đặt `115200`, `8N1`, không flow control. Chương trình nhận từng
byte vào bộ đệm. Khi nhận `!`, board phản hồi:

```text
ELE141502: <noi dung da nhan>\n\r
```

Trong đó `ELE1415` là mã lớp và `02` là mã nhóm. Ví dụ gửi `Hello!` thì
nhận `ELE141502: Hello`.

## Bài 02 - Ba LED dùng ngắt SysTick

Nối mỗi chân qua điện trở 220-330 ohm tới anode LED, cathode về GND.

| LED | Chân | Tần số nháy | Chu kỳ đổi trạng thái |
|---|---|---:|---:|
| LED 1 | PA0 | 0,1 Hz | 5000 ms |
| LED 2 | PA1 | 1 Hz | 500 ms |
| LED 3 | PA2 | 10 Hz | 50 ms |

HAL cấu hình SysTick 1 ms. Hàm `App_SysTick_1ms()` tăng ba bộ đếm độc lập.
Vì một lần đảo trạng thái chỉ bằng nửa chu kỳ, thời gian đảo chân bằng
`1 / (2 × tần số)`.

## Bài 03 - ADC và UART

### Kết nối

- Hai chân ngoài biến trở: `3.3V` và `GND`.
- Chân giữa biến trở: `PA0` (`ADC1_IN0`).
- UART dùng PA9/PA10 như bài 01.

Mỗi giây, chương trình đọc ADC 12-bit và tính:

```text
millivolt = ADC × 3300 / 4095
```

Ví dụ dữ liệu UART:

```text
ADC=2048 Voltage=1.650 V
```

## Bài 04 - PWM bốn kênh 1 kHz

TIM2 dùng chung một bộ đếm cho bốn kênh:

| Kênh TIM2 | Chân | Duty |
|---|---|---:|
| CH1 | PA0 | 10% |
| CH2 | PA1 | 30% |
| CH3 | PA2 | 50% |
| CH4 | PA3 | 70% |

Mỗi chân nối qua điện trở 220-330 ohm tới LED rồi về GND. Với clock
timer 8 MHz, `PSC=7` và `ARR=999`:

```text
fPWM = 8 MHz / ((7 + 1) × (999 + 1)) = 1 kHz
```

Các giá trị compare tương ứng là 100, 300, 500 và 700 trên 1000 mức.

## Bài 05 - UART ngắt điều khiển PWM

### Kết nối

- LED PWM: PA0 qua điện trở 220-330 ohm xuống GND.
- UART1: PA9/PA10, `115200 8N1`.

UART nhận từng byte bằng `HAL_UART_Receive_IT()`. Callback ngắt chỉ gom
dữ liệu; vòng lặp chính phân tích lệnh để ISR luôn ngắn. Mỗi lệnh phải
kết thúc bằng `!`.

| Lệnh | Kết quả |
|---|---|
| `ON!` | Bật LED theo duty gần nhất |
| `OFF!` | Tắt LED nhưng giữ duty đã cấu hình |
| `PWM:35%!` | Đặt duty 35% |
| `STATUS!` | Trả về trạng thái ON/OFF và duty |

Nếu đang OFF, lệnh `PWM:x%!` chỉ cập nhật biến duty; compare vẫn bằng 0.
Khi nhận `ON!`, chương trình mới áp dụng duty gần nhất. Đây là phần trạng
thái quan trọng nhất của bài 05.

## Video demo

Có thể tải video vào thư mục `video-demo/` của từng bài:

```text
bai-01-uart-buffer/video-demo/demo-bai-01.mp4
bai-02-systick-leds/video-demo/demo-bai-02.mp4
bai-03-adc-uart/video-demo/demo-bai-03.mp4
bai-04-pwm-4ch/video-demo/demo-bai-04.mp4
bai-05-uart-pwm-control/video-demo/demo-bai-05.mp4
```

Không cần tạo `.gitkeep`; khi đã có video, Git sẽ tự theo dõi thư mục.

## Các ý chính khi giải thích

1. `Platform_Init()` khởi tạo HAL, SysTick 1 ms và clock HSI 8 MHz.
2. Mỗi hàm `GPIO_Init`, `UART1_Init`, `ADC1_Init` hoặc `TIM2_PWM_Init`
   chỉ phụ trách một ngoại vi.
3. Bài 02 xử lý ba mốc thời gian trong ngắt SysTick nhưng không dùng delay.
4. Bài 03 đổi ADC sang millivolt bằng số nguyên nên dễ kiểm tra và không
   kéo thư viện số thực vào firmware.
5. Bài 05 tách nhận UART trong ngắt khỏi xử lý lệnh trong vòng lặp chính.
