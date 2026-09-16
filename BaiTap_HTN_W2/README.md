# STM32F103 Bare-Metal - Bài tập Week 02

## Thông tin nhóm

**Lớp: 02**

| STT | Họ và tên | Mã sinh viên |
|---:|---|---|
| 1 | Nguyễn Khắc Nhật Huy | B23DCDT123 |
| 2 | Nguyễn Trung Đán | B23DCDT043 |
| 3 | Hoàng Hải Đăng | B23DCDT303 |

## Môi trường và phần cứng

- STM32F103 Low-density: 32 KiB Flash, 10 KiB SRAM.
- ARM GNU Toolchain, GNU Make, stlink-tools trên Ubuntu.
- Lập trình trực tiếp thanh ghi, không phụ thuộc HAL/STD.
- Clock sau reset: HSI 8 MHz.
- UART1: 115200 bps, 8 data bits, no parity, 1 stop bit.
- Mức logic UART và GPIO: 3,3 V.

## Cấu trúc

```text
BaiTap_HTN_W2/
├── bai-01-uart-buffer/
├── bai-02-systick-leds/
├── bai-03-adc-uart/
├── bai-04-pwm-4ch/
└── bai-05-uart-pwm-control/
```

Mỗi thư mục bài gồm `main.c`, `startup.s`, `linker.ld` và
`Makefile`.

## Build và nạp

Ví dụ với bài 01:

```bash
cd BaiTap_HTN_W2/bai-01-uart-buffer
make clean
make
st-info --probe
make flash
```

Khi thành công, terminal hiển thị `Flash written and verified!`.

## Bài 01 - UART nhận chuỗi và phản hồi

Kết nối USB-UART:

| STM32F103 | USB-UART |
|---|---|
| PA9 (TX) | RX |
| PA10 (RX) | TX |
| GND | GND |

- Terminal: 115200, 8N1, no flow control.
- Gửi chuỗi kết thúc bằng `!`, ví dụ `Hello!`.
- Board phản hồi `0201: Hello\n\r`.
- `02` là mã lớp; mã nhóm mặc định là `01` và có thể đổi
  trong `main.c`.

## Bài 02 - Ba LED nháy bằng ngắt SysTick

Mỗi chân nối tới anode LED qua điện trở 220-330 ohm; cathode nối GND.

| Chân | Tần số nháy |
|---|---:|
| PA0 | 0,1 Hz |
| PA1 | 1 Hz |
| PA2 | 10 Hz |

SysTick tạo ngắt 1 ms; ba bộ đếm thời gian hoạt động độc lập.

## Bài 03 - Đo ADC và gửi điện áp qua UART

- Biến trở: hai chân ngoài nối 3,3 V/GND, chân giữa nối PA0.
- UART1 nối PA9/PA10 như bài 01.
- Board gửi giá trị ADC 12-bit và điện áp tính theo VREF=3,3 V mỗi giây.

Ví dụ:

```text
ADC=2048 Voltage=1.650 V
```

## Bài 04 - PWM bốn kênh 1 kHz

TIM2 phát PWM tại bốn chân. Mỗi chân nối LED qua điện trở
220-330 ohm xuống GND.

| TIM2 | Chân | Duty |
|---|---|---:|
| CH1 | PA0 | 10% |
| CH2 | PA1 | 30% |
| CH3 | PA2 | 50% |
| CH4 | PA3 | 70% |

## Bài 05 - Điều khiển PWM bằng UART ngắt

- LED PWM: PA0 qua điện trở 220-330 ohm xuống GND.
- UART1: PA9/PA10, 115200 8N1.
- Mọi lệnh kết thúc bằng `!`.

| Lệnh | Chức năng |
|---|---|
| `ON!` | Bật LED với duty đã lưu |
| `OFF!` | Tắt LED, không xóa duty đã lưu |
| `PWM:35%!` | Đặt duty 35%; nếu OFF thì chỉ lưu cấu hình |
| `STATUS!` | Trả về trạng thái và duty hiện tại |

## Video demo

Sau khi quay, tải video vào từng thư mục với tên:

- `bai-01-uart-buffer/video-demo/demo-bai-01.mp4`
- `bai-02-systick-leds/video-demo/demo-bai-02.mp4`
- `bai-03-adc-uart/video-demo/demo-bai-03.mp4`
- `bai-04-pwm-4ch/video-demo/demo-bai-04.mp4`
- `bai-05-uart-pwm-control/video-demo/demo-bai-05.mp4`
