#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "drive.h"
#include "robot_config.h"
#include "sensor_calibration.h"

int main(void)
{
    static const uint16_t white[9]=BOARD_SENSOR_WHITE_INIT,black[9]=BOARD_SENSOR_BLACK_INIT;
    uint16_t raw[9];
    TrackReading line;
    Tracker tracker;
    Drive drive;
    WheelPWM pwm;
    unsigned i,j;
    memcpy(raw,white,sizeof(raw));raw[8]=4095;
    Tracker_Init(&tracker);line=Tracker_Read(&tracker,raw);
    assert(line.quality==TRACK_BLANK && !line.mask);
    for(j=0;j<8;j++){
        memcpy(raw,white,sizeof(raw));raw[j]=black[j];raw[8]=4095;
        Tracker_Init(&tracker);line=Tracker_Read(&tracker,raw);
        assert(line.quality==TRACK_OK && line.position==4000-(int32_t)j*1000);
    }
    raw[0]=black[0];Tracker_Init(&tracker);
    assert(Tracker_Read(&tracker,raw).quality==TRACK_AMBIGUOUS);
    Tracker_Init(&tracker);assert(Tracker_Read(&tracker,black).quality==TRACK_WIDE);

    memcpy(raw,white,sizeof(raw));raw[4]=black[4];
    Tracker_Init(&tracker);line=Tracker_Read(&tracker,raw);
    Drive_Init(&drive);
    pwm=Drive_Update(&drive,line,0,1,0);
    assert(drive.state==DRIVE_IDLE && !pwm.left && !pwm.right);
    for(i=1;i<=CTRL_START_FRAMES;i++){
        pwm=Drive_Update(&drive,line,i*5,0,0);assert(!pwm.left && !pwm.right);
    }
    pwm=Drive_Update(&drive,line,50,1,0);
    assert(drive.state==DRIVE_ACTIVE && pwm.left==CTRL_PWM_RAMP && pwm.right==pwm.left);
    line.position=1000;pwm=Drive_Update(&drive,line,55,0,0);assert(pwm.left>pwm.right);
    line.position=-1000;pwm=Drive_Update(&drive,line,60,0,0);assert(pwm.right>pwm.left);
    line.quality=TRACK_BLANK;pwm=Drive_Update(&drive,line,65,0,0);
    assert(drive.state==DRIVE_DROPOUT && pwm.right>pwm.left);
    for(i=5;i<=CTRL_DROPOUT_MS;i+=5)pwm=Drive_Update(&drive,line,65+i,0,0);
    assert(drive.state==DRIVE_NO_LINE && !pwm.left && !pwm.right);
    line.quality=TRACK_OK;line.position=0;
    for(i=0;i<12;i++) {
        pwm=Drive_Update(&drive,line,105+i*5,0,0);assert(drive.state==DRIVE_NO_LINE && !pwm.left && !pwm.right);
    }
    pwm=Drive_Update(&drive,line,170,1,1);assert(drive.state==DRIVE_IDLE && !pwm.left && !pwm.right);
    (void)Drive_Update(&drive,line,175,1,0);
    pwm=Drive_Update(&drive,line,200,0,0);assert(drive.state==DRIVE_TIMING && !pwm.left && !pwm.right);
    for(j=0;j<2;j++){
        Drive_Init(&drive);line.position=0;
        for(i=0;i<CTRL_START_FRAMES;i++)(void)Drive_Update(&drive,line,5*i,0,0);
        (void)Drive_Update(&drive,line,5*CTRL_START_FRAMES,1,0);
        line.position=j?400:-400;
        for(i=1;i<=2000;i++){
            pwm=Drive_Update(&drive,line,5*(CTRL_START_FRAMES+i),0,0);
            assert(pwm.left>=0 && pwm.right>=0 && pwm.left<=drive.base && pwm.right<=drive.base);
        }
        assert(drive.integral==(j?1:-1)*CTRL_TRIM_LIMIT*CTRL_TRIM_DIV);
        assert(j?pwm.left>pwm.right:pwm.right>pwm.left);
    }
    {
        static const int32_t offsets[]={0,400,700,1200,1600,2300,4000};
        int32_t previous_outer=CTRL_BASE_PWM;
        for(i=0;i<sizeof(offsets)/sizeof(offsets[0]);i++){
            WheelPWM mirrored;
            Drive_Init(&drive);drive.state=DRIVE_ACTIVE;drive.throttle=drive.base;
            line.position=offsets[i];
            pwm=Drive_Update(&drive,line,5,0,0);
            Drive_Init(&drive);drive.state=DRIVE_ACTIVE;drive.throttle=drive.base;
            line.position=-offsets[i];
            mirrored=Drive_Update(&drive,line,5,0,0);
            assert(pwm.left==mirrored.right && pwm.right==mirrored.left);
            assert(pwm.left<=previous_outer);
            if(offsets[i]>=CTRL_CURVE_FULL)assert(pwm.left<=CTRL_CURVE_APPROACH_PWM);
            assert(pwm.right>=0 && pwm.right<=pwm.left);
            if(i==0)assert(pwm.left==CTRL_BASE_PWM && pwm.left==pwm.right);
            if(offsets[i]==1600)assert(pwm.left-pwm.right>CTRL_KP*offsets[i]/1000);
            previous_outer=pwm.left;
        }
    }
    for(j=0;j<2;j++){
        TrackReading center={0,16,900,TRACK_OK};
        TrackReading edge={j?4000:-3000,j?CTRL_CORNER_RIGHT_MASK:CTRL_CORNER_LEFT_MASK,900,TRACK_OK};
        TrackReading blank={0,0,0,TRACK_BLANK};
        TrackReading wide={0,255,1000,TRACK_WIDE};
        unsigned mode;
        Drive_Init(&drive);drive.state=DRIVE_ACTIVE;drive.throttle=drive.base;
        for(i=0;i<CTRL_CORNER_EARLY_FRAMES;i++){
            pwm=Drive_Update(&drive,edge,CTRL_PERIOD_MS*i,0,0);
            if(i+1<CTRL_CORNER_EARLY_FRAMES)assert(drive.state==DRIVE_ACTIVE);
        }
        assert(drive.state==(j?DRIVE_CORNER_RIGHT:DRIVE_CORNER_LEFT));
        assert(j?(pwm.left==CTRL_CORNER_PWM && !pwm.right):(!pwm.left && pwm.right==CTRL_CORNER_PWM));
        for(mode=0;mode<6;mode++){
            uint32_t now=0,turn_start;
            Drive_Init(&drive);
            for(i=0;i<CTRL_START_FRAMES;i++,now+=CTRL_PERIOD_MS)
                (void)Drive_Update(&drive,center,now,0,0);
            (void)Drive_Update(&drive,center,now,1,0);
            for(i=0;i<(mode==5?1:CTRL_CORNER_EDGE_FRAMES);i++){
                now+=CTRL_PERIOD_MS;(void)Drive_Update(&drive,edge,now,0,0);
            }
            now+=CTRL_PERIOD_MS;turn_start=now;
            pwm=Drive_Update(&drive,blank,now,0,0);
            if(mode==5){
                assert(drive.state==DRIVE_DROPOUT);continue;
            }
            assert(drive.state==(j?DRIVE_CORNER_RIGHT:DRIVE_CORNER_LEFT));
            assert(j?(pwm.left==CTRL_CORNER_PWM && !pwm.right):(!pwm.left && pwm.right==CTRL_CORNER_PWM));
            if(mode==0){
                for(i=1;i<CTRL_CORNER_CENTER_FRAMES;i++){
                    now+=CTRL_PERIOD_MS;pwm=Drive_Update(&drive,center,now,0,0);
                    assert(!pwm.left && !pwm.right);
                }
                pwm=Drive_Update(&drive,center,now+CTRL_PERIOD_MS,0,0);
                assert(drive.state==DRIVE_ACTIVE && pwm.left==CTRL_PWM_RAMP && pwm.right==pwm.left);
            }else{
                switch(mode){
                case 1:
                    do{now+=CTRL_PERIOD_MS;pwm=Drive_Update(&drive,blank,now,0,0);}
                    while(now-turn_start<CTRL_CORNER_TIMEOUT_MS);
                    assert(drive.state==DRIVE_CORNER_TIMEOUT);
                    break;
                case 2:
                    pwm=Drive_Update(&drive,blank,now+CTRL_PERIOD_MS,1,0);assert(drive.state==DRIVE_IDLE);break;
                case 3:
                    pwm=Drive_Update(&drive,wide,now+CTRL_PERIOD_MS,0,0);assert(drive.state==DRIVE_WIDE);break;
                case 4:
                    pwm=Drive_Update(&drive,center,now+CTRL_MAX_GAP_MS+1,0,0);assert(drive.state==DRIVE_TIMING);break;
                }
                assert(!pwm.left && !pwm.right);
                pwm=Drive_Update(&drive,center,now+CTRL_MAX_GAP_MS+10,0,0);
                assert(!pwm.left && !pwm.right);
            }
        }
    }
    {
        TrackReading pa1={3000,2,900,TRACK_OK};
        TrackReading blank={0,0,0,TRACK_BLANK};
        TrackReading wide={0,255,1000,TRACK_WIDE};
        Drive_Init(&drive);drive.state=DRIVE_ACTIVE;drive.throttle=drive.base;
        (void)Drive_Update(&drive,pa1,0,0,0);
        pwm=Drive_Update(&drive,blank,5,0,0);
        assert(drive.state==DRIVE_DROPOUT && pwm.left<=CTRL_DROPOUT_PWM && pwm.left>pwm.right);
        (void)Drive_Update(&drive,pa1,10,0,0);
        assert(drive.state==DRIVE_DROPOUT);
        (void)Drive_Update(&drive,blank,15,0,0);
        (void)Drive_Update(&drive,pa1,20,0,0);
        assert(drive.state==DRIVE_DROPOUT);
        pwm=Drive_Update(&drive,pa1,25,0,0);
        assert(drive.state==DRIVE_ACTIVE && pwm.left>pwm.right);
        (void)Drive_Update(&drive,blank,30,0,0);
        pwm=Drive_Update(&drive,wide,35,0,0);
        assert(drive.state==DRIVE_WIDE && !pwm.left && !pwm.right);
    }
    puts("PASS: calibration, arcs, 90-degree recovery, dropout confirmation and timeout, safety stops.");
    return 0;
}
