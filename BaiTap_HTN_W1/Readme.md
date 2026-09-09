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
│   ├── README.md
│   └── video-demo/
├── bai-04/
│   ├── main.c
│   ├── startup.s
│   ├── linker.ld
│   ├── Makefile
│   ├── README.md
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

- [Thư mục video demo bài 03](./bai-03/video-demo/)
- [Thư mục video demo bài 04](./bai-04/video-demo/)

Trước khi nộp, thay thông tin sinh viên và tải video vào đúng thư mục tương ứng.
