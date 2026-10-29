# Robot do line 9 mat — STM32F103C8T6 va TB6612FNG

Firmware bare-metal dung thanh ghi truc tiep cho robot hai dong co, 9 cam bien
line so va driver TB6612FNG. Ma duoc tach theo vai tro de thay doi chan noi,
PID hoac driver ma khong lam roi vong dieu khien.

Project co hai bien the: bare-metal de kiem tra tung ngoai vi va FreeRTOS de
chay cac task doc cam bien, dieu khien motor va LED trang thai. Hai bien the
dung chung driver cam bien, motor va boi canh phan cung.

## Cau truc

    Robot_Line_9_Sensor/
    ├── Core/
    │   ├── Inc/
    │   │   ├── board_config.h    # Chan noi va hang so can chinh
    │   │   ├── line_sensor.h
    │   │   ├── motor_tb6612.h
    │   │   ├── line_follower.h
    │   │   └── platform.h
    │   └── Src/
    │       ├── main.c            # Vong dieu khien 1 ms
    │       ├── platform.c        # Clock 72 MHz, SysTick, LED trang thai
    │       ├── line_sensor.c     # Doc va tinh vi tri tu 9 mat
    │       ├── motor_tb6612.c    # PWM va chieu quay hai dong co
    │       └── line_follower.c   # PID va xu ly mat line
    ├── FreeRTOS/
    │   ├── Core/Inc/FreeRTOSConfig.h
    │   ├── Core/Src/main_freertos.c
    │   └── Makefile
    ├── Middleware/FreeRTOS/
    ├── Startup/
    └── Makefile

## So do chan mac dinh

| Chuc nang | STM32F103C8T6 | Ket noi |
|---|---|---|
| Cam bien S0…S7 | PA0…PA7 | OUT cua 8 mat tu trai sang phai |
| Cam bien S8 | PB0 | OUT cua mat ngoai cung ben phai |
| PWMA | PA8 / TIM1_CH1 | TB6612 PWMA |
| PWMB | PA9 / TIM1_CH2 | TB6612 PWMB |
| AIN1, AIN2 | PB12, PB13 | TB6612 AIN1, AIN2 |
| BIN1, BIN2 | PB14, PB15 | TB6612 BIN1, BIN2 |
| STBY | PB11 | TB6612 STBY |
| LED trang thai | PC13 | LED tich hop Blue Pill |

Noi chung GND giua STM32, mach cam bien, TB6612 va nguon dong co. VCC logic
cua TB6612 noi 3.3 V. VM noi nguon dong co theo dien ap motor. Khong cap nguon
motor tu chan 3.3 V cua Blue Pill.

Chan OUT cua moi cam bien phai co muc cao toi da 3.3 V. Neu mach cam bien dang
cap 5 V va OUT cung la 5 V, dung mach chia ap hoac level shifter truoc khi noi
vao STM32.

Mac dinh cam bien active-low: gap vach den se xuat muc 0. Neu mach cam bien
xuat muc 1 khi gap vach den, doi LINE_SENSOR_ACTIVE_LOW tu 1U thanh 0U trong
Core/Inc/board_config.h.

## Thuat toan

Chuong trinh doc 9 cam bien moi 1 ms. Cac mat nhan trong so tu -4000 den
+4000; trung binh trong so cua nhung mat dang thay vach la sai so vi tri.
PID tao hieu chinh cho hai motor:

    left  = base_speed + correction
    right = base_speed - correction

Khi ca 9 mat khong thay line, xe quay ve phia ma no nhin thay line lan cuoi.
Thong so chinh xe nam trong Core/Inc/board_config.h:

    LINE_PID_KP            170
    LINE_PID_KI            2
    LINE_PID_KD            280
    LINE_BASE_SPEED        520
    LINE_LOST_TURN_SPEED   320

Bat dau voi toc do thap, tang LINE_BASE_SPEED, sau do chinh KP, KD va cuoi
cung moi tang rat it KI.

## Build va nap tren Ubuntu

    cd ~/projects/BaiTap_HTN/Robot_Line_9_Sensor
    make clean
    make
    make flash

CMSIS phu hop STM32F103C8T6 da nam trong Drivers/CMSIS, vi vay project co the
di chuyen hoac sao chep sang noi khac ma van build duoc.

## Build va nap ban FreeRTOS

Ban FreeRTOS co ba task:

- SensorTask: doc 9 mat moi 1 ms va gui vi tri line qua queue.
- ControlTask: nhan mau, chay PID va cap nhat hai kenh PWM.
- StatusTask: dao LED PC13 moi 250 ms de bao scheduler dang chay.

    cd ~/projects/BaiTap_HTN/Robot_Line_9_Sensor/FreeRTOS
    make clean
    make
    make flash

FreeRTOSConfig.h dat tick 1 kHz, heap 6 KB, uu tien Control cao hon Sensor
va dung port GCC ARM Cortex-M3.

## Dieu chinh khi doi day

Chi sua Core/Inc/board_config.h khi doi chan cam bien, chan TB6612, chieu
logic cam bien hoac hang so PID. Neu motor trai/phai chay nguoc chieu mong
muon, dao hai day motor hoac doi cap AIN/BIN tuong ung trong phan cau hinh.
