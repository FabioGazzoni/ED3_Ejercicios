/*
 * Utilizando el modo Capture, escribir un código en C  para que guarde en una variable ,llamada "shooter",
 * el tiempo (en milisegundos) que le lleva a una persona presionar dos pulsadores con un único dedo.
 * Adjuntar  el archivo .c del código.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_timer.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_clkpwr.h>

#define PR 0
#define DIVPCLK 1

void confTimer0();
void calculateTime();
uint32_t cap0 = 0;
uint32_t cap1 = 0;
uint32_t timems = 0;

int main(void) {
	confTimer0();

    while(1) {

    }
    return 0 ;
}

void confTimer0(){
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM2, ENABLE);

	PINSEL_CFG_Type pcapt0;
	pcapt0.Portnum = 0;
	pcapt0.Pinnum = 4;
	pcapt0.Funcnum = PINSEL_FUNC_3;
	pcapt0.Pinmode = PINSEL_PINMODE_PULLUP;
	PINSEL_ConfigPin(&pcapt0);

	PINSEL_CFG_Type pcapt1;
	pcapt1.Portnum = 0;
	pcapt1.Pinnum = 5;
	pcapt1.Funcnum = PINSEL_FUNC_3;
	pcapt1.Pinmode = PINSEL_PINMODE_PULLUP;
	PINSEL_ConfigPin(&pcapt1);

	TIM_TIMERCFG_Type timer2;
	timer2.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer2.PrescaleValue = PR + 1; // PR = 0

	TIM_CAPTURECFG_Type capt0;
	capt0.CaptureChannel = 0;
	capt0.FallingEdge = ENABLE;
	capt0.RisingEdge = DISABLE;
	capt0.IntOnCaption = ENABLE;
	TIM_ConfigCapture(LPC_TIM2, &capt0);

	TIM_CAPTURECFG_Type capt1;
	capt1.CaptureChannel = 1;
	capt1.FallingEdge = ENABLE;
	capt1.RisingEdge = DISABLE;
	capt1.IntOnCaption = ENABLE;
	TIM_ConfigCapture(LPC_TIM2, &capt1);

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer2);

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER2, CLKPWR_PCLKSEL_CCLK_DIV_1);

	TIM_Cmd(LPC_TIM2, ENABLE);

	NVIC_EnableIRQ(TIMER2_IRQn);
}

void TIMER2_IRQHandler(){
	if(TIM_GetIntStatus(LPC_TIM2, TIM_CR0_INT)){
		cap0 = TIM_GetCaptureValue(LPC_TIM2, TIM_COUNTER_INCAP0);

	}else if(TIM_GetIntStatus(LPC_TIM2, TIM_CR1_INT)){
		cap1 = TIM_GetCaptureValue(LPC_TIM2, TIM_COUNTER_INCAP1);
		calculateTime();
	}

	TIM_ClearIntPending(LPC_TIM2, TIM_CR1_INT);
	TIM_ClearIntPending(LPC_TIM2, TIM_CR0_INT);
}

void calculateTime(){
	uint32_t capt = cap1 - cap0;
	timems = (capt + 1) * (PR + 1) / (100000000/DIVPCLK) * 1000;
}
