# Bài tập 04 - Nút nhấn đảo trạng thái LED

## Yêu cầu

- Một chân input đọc nút nhấn và một chân output điều khiển LED.
- LED chỉ đảo trạng thái sau một thao tác nhấn rồi nhả hoàn chỉnh.
- Giữ hoặc thả nút không làm LED tự đổi liên tục.
- Chỉ thao tác thanh ghi, không sử dụng HAL/STD.

## Lựa chọn chân và đấu dây

- PA0: input đọc nút nhấn, dùng pull-up nội.
- PC13: output điều khiển LED tích hợp trên Blue Pill; LED active-low.
- Nối một đầu nút nhấn vào PA0 và đầu còn lại vào GND.
- ST-Link và kit phải nối chung GND; chỉ dùng mức logic 3,3 V.

## Cấu hình thanh ghi

- Bật clock GPIOA và GPIOC bằng `RCC_APB2ENR`.
- PA0 có `CNF=10`, `MODE=00`: input pull-up/pull-down.
- Đặt bit PA0 trong `GPIOA_ODR` lên 1 để chọn pull-up.
- PC13 có `CNF=00`, `MODE=10`: output push-pull 2 MHz.
- SysTick chạy từ HSI 8 MHz, tạo mốc 1 ms để chống dội phím.

## Thuật toán

```text
Khởi tạo PA0 input pull-up
Khởi tạo PC13 output, LED ban đầu tắt
Khởi tạo SysTick 1 ms
Lặp vô hạn:
    đọc trạng thái nút
    nếu trạng thái thay đổi:
        chờ chống dội 20 ms và đọc lại
        nếu chuyển sang nhấn: ghi nhớ lần nhấn
        nếu chuyển sang nhả và trước đó đã nhấn: đảo LED
```

Biến `press_seen` bảo đảm giữ nút không làm LED đảo liên tục. LED chỉ đổi sau
cạnh nhả của một chu trình nhấn-nhả hợp lệ.

## Build và nạp

```bash
make clean
make
make flash
```

## Kịch bản video demo

1. Quay rõ kit và nút nối giữa PA0-GND.
2. Hiển thị `make` và `make flash` thành công trên Ubuntu.
3. Nhấn và giữ nút: LED không đảo liên tục.
4. Nhả nút: LED đổi trạng thái đúng một lần.
5. Lặp lại nhiều lần để chứng minh hoạt động như nút nguồn.
