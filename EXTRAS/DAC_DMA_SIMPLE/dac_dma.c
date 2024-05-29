/**
 * Se generará una señal triangular con un periodo de 100ms y una amplitud de 3.3V a maxima resolución,
 * esta señal se transmite por DAC.
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_pinsel.h>
#include <lpc17xx_dac.h>
#include <lpc17xx_gpdma.h>
#include <cr_section_macros.h>

#define FREQ_DAC 10
#define SIZE_SAMPLES 2046

void generateSignal();
void configDAC();
void configDMA();

GPDMA_LLI_Type LLI;
uint32_t signal[SIZE_SAMPLES];

int main(void) {
    generateSignal();
    configDAC();
    configDMA();
    while(1) {
    }
    return 0 ;
}

void generateSignal() {
    for(int i = 0; i < SIZE_SAMPLES; i++) {
        if (i < 1024)
        {
            signal[i] = i << 6;//Se desplaza 6 bits para guardar en [15:6] Value.
        } else {
            signal[i] = (SIZE_SAMPLES - i) << 6;//Se desplaza 6 bits para guardar en [15:6] Value.
        }
    }
}

void configDAC() {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Pinnum = 26;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);

    DAC_CONVERTER_CFG_Type dacStruct;
    dacStruct.CNT_ENA = SET;
    dacStruct.DBLBUF_ENA = RESET;
    dacStruct.DMA_ENA = RESET;

    DAC_Init(LPC_DAC);
    DAC_SetBias(LPC_DAC, 0);
    DAC_SetDMATimeOut(LPC_DAC, 25000000/(FREQ_DAC*SIZE_SAMPLES));//Time out = PCLK(25MHz) * 1000000(BIAS) / (Fseñal * N° muestras)
    DAC_ConfigDAConverterControl(LPC_DAC, &dacStruct);
}

void configDMA() {

    LLI.SrcAddr = (uint32_t) signal;
    LLI.DstAddr = (uint32_t) &LPC_DAC->DACR;
    LLI.NextLLI = (uint32_t) &LLI;
    LLI.Control = SIZE_SAMPLES|//Transfer size
                  (0 << 12)|
                  (0 << 15)|
                  (0b010 << 18)|//Source transfer width (32 bits)
                  (0b010 << 21)|//Destination transfer width(32 bits)
                  (1 << 26)|//Source auto increment
                  (0 << 27)|//Destination auto increment
                  (0 << 31);//Terminal count interrupt disable


    GPDMA_Channel_CFG_Type GPDMACfg;
    GPDMACfg.ChannelNum = 0;
    GPDMACfg.SrcMemAddr = (uint32_t) signal;
    GPDMACfg.DstMemAddr = 0;
    GPDMACfg.TransferSize = SIZE_SAMPLES;
    GPDMACfg.TransferWidth = 0;
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    GPDMACfg.SrcConn = 0;
    GPDMACfg.DstConn = GPDMA_CONN_DAC;
    GPDMACfg.DMALLI = (uint32_t) &LLI;
    GPDMA_Init();

    GPDMA_Setup(&GPDMACfg);

    //Deshabilita la interrupcion
    LPC_GPDMACH0->DMACCConfig &= ~(0b1) << 31;

    GPDMA_ChannelCmd(0, ENABLE);
}
