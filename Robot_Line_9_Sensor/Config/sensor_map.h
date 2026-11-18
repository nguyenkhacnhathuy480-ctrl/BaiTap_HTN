#ifndef SENSOR_MAP_H
#define SENSOR_MAP_H
/* User-confirmed left -> right, looking in the forward travel direction:
 * PB0, PA7, PA6, PA5, PA4, PA3, PA2, PA1, PA0.
 * ADC buffer order remains PA0..PA7, PB0 (channels IN0..IN8).
 * Negative position = left; positive position = right. */
#define BOARD_SENSOR_ORDER_CONFIRMED 1U
/* ADC index 8 (PB0, far left) disabled at user's request.
 * Keep sampling all 9 ADC channels for diagnostics. Restore 0x1FFU after repair. */
#define BOARD_SENSOR_ACTIVE_MASK 0x0FFU
#if BOARD_SENSOR_ACTIVE_MASK == 0 || (BOARD_SENSOR_ACTIVE_MASK & ~0x1FFU)
#error Invalid_sensor_active_mask
#endif
#define BOARD_SENSOR_POSITION_INIT {4000,3000,2000,1000,0,-1000,-2000,-3000,-4000}
#endif
