/*
 * Escribir un programa para que por cada presión de un pulsador, la frecuencia de parpadeo disminuya a la mitad debido a
 * la modificación del registro del Match 0.
 * El pulsador debe producir una interrupción por EINT2 con flanco descendente.
 * Adjuntar el código en C.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_exti.h>
#include <lpc17xx_timer.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_clkpwr.h>

int main(void) {
	confExt();
	confTier2();
    while(1) {

    }
    return 0 ;
}

void confExt(){
	PINSEL_CFG_Type pineint2;
	pineint2.Portnum = 2;
	pineint2.Pinnum = 12;
	pineint2.Funcnum = PINSEL_FUNC_1;
	pineint2.Pinmode = PINSEL_PINMODE_PULLUP;

	PINSEL_ConfigPin(&pineint2);

	EXTI_InitTypeDef ext2;
	ext2.EXTI_Line = EXTI_EINT2;
	ext2.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	ext2.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

	EXTI_Init();
	EXTI_Config(&ext2);
	EXTI_ClearEXTIFlag(EXTI_EINT2);

	NVIC_EnableIRQ(EINT2_IRQn);
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

void EINT2_IRQHandler(){
	LPC_TIM2->TCR &= ~(1);

	uint32_t mr = LPC_TIM2->MR0;
	if(mr <= 4999999 * 10){
		TIM_UpdateMatchValue(LPC_TIM2, 0, mr + 4999999);
	}else{
		TIM_UpdateMatchValue(LPC_TIM2, 0, 4999999);
		LPC_TIM2->TCR |= 1<<1;
		LPC_TIM2->TCR &= ~(1<<1);
	}
	LPC_TIM2->TCR |= 1;

	EXTI_ClearEXTIFlag(EXTI_EINT2);
}

