# STM32F103 - Bài tập Week 02 bằng thanh ghi

## Thông tin nhóm

**Mã lớp: ELE1415 - Nhóm: 02**

| STT | Họ và tên | Mã sinh viên |
|---:|---|---|
| 1 | Nguyễn Khắc Nhật Huy | B23DCDT123 |
| 2 | Nguyễn Trung Đán | B23DCDT043 |
| 3 | Hoàng Hải Đăng | B23DCDT303 |

## Phạm vi

Dự án triển khai đủ 5 bài Week 02 cho STM32F103 bằng truy cập thanh ghi trực tiếp:

- Không gọi STM32 HAL hoặc Standard Peripheral Library.
- CMSIS chỉ cung cấp tên thanh ghi, bit mask, vector ngắt và startup/linker.
- Build/nạp trên Linux bằng ARM GNU Toolchain, GNU Make và `stlink-tools`.
- Hệ thống chạy HSI 8 MHz để không phụ thuộc thạch anh ngoài.
- Mỗi bài có `main.c` riêng; phần clock, SysTick và UART cơ bản dùng chung trong `Common/`.

Board STM32F103C8 vẫn chạy được firmware này. Dự án dùng cấu hình CMSIS
`STM32F103x6` và linker 32 KiB sẵn có trong repo; toàn bộ chương trình đều nhỏ
hơn giới hạn đó nên cũng tương thích với vùng nhớ lớn hơn của C8.

## Cấu trúc

```text
BaiTap_HTN_W2/
├── Common/
│   ├── Inc/platform.h
│   ├── Src/platform.c
│   └── Makefile.common
├── Drivers/CMSIS/
├── bai-01-uart-buffer/
├── bai-02-systick-leds/
├── bai-03-adc-uart/
├── bai-04-pwm-4ch/
└── bai-05-uart-pwm-control/
```

## Build, nạp và UART

Ví dụ bài 01:

```bash
cd ~/projects/BaiTap_HTN/BaiTap_HTN_W2/bai-01-uart-buffer
make clean
make
make flash
make uart
```

Các bài khác chỉ cần thay tên thư mục. `make flash` nạp file `.bin` tại địa chỉ
`0x08000000`; `make uart` mở `/dev/ttyUSB0` ở 115200 baud.

## Bài 01 - UART nhận chuỗi

Kết nối PA9 (TX) với RX của USB-UART, PA10 (RX) với TX và nối chung GND.
Chương trình cấu hình trực tiếp `GPIOA->CRH`, `USART1->BRR` và `USART1->CR1`.
Khi nhận dấu `!`, board phản hồi:

```text
ELE141502: <bản tin>\n\r
```

## Bài 02 - SysTick và 3 LED

Mỗi chân PA0, PA1, PA2 nối tới anode LED qua điện trở 220-330 ohm; cathode nối GND.

| LED | Chân | Tần số nháy | Đảo mức mỗi |
|---|---|---:|---:|
| LED 1 | PA0 | 0,1 Hz | 5000 ms |
| LED 2 | PA1 | 1 Hz | 500 ms |
| LED 3 | PA2 | 10 Hz | 50 ms |

`SysTick->LOAD` tạo ngắt 1 ms. ISR tăng ba bộ đếm độc lập và đảo bit tương ứng
trong `GPIOA->ODR`; không dùng delay chặn.

## Bài 03 - ADC và UART

- Chân giữa biến trở nối PA0 (`ADC1_IN0`), hai chân ngoài nối 3.3 V và GND.
- UART dùng PA9/PA10 như bài 01.
- `ADC1->SMPR2`, `SQRx`, `CR2` cấu hình một chuyển đổi bằng software trigger.
- ADC được reset calibration và calibration trước khi đo.
- Mỗi giây gửi giá trị ADC và điện áp tính bằng số nguyên:

```text
millivolt = ADC * 3300 / 4095
```

## Bài 04 - PWM 4 kênh 1 kHz

TIM2 điều khiển trực tiếp PA0..PA3:

Mỗi ngõ ra nối tới anode LED qua điện trở 220-330 ohm; cathode nối GND.

| Kênh | Chân | Duty |
|---|---|---:|
| TIM2_CH1 | PA0 | 10% |
| TIM2_CH2 | PA1 | 30% |
| TIM2_CH3 | PA2 | 50% |
| TIM2_CH4 | PA3 | 70% |

Với clock 8 MHz, `PSC=7`, `ARR=999` tạo PWM 1 kHz. Các thanh ghi `CCMR1`,
`CCMR2`, `CCER` chọn PWM mode 1; `CCR1..CCR4` đặt duty.

## Bài 05 - UART ngắt điều khiển PWM

- LED PWM: PA0/TIM2_CH1.
- UART1: PA9/PA10, 115200 8N1.
- `USART1->CR1.RXNEIE` bật ngắt nhận; `USART1_IRQHandler` chỉ gom dữ liệu.
- Vòng lặp chính phân tích lệnh kết thúc bằng `!`.

| Lệnh | Kết quả |
|---|---|
| `ON!` | Bật LED theo duty gần nhất |
| `OFF!` | Tắt LED nhưng giữ cấu hình duty |
| `PWM:35%!` | Cập nhật duty 35% |
| `STATUS!` | Trả về ON/OFF và duty hiện tại |

Khi LED đang OFF, lệnh PWM chỉ cập nhật `pwm_percent`; `TIM2->CCR1` vẫn bằng 0
cho đến khi nhận `ON!`.

## Các thanh ghi chính cần giải thích

| Ngoại vi | Thanh ghi |
|---|---|
| Clock | `RCC->CR`, `RCC->CFGR`, `RCC->APB1ENR`, `RCC->APB2ENR` |
| GPIO | `GPIOA->CRL`, `CRH`, `ODR`, `BRR` |
| UART | `USART1->SR`, `DR`, `BRR`, `CR1` |
| SysTick | `SysTick->LOAD`, `VAL`, `CTRL` (qua `SysTick_Config`) |
| ADC | `ADC1->SR`, `CR1`, `CR2`, `SMPR2`, `SQR1..3`, `DR` |
| PWM | `TIM2->PSC`, `ARR`, `CCMR1/2`, `CCER`, `CCR1..4`, `CR1` |

## Video demo

Video có thể đặt trong `video-demo/` của từng bài hoặc dẫn liên kết trong README.
