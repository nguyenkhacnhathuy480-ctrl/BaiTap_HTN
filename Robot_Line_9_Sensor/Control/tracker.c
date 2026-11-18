#include "tracker.h"
#include "robot_config.h"
#include "sensor_map.h"
#include "sensor_calibration.h"
#include <string.h>
static const uint16_t white[9]=BOARD_SENSOR_WHITE_INIT,black[9]=BOARD_SENSOR_BLACK_INIT;
static const int16_t location[9]=BOARD_SENSOR_POSITION_INIT;
void Tracker_Init(Tracker *t)
{
    unsigned i;
    memset(t,0,sizeof(*t));t->calibrated=1;
    for(i=0;i<9;i++) {
        int32_t span=(int32_t)black[i]-white[i];
        t->contrast[i]=span;
        if ((BOARD_SENSOR_ACTIVE_MASK & (1U<<i)) &&
            (white[i]>4095 || black[i]>4095 || (span>-120 && span<120))) t->calibrated=0;
    }
}
TrackReading Tracker_Read(Tracker *t,const uint16_t raw[9])
{
    TrackReading r={0,0,0,TRACK_BLANK};
    int32_t level[9]={0},weighted=0,total=0;
    unsigned i,dominant=0,first,last,strong=0;
    if(!t->calibrated) { r.quality=TRACK_CAL;return r; }
    for(i=0;i<9;i++) {
        int32_t target,v;
        if(!(BOARD_SENSOR_ACTIVE_MASK & (1U<<i))) continue;
        target=((int32_t)raw[i]-white[i])*1000/t->contrast[i];
        if(target<0)target=0;
        if(target>1000)target=1000;
        target*=256;
        if(!t->initialized)t->filter[i]=target;
        else t->filter[i]+=(target-t->filter[i])/2;
        v=level[i]=(t->filter[i]+128)/256;
        if(v>=LINE_STRONG){r.mask|=(uint16_t)(1U<<i);++strong;}
        if(v>r.peak){r.peak=(uint16_t)v;dominant=i;}
    }
    t->initialized=1;
    if(strong>=LINE_WIDE_COUNT){r.quality=TRACK_WIDE;return r;}
    if(r.peak<LINE_PEAK)return r;
    first=last=dominant;
    while(first && level[first-1]>=LINE_FLOOR)--first;
    while(last<8 && level[last+1]>=LINE_FLOOR)++last;
    for(i=0;i<9;i++) {
        if(i>=first && i<=last){
            /* Eliminate floor from the centroid weight instead of summing white noise. */
            int32_t weight=level[i]-LINE_FLOOR;
            total+=weight;weighted+=weight*location[i];
        }else if(level[i]>=LINE_PEAK){r.quality=TRACK_AMBIGUOUS;return r;}
    }
    r.position=weighted/total;r.quality=TRACK_OK;return r;
}
