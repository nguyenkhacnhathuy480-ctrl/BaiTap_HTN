# Third-party components

- FreeRTOS Kernel V10.3.1: `tasks.c`, `queue.c`, `list.c`, public headers, RVDS ARM_CM3 port, heap_4. Taken from the locally available STM32CubeF1 FreeRTOS distribution. License notices are retained in the original source/header files (MIT license).
- ARM CMSIS Core and ST STM32F1 device headers: retained from the existing vendor distribution, including their license files. Used for register names, core instructions and interrupt definitions, not HAL application code.
- `Startup/startup_stm32f10x_md.s`: ST startup supplied in Keil STM32F1xx_DFP 2.4.1, ARM assembler syntax. Original license header retained.

Application, board support, control logic, project scatter file and tests are newly implemented for this project. Vendor files are kept separate; their upstream licenses continue to apply.
