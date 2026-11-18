#ifndef UART_DEBUG_H
#define UART_DEBUG_H
#include <stdint.h>
void Uart_Init(void);
uint8_t Uart_Write(const char *text); /* drops whole message if ring full */
int Uart_Read(void); /* -1 if empty */
extern volatile uint32_t uart_tx_dropped,uart_rx_dropped;
#endif
