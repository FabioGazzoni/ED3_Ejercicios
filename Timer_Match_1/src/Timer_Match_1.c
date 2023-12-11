/*
 * Utilizando el Timer 1, escribir un código en C para que por cada presión de un pulsador,la frecuencia
 * de parpadeo de un led disminuya a la mitad debido a la modificación del reloj que llega al periférico.
 *
 * El pulsador debe producir una interrupción por GPIO0 con flanco descendente.
 * Adjuntar el código en C.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void confGPIO();
void confTimer();

int main(void) {

	confGPIO();

    while(1) {

    }
    return 0 ;
}

void confGPIO(){
	LPC_PINCON->PINSEL0 &= ~(0b11);// P0.0 como GPIO
	LPC_PINCON->PINMODE0  &= ~(0b11);//P0.0 con resistencia de pull-up

	LPC_GPIO0->FIODIR |= (0b1);//P0.0 como salida
	LPC_GPIO0->FIOCLR |= ~(0b1);//Empiezo con el led apagado
	LPC_GPIOINT->IO0IntEnF |= 0b1;//P0.0 interrumpe por flanco de bajada

	NVIC_EnableIRQ(EINT3_IRQn);
}

void confTimer(){
//	PCLK del Timer1 inicia en 8
	LPC_SC->PCONP |= (0b1<2);
	LPC_SC->PCLKSEL0 |= (0b111<4);

	LPC_TIM1->IR |= 0b1;//Interrupcion por MAT1.0
	LPC_TIM1->MCR |= 0b11;//MAT1.0 interrumpe y reinicia el TR1 pero no se detiene
	// MAT1.0 cada 20ms con N=8 PR=0
	LPC_TIM1->PR = 0;
	LPC_TIM1->MR0 = 250000;
}

void EINT3_IRQHandler(){

	switch((LPC_SC->PCLKSEL0>>2) & 0b11){
		case 0b11://PCLK 8
			LPC_SC->PCLKSEL0 &= ~(0b11<2);//N=4
		case 0b00://PCLK 4
			LPC_SC->PCLKSEL0 |= (0b01<2);//N=2
			LPC_SC->PCLKSEL0 &= ~(0b10<2);//N=2
		case 0b01://PCLK 2
			LPC_SC->PCLKSEL0 |= (0b10<2);//N=1
			LPC_SC->PCLKSEL0 &= ~(0b01<2);//N=1
		case 0b10://PCLK 1
			LPC_SC->PCLKSEL0 |= (0b11<2);//N=8
	}

	LPC_GPIOINT->IO0IntClr |= 0b1;//limpio la interrupcion del P0.0

}

