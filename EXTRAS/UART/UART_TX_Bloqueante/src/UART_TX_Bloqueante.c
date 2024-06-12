/**
 * Se quiere transmitir por comunicación UART un mensaje de texto de forma bloqueante.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_uart.h>

void confPort();
void confUART();
void sendUART();
uint8_t mensaje[] = "Hola mundo este mensaje tiene 41 bytes\n\r";

int main(void) {
    confPort();
    confUART();

    while(1) {
        for (int i = 0; i < 10000000; i++)
        {
            sendUART();
        }
    }
    return 0 ;
}


void confPort() {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 2;
    PINSEL_ConfigPin(&PinCfg);
}

void confUART() {
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    
    UART_ConfigStructInit(&UARTConfigStruct);
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
    UART_Init(LPC_UART0, &UARTConfigStruct);
    UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);
    UART_TxCmd(LPC_UART0, ENABLE);
}

void sendUART() {
    UART_Send(LPC_UART0, mensaje, sizeof(mensaje), BLOCKING);
}
