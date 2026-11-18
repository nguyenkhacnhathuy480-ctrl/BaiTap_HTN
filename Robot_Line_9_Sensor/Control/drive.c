#include "drive.h"
#include "robot_config.h"
#include <string.h>
static int32_t clamp(int32_t x,int32_t lo,int32_t hi){return x<lo?lo:(x>hi?hi:x);}
static int32_t mag(int32_t x){return x<0?-x:x;}
void Drive_Init(Drive *d){memset(d,0,sizeof(*d));d->base=CTRL_BASE_PWM;}
void Drive_SetSpeed(Drive *d,int32_t base){d->base=clamp(base,CTRL_MIN_PWM,CTRL_MAX_PWM);}
static void reset_steering(Drive *d)
{
    d->have_history=0;d->integral=0;d->d_q8=0;d->throttle=0;
    d->edge_side=0;d->edge_frames=0;d->center_frames=0;
    d->last_pwm.left=0;d->last_pwm.right=0;d->dropout_good=0;
}
WheelPWM Drive_Update(Drive *d,TrackReading line,uint32_t now,uint8_t toggle,uint8_t stop)
{
    WheelPWM out={0,0};
    int32_t e,diff,turn,base,blend,kp,speed_scale,approach_cap;
    uint32_t dt=CTRL_PERIOD_MS;
    int8_t edge=0;
    uint8_t corner=(uint8_t)(d->state==DRIVE_CORNER_LEFT || d->state==DRIVE_CORNER_RIGHT);
    uint8_t dropout=(uint8_t)(d->state==DRIVE_DROPOUT);
    if(line.quality==TRACK_OK && mag(line.position)<=CTRL_START_CENTER){
        if(d->good<CTRL_START_FRAMES)++d->good;
    }else d->good=0;
    if(stop || (toggle && (d->state==DRIVE_ACTIVE || corner || dropout))){
        d->state=DRIVE_IDLE;reset_steering(d);return out;
    }
    if(d->state!=DRIVE_ACTIVE && !corner && !dropout){
        if(!toggle || d->good<CTRL_START_FRAMES)return out;
        d->state=DRIVE_ACTIVE;
    }
    if(d->have_history){
        dt=now-d->last;
        if(!dt || dt>CTRL_MAX_GAP_MS){d->state=DRIVE_TIMING;d->good=0;reset_steering(d);return out;}
    }
    e=clamp(line.position,-4000,4000);
    if(!corner && !dropout && line.quality==TRACK_OK){
        if(e<=-CTRL_CORNER_EDGE_POS && (line.mask&CTRL_CORNER_LEFT_MASK))edge=-1;
        if(e>=CTRL_CORNER_EDGE_POS && (line.mask&CTRL_CORNER_RIGHT_MASK))edge=1;
        if(edge!=d->edge_side || !edge)d->edge_frames=0;
        d->edge_side=edge;
        if(edge && d->edge_frames<CTRL_CORNER_EARLY_FRAMES)++d->edge_frames;
    }
    if(!corner && !dropout && ((line.quality==TRACK_BLANK && d->edge_frames>=CTRL_CORNER_EDGE_FRAMES) ||
                  (line.quality==TRACK_OK && d->edge_frames>=CTRL_CORNER_EARLY_FRAMES))){
        d->state=d->edge_side<0?DRIVE_CORNER_LEFT:DRIVE_CORNER_RIGHT;
        reset_steering(d);d->corner_since=now;corner=1;
    }
    if(!corner && !dropout && line.quality==TRACK_BLANK && d->have_history){
        d->state=DRIVE_DROPOUT;d->dropout_since=now;d->dropout_good=0;dropout=1;
    }
    if(dropout && (line.quality==TRACK_BLANK || line.quality==TRACK_OK)){
        int32_t top=d->last_pwm.left>d->last_pwm.right?d->last_pwm.left:d->last_pwm.right;
        if(now-d->dropout_since>=CTRL_DROPOUT_MS){
            d->state=DRIVE_NO_LINE;d->good=0;reset_steering(d);return out;
        }
        if(line.quality==TRACK_OK)++d->dropout_good;
        else d->dropout_good=0;
        base=clamp(top,0,clamp(d->base,0,CTRL_DROPOUT_PWM));
        if(d->dropout_good<CTRL_DROPOUT_GOOD_FRAMES){
            if(top){out.left=d->last_pwm.left*base/top;out.right=d->last_pwm.right*base/top;}
            d->last=now;return out;
        }
        reset_steering(d);d->state=DRIVE_ACTIVE;d->throttle=base;dt=CTRL_PERIOD_MS;
    }
    if(line.quality!=TRACK_OK && !(corner && line.quality==TRACK_BLANK)){
        switch(line.quality){
        case TRACK_WIDE:d->state=DRIVE_WIDE;break;
        case TRACK_AMBIGUOUS:d->state=DRIVE_AMBIGUOUS;break;
        case TRACK_CAL:d->state=DRIVE_CAL;break;
        default:d->state=DRIVE_NO_LINE;break;
        }
        reset_steering(d);return out;
    }
    if(corner){
        if(now-d->corner_since>=CTRL_CORNER_TIMEOUT_MS){
            d->state=DRIVE_CORNER_TIMEOUT;d->good=0;reset_steering(d);return out;
        }
        if(line.quality==TRACK_OK && mag(line.position)<=CTRL_CORNER_CENTER)++d->center_frames;
        else d->center_frames=0;
        if(d->center_frames<CTRL_CORNER_CENTER_FRAMES){
            base=clamp(d->base,0,CTRL_CORNER_PWM);
            /* Stop turning as soon as the center sees line, then confirm it. */
            if(!d->center_frames){
                if(d->state==DRIVE_CORNER_LEFT)out.right=base;else out.left=base;
            }
            d->last=now;d->have_history=1;return out;
        }
        reset_steering(d);d->state=DRIVE_ACTIVE;dt=CTRL_PERIOD_MS;
    }
    diff=d->have_history?(e-d->prior)*256/(int32_t)dt:0;
    d->d_q8+=(diff-d->d_q8)*(int32_t)dt/(int32_t)(CTRL_D_FILTER_MS+dt);
    d->last=now;d->prior=e;d->have_history=1;
    if(mag(e)>CTRL_TRIM_WINDOW)d->integral=0;
    else if(mag(e)>50 && d->throttle>=d->base/2)
        d->integral=clamp(d->integral+e*(int32_t)dt,-CTRL_TRIM_LIMIT*CTRL_TRIM_DIV,CTRL_TRIM_LIMIT*CTRL_TRIM_DIV);
    d->throttle=clamp(d->throttle+CTRL_PWM_RAMP,0,d->base);
    blend=clamp((mag(e)-CTRL_CURVE_START)*1000/(CTRL_CURVE_FULL-CTRL_CURVE_START),0,1000);
    kp=CTRL_KP+(CTRL_CURVE_KP-CTRL_KP)*blend/1000;
    speed_scale=clamp(1000-mag(e)/10-CTRL_CURVE_SLOW_EXTRA*blend/100,
        CTRL_CURVE_PWM_FLOOR*10,1000);
    base=d->throttle*speed_scale/1000;
    /* Higher straight-line duty must not carry into the turn. Smoothly
       cap the outer wheel toward the independent approach duty. */
    approach_cap=d->throttle-clamp(d->throttle-CTRL_CURVE_APPROACH_PWM,0,d->throttle)*blend/1000;
    if(base>approach_cap)base=approach_cap;
    turn=clamp((kp*e+CTRL_KD*(d->d_q8/256))/1000+d->integral/CTRL_TRIM_DIV,-base,base);
    out.left=base+(turn<0?turn:0);
    out.right=base-(turn>0?turn:0);
    d->last_pwm=out;
    return out;
}
