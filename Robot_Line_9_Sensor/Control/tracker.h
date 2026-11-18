#ifndef TRACKER_H
#define TRACKER_H
#include <stdint.h>
typedef enum { TRACK_OK, TRACK_BLANK, TRACK_WIDE, TRACK_AMBIGUOUS, TRACK_CAL } TrackQuality;
typedef struct { int32_t position; uint16_t mask,peak; TrackQuality quality; } TrackReading;
typedef struct {
    int32_t filter[9];
    int32_t contrast[9];
    uint8_t initialized,calibrated;
} Tracker;
void Tracker_Init(Tracker *tracker);
TrackReading Tracker_Read(Tracker *tracker,const uint16_t raw[9]);
#endif
