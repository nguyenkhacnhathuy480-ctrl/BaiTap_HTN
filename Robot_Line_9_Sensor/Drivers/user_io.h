#ifndef USER_IO_H
#define USER_IO_H
#include <stdint.h>
void UserIO_Init(void);
uint8_t Buttons_Read(void); /* bit0 START PB5, bit1 CAL PB8, bit2 PAGE PB9 */
void Buzzer_Set(uint8_t on);
#endif
