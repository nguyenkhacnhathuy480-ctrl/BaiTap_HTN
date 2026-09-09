# STM32F103 Bare-Metal - Bài tập 03 và 04

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
└── SUBMISSION_CHECKLIST.md
```

## Công cụ đã sử dụng

- GNU Tools for STM32 14.3.1
- stlink-tools 1.8.0
- GNU Make trên Ubuntu

## Build

Bài 03:

```bash
cd bai-03
make clean
make
```

Bài 04:

```bash
cd bai-04
make clean
make
```

Mỗi project tạo tệp `.elf`, `.bin` và `.map` trong thư mục `build/`.

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
cd bai-03   # hoặc bai-04
make flash
```

Nếu bài 03 đã chạy và ST-Link không kết nối lại, giữ nút RESET trên kit, chạy
`make flash`, rồi thả RESET khi công cụ bắt đầu nhận dạng chip. Bài 03 phải tắt
SWD/JTAG để sử dụng đủ PA13, PA14 và PA15 theo yêu cầu đề bài.

## Video demo

- [Thư mục video demo bài 03](./bai-03/video-demo/): tải video
  `demo-bai-03.mp4` hoặc đặt liên kết công khai trong phần mô tả commit.
- [Thư mục video demo bài 04](./bai-04/video-demo/): tải video
  `demo-bai-04.mp4` hoặc đặt liên kết công khai trong phần mô tả commit.

Trước khi nộp, thay thông tin sinh viên và tải video vào đúng thư mục tương ứng.
