/*
 * Dada una señal de 100khz de componente espectral maxima que ingresa por un pin del ADC,
 * que se necesita procesar. El rango dinamico de amplitud es de 3.3v.
 *
 * AGREGADO PERSONAL:
 * 1_Señal generada por DAC
 * 2_ADC con DMA
 *
 * ACLARACION: La frecuencia con la que manda cada muestra es de 100kHz por lo que la señal completa es
 * 2048 veces eso, no es posible enviar todos los datos a esa frecuencia ya que la frecuencia del
 * DAC esta limitada a 200kHz
 */

#include <lpc17xx_adc.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_dac.h>
#include <lpc17xx_gpdma.h>
#include <lpc17xx_timer.h>
#include <lpc17xx_clkpwr.h>

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

void generate_signal();
void confADC();
void confTimer0();
void confDAC();
void confDMA4ADC();

uint32_t valADC[2048];
uint32_t signal_dac[2048];
uint16_t pointerSignalDac = 0;
GPDMA_Channel_CFG_Type dma;

int main(void) {
	generate_signal();
	confADC();
	confDMA4ADC();
	confDAC();
	confTimer0();

    while(1) {

    }
    return 0 ;
}

void generate_signal(){
	for(int i = 0; i <= 1023; i++){
		signal_dac[i] = i;
		signal_dac[1024 + i] = (1023 - i);
	}
}

void confADC(){
	PINSEL_CFG_Type pin_adc;
	pin_adc.Portnum = 0;
	pin_adc.Pinnum = 23;
	pin_adc.Funcnum = 1;
	pin_adc.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pin_adc);

	ADC_Init(LPC_ADC, 200000);//1Khz de AB señal * 2 -> 2KHz de frec de muestreo
	ADC_BurstCmd(LPC_ADC, 1);

	ADC_ChannelCmd(LPC_ADC, 0, ENABLE);

	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, SET);

	/*
	 * Importante, si se utiliza DMA no se debe habilitar el NVIC con ADC
	 */
}

void confDMA4ADC(){
	GPDMA_LLI_Type LLI;
	LLI.SrcAddr = (uint32_t)&LPC_ADC->ADDR0;
	LLI.DstAddr = (uint32_t)valADC;
	LLI.NextLLI = (uint32_t)0;
	LLI.Control = 2048
				| (2<<18) //source width 32 bit
				| (2<<21) //dest. width 32 bit
				| (1<<27) //dest. increment
				| (1<<31);

	GPDMA_Init();

	dma.ChannelNum = 0;
	dma.TransferSize = 2048;
	dma.TransferWidth = 0; //No lo uso
	dma.SrcMemAddr = 0; //No lo uso
	dma.DstMemAddr = (uint32_t) valADC;
	dma.TransferType = GPDMA_TRANSFERTYPE_P2M;
	dma.SrcConn = GPDMA_CONN_ADC;
	dma.DstConn = 0;//No lo uso
	dma.DMALLI = (uint32_t)&LLI;

	GPDMA_Setup(&dma);
	GPDMA_ChannelCmd(0, ENABLE);
	NVIC_SetPriority(DMA_IRQn, 0);
	NVIC_EnableIRQ(DMA_IRQn);
}


void DMA_IRQHandler(){
	GPDMA_ChannelCmd(0, DISABLE);// deshabilito DMA
	// check GPDMA interrupt on channel 0
		if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){ //check interrupt status on channel 0
			// Check counter terminal status
			if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)){
				// Clear terminate counter Interrupt pending
				for(int i = 0; i <= 2048 - 1; i++){
					valADC[i] = (valADC[i] >> 4) & 0xFFF;
				}

				GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0);
				GPDMA_Setup(&dma);
				GPDMA_ChannelCmd(0, ENABLE);// Habilito DMA
			}
			if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0)){
				// Clear error counter Interrupt pending
				GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 0);
				while(1){};
			}
		}
}

void confDAC(){
	PINSEL_CFG_Type pin;
	pin.Portnum = 0;
	pin.Pinnum = 26;
	pin.Funcnum = 2;
	pin.Pinmode = 0;
	pin.OpenDrain = 0;

	PINSEL_ConfigPin(&pin);

	DAC_Init(LPC_DAC);
}


void confTimer0(){
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0, ENABLE);

	TIM_TIMERCFG_Type timer0;
	timer0.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer0.PrescaleValue = 1; // PR = 0

	TIM_MATCHCFG_Type match0;
	match0.MatchChannel = 0;
	match0.IntOnMatch = ENABLE;
	match0.StopOnMatch = DISABLE;
	match0.ResetOnMatch = ENABLE;
	match0.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	match0.MatchValue = (100000000/(100000))/(0+1)-1; // time = 1/fsignal -> MR = (pclk/(fmuestra*cantmuestras))/(PR + 1) - 1

	TIM_ConfigMatch(LPC_TIM0, &match0);

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer0);

	//PCLK = %1
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_1);
	TIM_Cmd(LPC_TIM0, ENABLE);

	DAC_UpdateValue(LPC_DAC, signal_dac[pointerSignalDac]);//first value DAC
	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(){
	pointerSignalDac++;
	if(pointerSignalDac >= 2048){
		pointerSignalDac = 0;
	}
	uint32_t val = signal_dac[pointerSignalDac];
	DAC_UpdateValue(LPC_DAC, val);

	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}
