/**
 * Se quiere transmitir por comunicación UART un mensaje de texto de forma no bloqueante.
 */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_uart.h>

void confPort();
void confUART();

uint8_t mensaje[] = "Hola mundo este mensaje tiene 41 bytes\n\r";
uint8_t mensajeIndex = sizeof(mensaje);

int main(void) {
    confPort();
    confUART();
    mensajeIndex -= UART_Send(LPC_UART0, &mensaje[0], sizeof(mensaje), NONE_BLOCKING);
    while(1) {}
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
    UART_IntConfig(LPC_UART0, UART_INTCFG_THRE, ENABLE);

    NVIC_EnableIRQ(UART0_IRQn);
}

void UART0_IRQHandler() {
    //Verifico si la interrupción fue por transmisión
    if ((UART_GetIntId(LPC_UART0)& UART_IIR_INTID_MASK)== UART_IIR_INTID_THRE) {
        if (mensajeIndex > 0) {
            mensajeIndex -= UART_Send(LPC_UART0, &mensaje[sizeof(mensaje) - mensajeIndex], mensajeIndex, NONE_BLOCKING);
        }
        else {
            mensajeIndex = sizeof(mensaje);
            mensajeIndex -= UART_Send(LPC_UART0, &mensaje[0], sizeof(mensaje), NONE_BLOCKING);
        }
    }else {
        //Cualquier otro caso no debería ocurrir
        while(1) {}
    }
    
}
