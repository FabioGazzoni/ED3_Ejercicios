/**
 * Se quiere recibir por comunicación UART un mensaje de 20 bits de texto de forma no bloqueante.
 */ 

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_uart.h>

void confPort();
void confUART();

uint8_t mensaje[20];
uint8_t mensajeIndex = sizeof(mensaje);

int main(void) {
    confPort();
    confUART();

    while(1) {}
    return 0 ;
}

void confPort() {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 3;
    PINSEL_ConfigPin(&PinCfg);
}

void confUART() {
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    
    UART_ConfigStructInit(&UARTConfigStruct);
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
    UART_Init(LPC_UART0, &UARTConfigStruct);
    UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);
    // Habilita interrupcion por el estado de la linea UART
    UART_IntConfig(LPC_UART0, UART_INTCFG_RLS, ENABLE);
    // Habilita interrupcion por el RX del UART
    UART_IntConfig(LPC_UART0, UART_INTCFG_RBR, ENABLE);
    NVIC_EnableIRQ(UART0_IRQn);
}

/**
 * Cuando se recibe algún dato, se ejecuta la interrupción UART0_IRQHandler,
 * se almacena el mensaje en la variable mensaje de forma no bloqueante.
 * No se espera a recibir los 20 bytes, solo se almacena lo que hay en la FIFO en ese momento.
 * Los datos faltantes se recibirán en las siguientes interrupciones.
 */
void UART0_IRQHandler() {
	uint32_t intsrc, tmp, tmp1; // Receive Data Available or Character time-out
	// Determina la fuente de interrupcion
	intsrc = UART_GetIntId(LPC_UART0);
	tmp = intsrc & UART_IIR_INTID_MASK;
	// Evalua Line Status
	if (tmp == UART_IIR_INTID_RLS)
	{
		tmp1 = UART_GetLineStatus(LPC_UART0);
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// ingresa a un Loop infinito si hay error
		if (tmp1)
		{
			while (1)
			{
			};
		}
	}

	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
	{
		mensajeIndex -= UART_Receive(LPC_UART0, &mensaje[sizeof(mensaje) - mensajeIndex], mensajeIndex, NONE_BLOCKING);
        if (mensajeIndex == 0) {
            mensajeIndex = sizeof(mensaje);
        }
    }
}
