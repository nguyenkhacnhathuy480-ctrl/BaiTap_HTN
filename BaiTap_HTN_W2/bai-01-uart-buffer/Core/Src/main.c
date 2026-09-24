#include "platform.h"

#define RX_BUFFER_SIZE 64U

int main(void)
{
    static const char prefix[] = "ELE141502: ";
    char buffer[RX_BUFFER_SIZE];
    uint16_t length = 0U;

    Platform_Init();
    UART1_Init(115200U);

    UART1_WriteString("Nhap chuoi, ket thuc bang dau !\r\n");

    while (1) {
        const uint8_t received = UART1_ReadByteBlocking();

        if (received == (uint8_t)'!') {
            buffer[length] = '\0';
            UART1_WriteString(prefix);
            UART1_WriteString(buffer);
            UART1_WriteString("\n\r");
            length = 0U;
        } else if (length < (RX_BUFFER_SIZE - 1U)) {
            buffer[length++] = (char)received;
        } else {
            length = 0U;
            UART1_WriteString("ERROR: BUFFER FULL\n\r");
        }
    }
}
