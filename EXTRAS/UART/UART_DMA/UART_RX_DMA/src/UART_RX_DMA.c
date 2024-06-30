/*
 * Se va a recibir por UART una cantidad N de datos.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_uart.h>
#include <lpc17xx_gpdma.h>

void conf_pin();
void conf_uart();
void conf_dma();

uint8_t data_Rx[10];

int main(void) {
    conf_pin();
    conf_uart();
    conf_dma();

    while(1) {

    }
    return 0 ;
}

void conf_pin() {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 3;
    PINSEL_ConfigPin(&PinCfg);
}

void conf_uart() {
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    UARTFIFOConfigStruct.FIFO_DMAMode = ENABLE;
    UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV0;
    UARTFIFOConfigStruct.FIFO_ResetRxBuf = ENABLE;
    UARTFIFOConfigStruct.FIFO_ResetTxBuf = ENABLE;

    UART_ConfigStructInit(&UARTConfigStruct);
    UART_Init(LPC_UART0, &UARTConfigStruct);
    UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);
}

void conf_dma() {
    GPDMA_Init();
    GPDMA_Channel_CFG_Type GPDMACfg;
    GPDMACfg.ChannelNum = 0;
    GPDMACfg.SrcMemAddr = 0;
    GPDMACfg.DstMemAddr = (uint32_t) data_Rx;
    GPDMACfg.TransferSize = sizeof(data_Rx);
    GPDMACfg.TransferWidth = 0;
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    GPDMACfg.SrcConn = GPDMA_CONN_UART0_Rx;
    GPDMACfg.DstConn = 0;
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);
    GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
    GPDMA_ChannelCmd(0, ENABLE);

    NVIC_EnableIRQ(DMA_IRQn);
}

void DMA_IRQHandler() {
    if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){
        GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
    }else if(GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) || GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1)){
        while(1);
    }
}
