/*
 * Escribir un programa para que por cada presión de un pulsador, la frecuencia de parpadeo de un led disminuya a
 * la mitad debido a la modificación del pre-escaler del Timer 2.
 * El pulsador debe producir una interrupción por EINT1 con flanco descendente.
 * Adjuntar el código en C.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_exti.h>
#include <lpc17xx_timer.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_clkpwr.h>

void confExt();
void confTier2();

int main(void) {
	confExt();
	confTier2();
    while(1) {

    }
    return 0 ;
}

void confExt(){
	PINSEL_CFG_Type pineint1;
	pineint1.Portnum = 2;
	pineint1.Pinnum = 11;
	pineint1.Funcnum = PINSEL_FUNC_1;
	pineint1.Pinmode = PINSEL_PINMODE_PULLUP;

	PINSEL_ConfigPin(&pineint1);

	EXTI_InitTypeDef ext1;
	ext1.EXTI_Line = EXTI_EINT1;
	ext1.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	ext1.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

	EXTI_Init();
	EXTI_Config(&ext1);
	EXTI_ClearEXTIFlag(EXTI_EINT1);

	NVIC_EnableIRQ(EINT1_IRQn);
}

void confTier2(){
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM2, ENABLE);

	//P0.6 en modo match2.0
	PINSEL_CFG_Type pmatch0;
	pmatch0.Portnum = 0;
	pmatch0.Pinnum = 6;
	pmatch0.Funcnum = PINSEL_FUNC_3;
	pmatch0.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pmatch0);

	TIM_TIMERCFG_Type timer2;
	timer2.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer2.PrescaleValue = 1; // PR = 0

	TIM_MATCHCFG_Type match0;
	match0.MatchChannel = 0;
	match0.IntOnMatch = DISABLE;
	match0.StopOnMatch = DISABLE;
	match0.ResetOnMatch = ENABLE;
	match0.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	match0.MatchValue = 4999999; // +- 0.1s

	TIM_ConfigMatch(LPC_TIM2, &match0);

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer2);

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER2, CLKPWR_PCLKSEL_CCLK_DIV_1);

	TIM_Cmd(LPC_TIM2, ENABLE);
}

void EINT1_IRQHandler(){
	LPC_TIM2->TCR &= ~(1);

	uint32_t pr = LPC_TIM2->PR;
	if(pr <= 10){
		LPC_TIM2->PR++;
	}else{
		LPC_TIM2->PR = 0;
	}
	LPC_TIM2->TCR |= 1;

	EXTI_ClearEXTIFlag(EXTI_EINT1);
}
