/*
 * Calcular cual es el tiempo máximo que se puede temporizar utilizando un timer en
 * modo match con máximo valor de prescaler y máximo divisor de frecuencia de periférico.
 *
 * Especificar el valor a cargar en los correspondientes registros del timer.
 * Suponer una frecuencia de core cclk de 50 Mhz.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void confTimer();

int main(void) {
	confTimer();

	while(1){

	};
}

void confTimer(){
	LPC_SC->PCON |= (0b1<1);//Habilito el clock del periferico
	LPC_SC->PCLKSEL0 |= (0b11<<2); //Timer0 con PCLK = Clck/8 = 50*10⁻⁶/8 = 6.25*10⁻6

	//Configuro el Timer0 en modo match y el P1.28 como MAT0.0
	LPC_PINCON->PINSEL3 |= (0b11<24);

	LPC_TIM0->PR = 0xFFFFFFFF; // Prescaler al maximo 2³²
	LPC_TIM0->MR0 = 0xFFFFFFFF; // Match 0.0 al tener en timer counter 0 en = 2³²
	LPC_TIM0->MCR |= (0b111); // el Match 0 interrumpe, resetea al desbordar y se detiene el timer0
	LPC_TIM0->EMR |= (0b11<4);

	LPC_TIM0->TCR = 0b11;//Habilito el timer y lo reseteo
	LPC_TIM0->TCR &= ~(0b1<1);//Saco el reset

	//Se producira una interrupcion cada: 1/PCLK * (PR + 1)*(MR0 + 1) = 6.25*6 * (2³² + 1) * (2³² + 1)
	// sgundos: 1.153*10²⁶ s
	// minutos: 1.92*10²⁴ min
	// horas: 3.2*10²² h
	// dias: 1.33*10²¹ dias
	// años: 3.66*10¹⁸ años (edad del universo: 1.37*10¹⁰)

	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(){
	//Hacer algo al interrumpir, ademas al llegar al MAT0.0 se toglea el P1.28

	LPC_TIM0->IR &= ~(0b1); //Limpio la interrupcion
}
