/**
 * Se toma una señal por el ADC de 1000 muestras y luergo se reproduce por el DAC
 * IMPORTANTE: La frecuencia del ADC esta en modo burst de 200kHz por muestra.
 * La frecuencia de salida del DAC no es la misma que la frecuencia establecida por el TimeOut.
 */


#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_pinsel.h>
#include <lpc17xx_adc.h>
#include <lpc17xx_gpio.h>
#include <lpc17xx_gpdma.h>
#include <lpc17xx_dac.h>

void confPorts();
void confADC();
void confDAC();
void confDMA();

#define FREQ_DAC 1000//Es la frecuencia a la que se muestran todos los datos, la frecuencia de la señal depende de la señal muestreada
#define SIZE_SAMPLES 1000

GPDMA_LLI_Type DMA_LLI_DAC;
uint32_t dataADC[SIZE_SAMPLES];
uint32_t dataDAC[SIZE_SAMPLES];

int main(void) {
    confPorts();
    confADC();
    confDAC();
    confDMA();

    while(1) {
        
    }
    return 0 ;
}

//Configuracion de los pines para el ADC, DAC y led por GPIO2.0
void confPorts(){
    PINSEL_CFG_Type portADCStruct;
    portADCStruct.Funcnum = 1;
    portADCStruct.OpenDrain = 0;
    portADCStruct.Pinmode = PINSEL_PINMODE_TRISTATE;
    portADCStruct.Pinnum = 23;
    portADCStruct.Portnum = 0;
    PINSEL_ConfigPin(&portADCStruct);

    PINSEL_CFG_Type portDACStruct;
    portDACStruct.Funcnum = 2;
    portDACStruct.OpenDrain = 0;
    portDACStruct.Pinmode = PINSEL_PINMODE_TRISTATE;
    portDACStruct.Pinnum = 26;
    portDACStruct.Portnum = 0;
    PINSEL_ConfigPin(&portDACStruct);

    PINSEL_CFG_Type portGPIO20Struct;
    portGPIO20Struct.Funcnum = 0;
    portGPIO20Struct.OpenDrain = 0;
    portGPIO20Struct.Pinmode = PINSEL_PINMODE_PULLUP;
    portGPIO20Struct.Pinnum = 0;
    portGPIO20Struct.Portnum = 2;
    PINSEL_ConfigPin(&portGPIO20Struct);

    GPIO_SetDir(2, 1<<0, 1);//Led como salida
    GPIO_SetValue(2, 0<<0);//Apagar led

}

//Configuracion del ADC a maxima frecuencia de muestreo en modo burst con dma, no interrumpe el ADC sino que lo hara el DMA
void confADC(){
    ADC_Init(LPC_ADC, 200000);//Frecuencia de muestreo de 200KHz (maxima)
    ADC_BurstCmd(LPC_ADC, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);//Canal 0 del ADC (P0.23)
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
}

//Configuracion del DAC a maxima frecuencia (1MHz) y con DMA
void confDAC(){
    DAC_Init(LPC_DAC);
    
    DAC_CONVERTER_CFG_Type DACStruct;
    DACStruct.CNT_ENA = SET;
    DACStruct.DBLBUF_ENA = RESET;
    DACStruct.DMA_ENA = SET;
    DAC_ConfigDAConverterControl(LPC_DAC, &DACStruct);

    uint32_t timeOut = 25000000/(FREQ_DAC*SIZE_SAMPLES); //Time out = PCLK(25MHz) * 1000000(BIAS) / (Fseñal * N° muestras)

    if(timeOut < 25){//No se cumple el tiempo de establecimiento de 1MHz por muestra
    	while(1){}
    }

    DAC_SetDMATimeOut(LPC_DAC, timeOut);
}

//Configuracion del DMA para el ADC, los datos se guardaran en dataADC y interrumpira cuando se llene
void confDMA(){
    GPDMA_Init();
//---------------------Canal 0 (ADC)---------------------
    GPDMA_Channel_CFG_Type DMACh0Struct;
    DMACh0Struct.ChannelNum = 0;
    DMACh0Struct.SrcMemAddr = (uint32_t) &(LPC_ADC->ADDR0);
    DMACh0Struct.DstMemAddr = (uint32_t) dataADC;
    DMACh0Struct.TransferSize = SIZE_SAMPLES;
    DMACh0Struct.TransferWidth = 0;
    DMACh0Struct.TransferType = GPDMA_TRANSFERTYPE_P2M;
    DMACh0Struct.SrcConn = GPDMA_CONN_ADC;
    DMACh0Struct.DstConn = 0;
    DMACh0Struct.DMALLI = 0;
    GPDMA_Setup(&DMACh0Struct);
    GPDMA_ChannelCmd(0, ENABLE);

//---------------------Canal 1 (DAC)---------------------
    DMA_LLI_DAC.SrcAddr = (uint32_t) dataDAC;
    DMA_LLI_DAC.DstAddr = (uint32_t) &(LPC_DAC->DACR);
    DMA_LLI_DAC.NextLLI = (uint32_t) &DMA_LLI_DAC;
    DMA_LLI_DAC.Control = SIZE_SAMPLES
                        | (0b010 << 18)//Source size 32 bits
                        | (0b010 << 21)//Destination size 32 bits
                        | (0b1 << 26)//Source increment
                        | (0b0 << 27)//Destination increment disable
                        | (0b0 << 31);//Terminal count interrupt disable

    GPDMA_Channel_CFG_Type DMACh1Struct;
    DMACh1Struct.ChannelNum = 1;
    DMACh1Struct.SrcMemAddr = (uint32_t) dataDAC;
    DMACh1Struct.DstMemAddr = (uint32_t) &(LPC_DAC->DACR);
    DMACh1Struct.TransferSize = SIZE_SAMPLES;
    DMACh1Struct.TransferWidth = 0;
    DMACh1Struct.TransferType = GPDMA_TRANSFERTYPE_M2P;
    DMACh1Struct.SrcConn = 0;
    DMACh1Struct.DstConn = GPDMA_CONN_DAC;
    DMACh1Struct.DMALLI = (uint32_t) &DMA_LLI_DAC;
    GPDMA_Setup(&DMACh1Struct);

    //Deshabilita la interrupcion del canal 1
    LPC_GPDMACH1->DMACCConfig &= ~(0b1) << 31;

    NVIC_EnableIRQ(DMA_IRQn);
}

//Interrupcion del DMA, enciende el led cuando se llena el buffer de datos, verifica error y limpia la interrupcion
void DMA_IRQHandler(){
    if(GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){//---------Canal 0-----------
        for(int i = 0; i < SIZE_SAMPLES; i++){
        	dataADC[i] = 0x0FFF & (dataADC[i] >> 4);//Data de ADC se encuentra en [18;4] y
        }
        for(int i = 0; i < SIZE_SAMPLES; i++){

        	if(dataADC[i] >= 1024){
        		dataDAC[i] = 1023 << 6;//Maximo valor permitido del DAC
        	}else{
        		dataDAC[i] = (dataADC[i]) << 6; //Hay que posicionarlo en [6;15] para el DAC
        	}

        }
        GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
        GPIO_SetValue(2, 1<<0);
        GPDMA_ChannelCmd(0, DISABLE);
        GPDMA_ChannelCmd(1, ENABLE);

    }
    else if(GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) || GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1)){
        while(1);
    }
}
