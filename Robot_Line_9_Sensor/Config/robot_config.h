#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H
/* PWM values are in 0..1000. Position units: 1000 per eye spacing. */
#define ROBOT_PWM_LIMIT 650
#define ROBOT_DUTY_SCALE 1000
#define CTRL_PERIOD_MS 5U
#define CTRL_MAX_GAP_MS 20U
#define CTRL_START_FRAMES 5U
#define CTRL_START_CENTER 1400
#define CTRL_BASE_PWM 520
#define CTRL_MIN_PWM 100
#define CTRL_MAX_PWM 620
#define CTRL_SPEED_STEP 25
#define CTRL_PWM_RAMP 8
#define CTRL_KP 75
#define CTRL_KD 700 /* acts on position units per millisecond */
#define CTRL_TRIM_DIV 20000
#define CTRL_TRIM_LIMIT 30
#define CTRL_TRIM_WINDOW 700
#define CTRL_D_FILTER_MS 20U
/* Curve response blends in by optical offset; this is not a radius estimate. */
#define CTRL_CURVE_START 700      /* 0.7 eye spacing */
#define CTRL_CURVE_FULL  2300     /* 2.3 eye spacings */
#define CTRL_CURVE_KP    105      /* proportional gain at full blend */
#define CTRL_CURVE_SLOW_EXTRA 4   /* up to 4 extra percentage points of slowdown */
#define CTRL_CURVE_PWM_FLOOR 82   /* relative floor before the approach cap */
#define CTRL_CURVE_APPROACH_PWM 360 /* outer PWM cap at full blend */
/* A sharp turn requires a strong line at the same edge in consecutive frames.
 * A longer streak starts turning before loss. Masks are in ADC channel order. */
#define CTRL_CORNER_EDGE_POS       2400
#define CTRL_CORNER_LEFT_MASK      (1U<<7) /* PA7: outermost enabled left */
#define CTRL_CORNER_RIGHT_MASK     (1U<<0) /* PA0: outermost right */
#define CTRL_CORNER_EDGE_FRAMES    2U
#define CTRL_CORNER_EARLY_FRAMES   4U
#define CTRL_CORNER_CENTER         800
#define CTRL_CORNER_CENTER_FRAMES  3U
#define CTRL_CORNER_PWM            360
#define CTRL_CORNER_TIMEOUT_MS     450U
/* Brief optical dropout between eyes: retain the last steering ratio at low PWM. */
#define CTRL_DROPOUT_MS            35U
#define CTRL_DROPOUT_PWM           280
#define CTRL_DROPOUT_GOOD_FRAMES   2U
#if CTRL_DROPOUT_MS <= CTRL_PERIOD_MS*CTRL_DROPOUT_GOOD_FRAMES || CTRL_DROPOUT_MS > 60 || CTRL_DROPOUT_PWM <= 0 || CTRL_DROPOUT_PWM > CTRL_CURVE_APPROACH_PWM
#error Invalid_dropout_limits
#endif
#if CTRL_CORNER_PWM <= 0 || CTRL_CORNER_PWM > ROBOT_PWM_LIMIT || CTRL_CORNER_EDGE_FRAMES < 2 || CTRL_CORNER_EARLY_FRAMES < CTRL_CORNER_EDGE_FRAMES || CTRL_CORNER_EARLY_FRAMES > 255 || CTRL_CORNER_CENTER_FRAMES < 2 || CTRL_CORNER_TIMEOUT_MS <= CTRL_PERIOD_MS
#error Invalid_corner_settings
#endif
#if CTRL_CURVE_APPROACH_PWM < CTRL_MIN_PWM || CTRL_CURVE_APPROACH_PWM > CTRL_MAX_PWM
#error Invalid_curve_approach_PWM
#endif
#if CTRL_MIN_PWM <= 0 || CTRL_BASE_PWM < CTRL_MIN_PWM || CTRL_BASE_PWM > CTRL_MAX_PWM || CTRL_MAX_PWM > ROBOT_PWM_LIMIT
#error Invalid_base_PWM_limits
#endif
#if CTRL_CURVE_START < 0 || CTRL_CURVE_FULL <= CTRL_CURVE_START || CTRL_CURVE_FULL > 4000
#error Invalid_curve_offset_range
#endif
#if CTRL_CURVE_KP < CTRL_KP || CTRL_CURVE_SLOW_EXTRA < 0 || CTRL_CURVE_SLOW_EXTRA > 40 || CTRL_CURVE_PWM_FLOOR < 1 || CTRL_CURVE_PWM_FLOOR > 100
#error Invalid_curve_gains_or_slowdown
#endif
#define LINE_FLOOR 120
#define LINE_PEAK 240
#define LINE_STRONG 500
#define LINE_WIDE_COUNT 7U
#endif
