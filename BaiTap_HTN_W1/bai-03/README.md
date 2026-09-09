# Bài tập 03 - Đọc 8 input, đảo bit và xuất ra 8 LED

## Yêu cầu

- Cấu hình PA0-PA7 input và PA8-PA15 output.
- Đọc PA0-PA7, đảo từng bit rồi ghi ra PA8-PA15.
- Chỉ thao tác thanh ghi, không sử dụng HAL/STD.

## Đấu dây

Mỗi nút nối giữa chân PA0-PA7 tương ứng và GND. Chương trình bật pull-up nội.
Mỗi chân PA8-PA15 nối tới anode LED qua điện trở 220-330 ohm; cathode nối GND.

| Kênh | Input | Output LED |
|---|---|---|
| 0 | PA0 | PA8 |
| 1 | PA1 | PA9 |
| 2 | PA2 | PA10 |
| 3 | PA3 | PA11 |
| 4 | PA4 | PA12 |
| 5 | PA5 | PA13 |
| 6 | PA6 | PA14 |
| 7 | PA7 | PA15 |

Chỉ dùng mức logic 3,3 V và phải nối chung GND.

## Cấu hình thanh ghi

- Bật clock AFIO và GPIOA bằng `RCC_APB2ENR`.
- `GPIOA_CRL = 0x88888888`: PA0-PA7 input pull-up/pull-down.
- Đặt PA0-PA7 trong `GPIOA_ODR` lên 1 để chọn pull-up.
- `GPIOA_CRH = 0x22222222`: PA8-PA15 output push-pull 2 MHz.
- Đặt `SWJ_CFG = 100` trong `AFIO_MAPR` để dùng PA13-PA15 làm GPIO.

## Thuật toán

```text
Khởi tạo PA0-PA7 input pull-up
Khởi tạo PA8-PA15 output push-pull
Tắt SWD/JTAG để sử dụng PA13-PA15
Lặp vô hạn:
    input = GPIOA_IDR & 0xFF
    output = (~input) & 0xFF
    ghi output vào PA8-PA15 bằng GPIOA_BSRR
```

Không nhấn: input bằng 1, đảo thành 0 nên LED tắt. Nhấn: input bằng 0, đảo
thành 1 nên LED tương ứng sáng.

## Build và nạp

```bash
make clean
make
make flash
```

## Kịch bản video demo

1. Quay rõ kit, ST-Link, 8 nút và 8 LED.
2. Hiển thị lệnh `make` thành công trên Ubuntu.
3. Nhấn lần lượt SW0-SW7 và quay LED0-LED7 sáng tương ứng.
4. Nhấn đồng thời một số nút để chứng minh 8 kênh hoạt động độc lập.
