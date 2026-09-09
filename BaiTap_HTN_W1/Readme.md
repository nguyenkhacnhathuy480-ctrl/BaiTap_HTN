# STM32F103 Bare-Metal - Bài tập 01 đến 04

Họ và tên: **Nguyễn Khắc Nhật Huy**

Mã sinh viên: **Điền mã sinh viên**

Lớp: **Điền tên lớp**

## Giới thiệu

Repository thực hiện bài tập tuần 01 trên STM32F103 bằng phương pháp bare-metal:

- Lập trình trực tiếp thanh ghi ngoại vi.
- Không sử dụng HAL, Standard Peripheral Library hoặc KeilC.
- Build trên Ubuntu bằng ARM GNU Toolchain.
- Nạp chương trình bằng ST-Link Tools.

Mạch thực tế được nhận dạng là STM32F1 Low-density, có 32 KiB Flash và 10 KiB
SRAM. Linker script đã được cấu hình theo đúng dung lượng này.

## Cấu trúc repository

```text
bare-metal-week-01/
├── bai-01/
│   ├── main.c
│   ├── startup.s
│   ├── linker.ld
│   └── Makefile
├── bai-02/
│   ├── main.c
│   ├── startup.s
│   ├── linker.ld
│   └── Makefile
├── bai-03/
│   ├── main.c
│   ├── startup.s
│   ├── linker.ld
│   ├── Makefile
│   └── video-demo/
├── bai-04/
│   ├── main.c
│   ├── startup.s
│   ├── linker.ld
│   ├── Makefile
│   └── video-demo/
```

## Công cụ đã sử dụng

- GNU Tools for STM32 14.3.1
- stlink-tools 1.8.0
- GNU Make trên Ubuntu

## Build

Chọn thư mục bài cần build, ví dụ:

```bash
cd bai-01
make clean
make
```

Bài 01 có thể thay đổi thời gian giữa hai lần đảo LED:

```bash
make clean && make BLINK_INTERVAL_MS=500
```

Bài 02 có thể thay đổi thời gian dừng ở mỗi LED:

```bash
make clean && make LED_STEP_MS=250
```

Mỗi project tạo tệp `.elf`, `.bin` và `.map` trong thư mục `build/`.

## Bài 01 - LED PC13 nhấp nháy

- PC13 được cấu hình output push-pull 2 MHz.
- LED tích hợp trên Blue Pill hoạt động mức thấp.
- LED đảo trạng thái mặc định mỗi 1000 ms.
- Đổi tham số `BLINK_INTERVAL_MS` khi build để thay đổi tốc độ.

## Bài 02 - LED chạy hai chiều

- PA0-PA7 được cấu hình output push-pull 2 MHz.
- Nối mỗi chân PA0-PA7 tới anode của một LED qua điện trở
  220-330 ohm; cathode nối GND.
- Bố trí PA0 ở bên trái và PA7 ở bên phải.
- Chương trình chỉ bật một LED, chạy PA0→PA7→PA0 và lặp lại.
- Đổi tham số `LED_STEP_MS` khi build để thay tốc độ LED chạy.

## Bài 03 - Đọc 8 input, đảo bit và xuất ra 8 LED

- PA0-PA7: input pull-up nội; mỗi nút nhấn nối giữa chân PA tương ứng và GND.
- PA8-PA15: output; mỗi chân nối anode LED qua điện trở 220-330 ohm,
  cathode nối GND.
- Quan hệ input/output: PA0→PA8, PA1→PA9, ..., PA7→PA15.
- Nhấn nút sẽ làm LED tương ứng sáng; các kênh hoạt động độc lập.
- Firmware tắt SWD/JTAG để dùng PA13-PA15 làm GPIO. Sau khi nạp,
  ngắt SWDIO/SWCLK trước khi kiểm tra các LED này.

## Bài 04 - Nút nhấn đảo trạng thái LED

- PA0: input pull-up nội; nút nhấn nối giữa PA0 và GND.
- PC13: LED tích hợp trên Blue Pill, hoạt động mức thấp.
- LED chỉ đảo trạng thái một lần sau mỗi chu trình nhấn-rồi-nhả.
- Chương trình dùng SysTick 1 ms và chống dội phím 20 ms.

## Nạp chương trình

```bash
st-info --probe
cd bai-01   # hoặc bai-02, bai-03, bai-04
make flash
```

Nếu bài 03 đã chạy và ST-Link không kết nối lại, giữ nút RESET trên kit, chạy
`make flash`, rồi thả RESET khi công cụ bắt đầu nhận dạng chip. Bài 03 phải tắt
SWD/JTAG để sử dụng đủ PA13, PA14 và PA15 theo yêu cầu đề bài.

## Video demo

- Sau khi quay, tải `demo-bai-01.mp4` vào thư mục `bai-01/video-demo/`.
- Sau khi quay, tải `demo-bai-02.mp4` vào thư mục `bai-02/video-demo/`.
- [Thư mục video demo bài 03](./bai-03/video-demo/): tải video
  `demo-bai-03.mp4` hoặc đặt liên kết công khai trong phần mô tả commit.
- [Thư mục video demo bài 04](./bai-04/video-demo/): tải video
  `demo-bai-04.mp4` hoặc đặt liên kết công khai trong phần mô tả commit.

Trước khi nộp, thay thông tin sinh viên và tải video vào đúng thư mục tương ứng.
