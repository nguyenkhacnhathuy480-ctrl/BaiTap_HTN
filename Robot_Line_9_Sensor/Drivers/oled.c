/* DM-OLED096-636 (SSD1315) and verified SSD1306 substitutes, 128x64.
 * I2C callbacks use PB6/PB7 open drain. A bus ACK does not identify the IC. */
#include "oled.h"
#include "board.h"
#include <string.h>
static uint8_t pixels[1024];
static I2CSoftBus bus;
uint8_t oled_address;
volatile OLEDDiagnostics g_oled;
static void sda(uint8_t high) { Board_Write(GPIOB,7U,high); }
static void scl(uint8_t high) { Board_Write(GPIOB,6U,high); }
static uint8_t read_scl(void) { return (GPIOB->IDR & (1U<<6U))!=0U; }
static uint8_t read_sda(void) { return (GPIOB->IDR & (1U<<7U))!=0U; }
static void snapshot(void)
{
    g_oled.error=bus.error;g_oled.scl=read_scl();g_oled.sda=read_sda();
    g_oled.address=oled_address;g_oled.data_index=bus.data_index;
    g_oled.transactions=bus.writes;g_oled.failures=bus.failures;
}
static uint8_t send(uint8_t control,const uint8_t *data,uint16_t length)
{
    uint8_t ok=I2CSoft_Write(&bus,oled_address,control,data,length);
    snapshot();
    if (!ok) g_oled.stage=OLED_STAGE_FAILED;
    return ok;
}
uint8_t OLED_Init(void)
{
    static const I2CSoftIO io={scl,sda,read_scl,read_sda,Board_DelayUs};
#if BOARD_OLED_CONTROLLER == 1315
    /* DisplayModule module datasheet example, page 13/14. */
    static const uint8_t init[]={0xAE,0x00,0x10,0x40,0xB0,0x81,0xB0,
        0xA1,0xA4,0xA6,0xA8,0x3F,0xC8,0xD3,0x00,0xD5,0x90,
        0xD9,0x22,0xDA,0x12,0xDB,0x30,0x2E,0x20,0x02};
#elif BOARD_OLED_CONTROLLER == 1306
    static const uint8_t init[]={0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,0x40,
        0x20,0x02,0xA1,0xC8,0xDA,0x12,0x81,0x7F,0xD9,0xF1,0xDB,0x40,0x2E,0xA4,0xA6};
#else
#error Unsupported_OLED_controller
#endif
    static const uint8_t pump[]={0x8D,0x14},on[]={0xAF};
    uint32_t i;
    uint8_t first=BOARD_OLED_ADDRESS_FIRST,second=first==0x3CU?0x3DU:0x3CU;
    ++g_oled.attempts;g_oled.stage=OLED_STAGE_BUS;
    g_oled.ack_3c=0U;g_oled.ack_3d=0U;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    sda(1U);scl(1U);
    GPIOB->CRL=(GPIOB->CRL & ~(255U<<24U))|(0x66U<<24U);
    I2CSoft_Init(&bus,&io);
    /* Four-pin header has no MCU-controlled RES. Allow module RC/POR to settle.
     * This delay cannot repair a missing reset circuit or supply fault. */
    for (i=0U;i<250U;++i) Board_DelayUs(1000U);
    if (!I2CSoft_Recover(&bus)) { snapshot();g_oled.stage=OLED_STAGE_FAILED;return 0U; }
    g_oled.stage=OLED_STAGE_PROBE;
    oled_address=first;
    if (!I2CSoft_Probe(&bus,first)) {
        oled_address=second;
        if (!I2CSoft_Probe(&bus,second)) {
            snapshot();g_oled.stage=OLED_STAGE_FAILED;return 0U;
        }
    }
    if (oled_address==0x3CU) g_oled.ack_3c=1U;
    else g_oled.ack_3d=1U;
    snapshot();g_oled.stage=OLED_STAGE_INIT;
    if (!send(0U,init,(uint16_t)sizeof(init))) return 0U;
    memset(pixels,0,sizeof(pixels));g_oled.stage=OLED_STAGE_CLEAR;
    for (i=0U;i<8U;++i) if (!OLED_FlushPage((uint8_t)i)) return 0U;
    g_oled.stage=OLED_STAGE_ON;
    if (!send(0U,pump,2U)) return 0U;
    for (i=0U;i<100U;++i) Board_DelayUs(1000U);
    if (!send(0U,on,1U)) return 0U;
    g_oled.stage=OLED_STAGE_READY;return 1U;
}
uint8_t OLED_AllPixels(uint8_t on)
{
    uint8_t command=on?0xA5U:0xA4U;
    return send(0U,&command,1U);
}
/* Original compact 5x7 glyphs; only upper-case text is used by the UI. */
static void glyph(char c,uint8_t out[5])
{
    static const uint8_t digits[10][5]={
        {62,81,73,69,62},{0,66,127,64,0},{66,97,81,73,70},{33,65,69,75,49},{24,20,18,127,16},
        {39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},{54,73,73,73,54},{6,73,73,41,30}};
    static const uint8_t letters[26][5]={
        {126,17,17,17,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},
        {127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},
        {0,65,127,65,0},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},
        {127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},
        {62,65,81,33,94},{127,9,25,41,70},{70,73,73,73,49},{1,1,127,1,1},
        {63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},
        {7,8,112,8,7},{97,81,73,69,67}};
    memset(out,0,5U);
    if (c>='0' && c<='9') memcpy(out,digits[c-'0'],5U);
    else if (c>='A' && c<='Z') memcpy(out,letters[c-'A'],5U);
    else if (c=='-') memset(out,8,5U);
    else if (c==':') { out[1]=54U; out[2]=54U; }
    else if (c=='/') { out[0]=32U;out[1]=16U;out[2]=8U;out[3]=4U;out[4]=2U; }
    else if (c=='.') out[2]=64U;
}
void OLED_Text(uint8_t row,const char *text)
{
    uint32_t column=0U;
    if (row>=8U) return;
    memset(&pixels[row*128U],0,128U);
    while (*text && column<21U) {
        glyph(*text++,&pixels[row*128U+column*6U]); ++column;
    }
}
uint8_t OLED_FlushPage(uint8_t page)
{
    uint8_t cmd[3];
    if (page>=8U) return 0U;
    cmd[0]=(uint8_t)(0xB0U+page);cmd[1]=0U;cmd[2]=0x10U;
    if (!send(0U,cmd,3U)) return 0U;
    return send(0x40U,&pixels[page*128U],128U);
}
