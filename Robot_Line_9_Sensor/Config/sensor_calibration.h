#ifndef SENSOR_CALIBRATION_H
#define SENSOR_CALIBRATION_H
/* UART measurements 2026-11-18, battery powered, ADC order PA0..PA7, PB0.
 * Only valid for the measured mounting height, surface and power conditions.
 * PB0 remains disabled in sensor_map.h. Manual capture overrides this RAM copy. */
#define BOARD_SENSOR_PRESET_ENABLED 1U
/* Production driving build: calibration cannot be erased by buttons/UART.
 * Set to 1 only for deliberate recalibration, then rebuild. */
#define BOARD_SENSOR_MANUAL_CAL_ENABLED 0U
#define BOARD_SENSOR_WHITE_INIT {143,147,169,164,207,168,158,137,124}
#define BOARD_SENSOR_BLACK_INIT {1754,1879,2450,2866,3908,3270,2619,1570,223}
#endif
