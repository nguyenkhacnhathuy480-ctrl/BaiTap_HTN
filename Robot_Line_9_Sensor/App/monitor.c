#include "robot.h"
#include "board.h"
#include "oled.h"
#include "uart_debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

static const char *state_name(DriveState s)
{
    switch(s){
    case DRIVE_ACTIVE:return "RUN";
    case DRIVE_NO_LINE:return "LOST";
    case DRIVE_WIDE:return "WIDE";
    case DRIVE_AMBIGUOUS:return "AMBIG";
    case DRIVE_TIMING:return "TIMING";
    case DRIVE_ADC:return "ADC ERR";
    case DRIVE_CAL:return "CAL ERR";
    case DRIVE_CORNER_LEFT:return "90 LEFT";
    case DRIVE_CORNER_RIGHT:return "90 RIGHT";
    case DRIVE_CORNER_TIMEOUT:return "90 TIMEOUT";
    case DRIVE_DROPOUT:return "GAP";
    default:return "STOP";
    }
}
static void telemetry(const RobotStatus *s)
{
    char out[224];
    (void)snprintf(out,sizeof(out),"T=%lu STATE=%s READY=%u Q=%u E=%ld MASK=%03X PEAK=%u L=%ld R=%ld BASE=%ld BTN=%u ADC=%u,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
        (unsigned long)s->ticks,state_name(s->state),(unsigned)s->ready,(unsigned)s->track.quality,
        (long)s->track.position,(unsigned)s->track.mask,(unsigned)s->track.peak,
        (long)s->pwm.left,(long)s->pwm.right,(long)s->base,(unsigned)s->buttons,
        (unsigned)s->raw[0],(unsigned)s->raw[1],(unsigned)s->raw[2],(unsigned)s->raw[3],
        (unsigned)s->raw[4],(unsigned)s->raw[5],(unsigned)s->raw[6],(unsigned)s->raw[7],(unsigned)s->raw[8]);
    (void)Uart_Write(out);
}
static void paint(const RobotStatus *s)
{
    char b[32];
    unsigned i;
    (void)snprintf(b,sizeof(b),"ROBOT %s",state_name(s->state));OLED_Text(0,b);
    for(i=0;i<3;i++){
        (void)snprintf(b,sizeof(b),"ADC %u:%4u %4u %4u",i*3,(unsigned)s->raw[i*3],
            (unsigned)s->raw[i*3+1],(unsigned)s->raw[i*3+2]);OLED_Text((uint8_t)(i+1),b);
    }
    (void)snprintf(b,sizeof(b),"E:%ld Q:%u M:%03X",(long)s->track.position,
        (unsigned)s->track.quality,(unsigned)s->track.mask);OLED_Text(4,b);
    (void)snprintf(b,sizeof(b),"L:%ld R:%ld B:%ld",(long)s->pwm.left,(long)s->pwm.right,(long)s->base);OLED_Text(5,b);
    OLED_Text(6,s->ready?"READY PB5 START/STOP":"CENTER ON BLACK LINE");
    OLED_Text(7,"PB8+ PB9- BOTH STOP");
}
void Robot_Monitor(void *arg)
{
    RobotStatus s;
    uint8_t online=0,page=0;
    uint32_t now,last_log=0,last_try=0,last_led=0;
    unsigned i;
    (void)arg;
    (void)Uart_Write("ROBOT KEIL FINAL: PB5 toggle, PB8/PB9 speed. UART a=status, x=stop.\r\n");
    for(;;){
        now=(uint32_t)xTaskGetTickCount();
        Robot_GetStatus(&s);
        for(i=0;i<8;i++){
            int c=Uart_Read();
            if(c<0)break;
            if(c=='x')Robot_RequestStop();
            if(c=='a')telemetry(&s);
            if(c=='?')(void)Uart_Write("a=ADC/state x=stop. Start only via PB5.\r\n");
        }
        if(now-last_log>=100U){last_log=now;telemetry(&s);}
        if(now-last_led>=250U){last_led=now;Board_LedToggle();}
        if(!online && (!last_try || now-last_try>=2000U)){
            last_try=now?now:1U;online=OLED_Init();page=0;
        }
        if(online){
            if(page==0)paint(&s);
            if(!OLED_FlushPage(page))online=0;
            page=(uint8_t)((page+1U)%8U);
        }
        vTaskDelay(pdMS_TO_TICKS(10U));
    }
}
