/*
 * Se envia por UART una cantidad N de datos.
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

uint8_t data_Tx[300] = "Hola mundo\n\rEsta es la asignatura de Electronica Digiral III\n\rSe esta comunicando por UART con DMA\n\r";
GPDMA_LLI_Type LLI_Tx;

int main(void)
{
    conf_pin();
    conf_uart();
    conf_dma();

    while (1)
    {
    }
    return 0;
}

void conf_pin()
{
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 1;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 2;
    PINSEL_ConfigPin(&PinCfg);
}

void conf_uart()
{
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    UARTFIFOConfigStruct.FIFO_DMAMode = ENABLE;
    UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV0;
    UARTFIFOConfigStruct.FIFO_ResetRxBuf = ENABLE;
    UARTFIFOConfigStruct.FIFO_ResetTxBuf = ENABLE;

    UART_ConfigStructInit(&UARTConfigStruct);
    UART_Init(LPC_UART0, &UARTConfigStruct);
    UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);
    UART_TxCmd(LPC_UART0, ENABLE);
}

void conf_dma()
{
    LLI_Tx.SrcAddr = (uint32_t)data_Tx;
    LLI_Tx.DstAddr = (uint32_t)&LPC_UART0->THR;
    LLI_Tx.NextLLI = (uint32_t)&LLI_Tx;
    LLI_Tx.Control = sizeof(data_Tx) |
                     (0 << 12) |
                     (0 << 15) |
                     (0b000 << 18) | // Source transfer width (8 bits)
                     (0b000 << 21) | // Destination transfer width (8 bits)
                     (0b1 << 26) |
                     (0b0 << 27) |
                     (0b1 << 31);

    GPDMA_Init();
    GPDMA_Channel_CFG_Type GPDMACfg;
    GPDMACfg.ChannelNum = 0;
    GPDMACfg.SrcMemAddr = (uint32_t)data_Tx;
    GPDMACfg.DstMemAddr = 0;
    GPDMACfg.TransferSize = sizeof(data_Tx);
    GPDMACfg.TransferWidth = 0;
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    GPDMACfg.SrcConn = 0;
    GPDMACfg.DstConn = GPDMA_CONN_UART0_Tx;
    GPDMACfg.DMALLI = &LLI_Tx;

    GPDMA_Setup(&GPDMACfg);
    LPC_GPDMACH0->DMACCConfig &= ~(0b1 << 31);
    GPDMA_ChannelCmd(0, ENABLE);
}
