/*
 * 1_Generar con timer0 una señal de freq. variable.
 * 2_Usando el capture “medir”el periodo usando otro timer.
 * 3_Prender leds tipo vúmetro según la frecuencia.
 * 4_Con un pulsador cambiar la frecuencia de pasos de 100khz(lo hago con pasos de 1khz). Actualizar el vúmetro.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_gpio.h>
#include <lpc17xx_exti.h>
#include <lpc17xx_timer.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_clkpwr.h>

#define PR 0
#define PCLK 100000000

void confGPIO();
void confEINT();
void confTimer2();
void confTimer3();
void calculateFrequenci(uint32_t first, uint32_t second);
void updateVolmetro(uint32_t freq);

uint32_t freqHz = 1000;
uint8_t statusCapture = 0;
uint32_t captureSync = 0;
uint32_t captureA = 0;
uint32_t captureB = 0;
uint32_t captureFreq = 0;

int main(void) {
	confGPIO();
	confEINT();
	confTimer2();
	confTimer3();
    while(1) {

    }
    return 0 ;
}

/*
 * GPIO0.0 - 0.3 como vulometro
 */
void confGPIO(){
	PINSEL_CFG_Type pgpio;
	pgpio.Funcnum = 0;
	pgpio.Portnum = 0;
	pgpio.Pinmode = PINSEL_PINMODE_PULLDOWN;

	for (int i = 0; i <= 3; ++i) {
		pgpio.Pinnum = i;
		PINSEL_ConfigPin(&pgpio);
		GPIO_SetDir(0, 1<<i, 1);
		GPIO_ClearValue(0, 1<<i);
	}
}

/*
 * Se interrumpe por EXINT0 para cambiar la frecuencia actual de la salida generada por Timer 2
 */
void confEINT(){
	PINSEL_CFG_Type pexti0;
	pexti0.Portnum = 2;
	pexti0.Pinnum = 10;
	pexti0.Funcnum = 1;
	pexti0.Pinmode = PINSEL_PINMODE_PULLUP;
	PINSEL_ConfigPin(&pexti0);

	EXTI_InitTypeDef exti0;
	exti0.EXTI_Line = EXTI_EINT0;
	exti0.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	exti0.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

	EXTI_Init();
	EXTI_Config(&exti0);
	EXTI_ClearEXTIFlag(EXTI_EINT0);

	NVIC_EnableIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler(){
	freqHz += 1000;
	if(freqHz > 4000){
		freqHz = 0;
	}

	TIM_UpdateMatchValue(LPC_TIM2, 0, (PCLK/(2*freqHz))/(PR + 1) - 1);
	TIM_ResetCounter(LPC_TIM2);
	updateVolmetro(freqHz);
	for (int i = 0; i < 5000000; ++i) {//Anti rebote

	}
	EXTI_ClearEXTIFlag(EXTI_EINT0);
}

/*
 * Timer 2 con Match 0 con TRISTATE generando una señal a la freciencia freqHz
 */
void confTimer2(){
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM2, ENABLE);

	PINSEL_CFG_Type pmatch0;
	pmatch0.Portnum = 0;
	pmatch0.Pinnum = 6;
	pmatch0.Funcnum = PINSEL_FUNC_3;
	pmatch0.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pmatch0);

	TIM_TIMERCFG_Type timer2;
	timer2.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer2.PrescaleValue = PR + 1; // PR = 0

	TIM_MATCHCFG_Type match0;
	match0.MatchChannel = 0;
	match0.IntOnMatch = DISABLE;
	match0.StopOnMatch = DISABLE;
	match0.ResetOnMatch = ENABLE;
	match0.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	match0.MatchValue = (PCLK/(2*freqHz))/(PR + 1) - 1; // empieza en 1khz

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &timer2);

	TIM_ConfigMatch(LPC_TIM2, &match0);


	//PCLK = %1
	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER2, CLKPWR_PCLKSEL_CCLK_DIV_1);

	TIM_Cmd(LPC_TIM2, ENABLE);
}

/*
 * Timer 3 con Capture 1 midiendo la salida de la señal generada por Timer 2
 */
void confTimer3(){
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCTIM0, ENABLE);

	PINSEL_CFG_Type pmatch0;
	pmatch0.Portnum = 0;
	pmatch0.Pinnum = 24;
	pmatch0.Funcnum = PINSEL_FUNC_3;
	pmatch0.Pinmode = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pmatch0);

	TIM_TIMERCFG_Type timer3;
	timer3.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timer3.PrescaleValue = PR + 1; // PR = 0

	TIM_Init(LPC_TIM3, TIM_TIMER_MODE, &timer3);

	TIM_CAPTURECFG_Type capt1;
	capt1.CaptureChannel = 1;
	capt1.FallingEdge = DISABLE;
	capt1.RisingEdge = ENABLE;
	capt1.IntOnCaption = ENABLE;

	TIM_ConfigCapture(LPC_TIM3, &capt1);

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER3, CLKPWR_PCLKSEL_CCLK_DIV_1);

	TIM_Cmd(LPC_TIM3, ENABLE);

	TIM_ClearIntPending(LPC_TIM3, TIM_CR1_INT);

	NVIC_EnableIRQ(TIMER3_IRQn);
}

void TIMER3_IRQHandler(){
	if(statusCapture == 0){
		captureA = TIM_GetCaptureValue(LPC_TIM3, TIM_COUNTER_INCAP1);
		statusCapture = 1;
	}else if(statusCapture == 1){
		captureB =  TIM_GetCaptureValue(LPC_TIM3, TIM_COUNTER_INCAP1);
		calculateFrequenci(captureA, captureB);
		statusCapture = 2;
	}else if (statusCapture == 2){
		captureA = TIM_GetCaptureValue(LPC_TIM3, TIM_COUNTER_INCAP1);
		calculateFrequenci(captureB, captureA);
		statusCapture = 1;
	}

	TIM_ClearIntPending(LPC_TIM3, TIM_CR1_INT);
}

/*
 * Calcula la frecuencia actual generada por Timer 2
 */
void calculateFrequenci(uint32_t first, uint32_t second){
	uint32_t firstTime_us = (first + 1) * (PR + 1); //(capt + 1) * (PR + 1) / PCLK; para no perder valores se multiplica por PCLK en el calculo de la fecuencia
	uint32_t secondTime_us = (second + 1) * (PR + 1);

	captureFreq = PCLK/((secondTime_us - firstTime_us));
	updateVolmetro(captureFreq);
}

/*
 * Actualiza el volumetro segun la frecuencia medida por el Capture de Timer 3
 */
void updateVolmetro(uint32_t freq){
	GPIO_ClearValue(0, 0b1111);
	for (int i = 0; i <= 3; ++i) {
		if(freq >= 1000 * (1 + i)){
			GPIO_SetValue(0, 0b1<<i);
		}
	}
}
