/*
 * Escribir el código que configure el timer0 para cumplir con las especificaciones dadas en la figura adjunta.
 * (Pag 510 Figura 115 del manual de usuario del LPC 1769).
 * Considerar una frecuencia de cclk de 100 Mhz y una división de reloj de periférico de 2.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_timer.h>
#include <lpc17xx_clkpwr.h>

void confTimer0();

int main(void) {
	confTimer0();
    while(1) {

    }
    return 0 ;
}

void confTimer0(){
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0, ENABLE);

	TIM_TIMERCFG_Type timer0;
	timer0.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer0.PrescaleValue = 3; // PR = 2

	TIM_MATCHCFG_Type match0;
	match0.MatchChannel = 0;
	match0.IntOnMatch = ENABLE;
	match0.StopOnMatch = DISABLE;
	match0.ResetOnMatch = ENABLE;
	match0.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	match0.MatchValue = 6; // +- 3us

	TIM_ConfigMatch(LPC_TIM0, &match0);

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer0);

	//PCLK = %2
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_2);

	TIM_Cmd(LPC_TIM0, ENABLE);

	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(){
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}
