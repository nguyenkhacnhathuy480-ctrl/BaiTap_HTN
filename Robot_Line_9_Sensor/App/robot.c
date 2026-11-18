#include "robot.h"
#include "robot_config.h"
#include "sensor_map.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "line_sensor.h"
#include "motor_driver.h"
#include "user_io.h"
#include <string.h>

static RobotStatus latest;
static uint8_t stop_requested,adc_ok;

static void fatal(void)
{
    __disable_irq();
    Board_EmergencyStop();
    Buzzer_Set(0);
    for(;;){}
}
void App_AssertFailed(void){fatal();}
void vApplicationMallocFailedHook(void){fatal();}
void vApplicationStackOverflowHook(TaskHandle_t t,char *name){(void)t;(void)name;fatal();}
void HardFault_Handler(void){fatal();}
void MemManage_Handler(void){fatal();}
void BusFault_Handler(void){fatal();}
void UsageFault_Handler(void){fatal();}

void Robot_GetStatus(RobotStatus *out)
{
    taskENTER_CRITICAL();*out=latest;taskEXIT_CRITICAL();
}
void Robot_RequestStop(void)
{
    taskENTER_CRITICAL();stop_requested=1;taskEXIT_CRITICAL();
}

/* Individual counters avoid one bouncing switch delaying another.
   Held-at-boot switches are initialized as pressed without emitting an edge. */
static uint8_t button_edges(uint8_t raw,uint8_t *stable,uint8_t age[3])
{
    unsigned i;
    uint8_t pressed=0;
    for(i=0;i<3;i++){
        uint8_t bit=(uint8_t)(1U<<i);
        if(!((raw^*stable)&bit))age[i]=0;
        else if(++age[i]>=4U){
            *stable^=bit;pressed|=(uint8_t)(*stable&bit);age[i]=0;
        }
    }
    return pressed;
}

static void control_task(void *arg)
{
    Tracker tracker;
    Drive drive;
    RobotStatus s={0};
    TickType_t release=xTaskGetTickCount();
    uint32_t now,last=0,beep_start=0;
    uint8_t seen=0,beeping=0,stable=Buttons_Read(),age[3]={0,0,0};
    (void)arg;
    Tracker_Init(&tracker);Drive_Init(&drive);
    for(;;){
        uint8_t toggle,stop,edges;
        s.buttons=Buttons_Read();
        edges=button_edges(s.buttons,&stable,age);
        toggle=(uint8_t)(edges&1U);
        /* A Stop can take effect on raw input; arming needs debounced PB5. */
        stop=(uint8_t)((s.buttons&6U)==6U);
        if(!stop && (stable&6U)!=6U){
            if(edges&2U)Drive_SetSpeed(&drive,drive.base+CTRL_SPEED_STEP);
            if(edges&4U)Drive_SetSpeed(&drive,drive.base-CTRL_SPEED_STEP);
        }
        if(adc_ok && !Sensors_Read(s.raw))adc_ok=0;
        s.track=Tracker_Read(&tracker,s.raw);
        now=(uint32_t)xTaskGetTickCount();
        taskENTER_CRITICAL();
        if(stop_requested){stop=1;stop_requested=0;}
        taskEXIT_CRITICAL();
        if(!BOARD_SENSOR_ORDER_CONFIRMED)toggle=0;
        if(seen && (uint32_t)(now-last)>CTRL_MAX_GAP_MS){
            s.pwm=Drive_Update(&drive,s.track,now,0,1);
            drive.state=DRIVE_TIMING;drive.good=0;
        }else s.pwm=Drive_Update(&drive,s.track,now,toggle,stop);
        seen=1;last=now;
        if(!adc_ok || !tracker.calibrated){
            s.pwm=Drive_Update(&drive,s.track,now,0,1);
            drive.state=!adc_ok?DRIVE_ADC:DRIVE_CAL;
        }
       if(s.pwm.left || s.pwm.right)Motors_Set(s.pwm.right,s.pwm.left);
        else Motors_Stop();
        if(edges){Buzzer_Set(1);beeping=1;beep_start=now;}
        if(beeping && (uint32_t)(now-beep_start)>=55U){beeping=0;Buzzer_Set(0);}
        s.ticks=now;s.state=drive.state;s.base=drive.base;
        s.ready=(uint8_t)(adc_ok && tracker.calibrated && BOARD_SENSOR_ORDER_CONFIRMED &&
            drive.good>=CTRL_START_FRAMES);
        taskENTER_CRITICAL();latest=s;taskEXIT_CRITICAL();
        if((TickType_t)(xTaskGetTickCount()-release)>=pdMS_TO_TICKS(CTRL_PERIOD_MS))
            release=xTaskGetTickCount();
        vTaskDelayUntil(&release,pdMS_TO_TICKS(CTRL_PERIOD_MS));
    }
}
void Robot_Start(uint8_t ok)
{
    adc_ok=ok;
    if(xTaskCreate(control_task,"Drive",320,0,3,0)!=pdPASS ||
       xTaskCreate(Robot_Monitor,"Monitor",512,0,1,0)!=pdPASS)fatal();
    vTaskStartScheduler();
    fatal();
}
