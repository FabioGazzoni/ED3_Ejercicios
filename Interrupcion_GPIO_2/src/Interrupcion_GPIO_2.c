/*
 * Realizar un programa que configure el puerto P0.0 y P2.0 para que provoquen una interrupción
 * por flanco de subida. Si la interrupción es por P0.0 guardar el valor binario 100111
 * en la variable "auxiliar", si es por P2.0 guardar el valor binario 111001011010110.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#define SEQ1 0b100111
#define SEQ2 0b111001011010110

void confPorts();

uint16_t auxiliar = 0b0;

int main(void) {

	confPorts();
    while(1) {

    }
    return 0 ;
}

void EINT3_IRQHandler(){
	//Interrupcion por P0.0, flanco de subida
	if(LPC_GPIOINT->IO0IntStatF & 0b1){
		auxiliar = SEQ1;
	}//Interrupcion por P2.0, flanco de subida
	else if(LPC_GPIOINT->IO2IntStatF & 0b1){
		auxiliar = SEQ2;
	}
}

void confPorts(){
	//P0.0 y P2.0 como GPIO de entrada
	LPC_PINCON->PINSEL0 &= ~(0b11);
	LPC_PINCON->PINSEL4 &= ~(0b11);

	LPC_GPIO0->FIODIR &= ~(0b1);
	LPC_GPIO2->FIODIR &= ~(0b1);

	//Interrupcion por flanco de subida
	LPC_GPIOINT->IO0IntEnR |= (0b1);
	LPC_GPIOINT->IO2IntEnR |= (0b1);

	//Habilito EINT3
	NVIC_EnableIRQ(EINT3_IRQn);
}
