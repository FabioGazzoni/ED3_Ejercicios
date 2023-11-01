/*
 *En los pines P2.0 a P2.7 se encuentra conectado un display de 7 segmentos.
 *Utilizando la variable numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67}
 *que codifica los números del 0 a 9 para ser mostrados en el display,
 *realizar un programa que muestre indefinidamente la cuenta de 0 a 9 en dicho display.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void configPorts();
void sequenceDisplay(int time);
void delay(int time);

uint8_t numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};

int main(void) {
	configPorts();

    while(1) {
    	sequenceDisplay(100000);
    }
    return 0 ;
}

void configPorts(){
	//P2.0 - P2.7 como GPIO
	LPC_PINCON->PINSEL4 &= ~(0x0003ffff); //xx00 0000 0000 0000
	//P2.0 - P2.7 como salida
	LPC_GPIO2->FIODIR |= (0x000000ff);// 1111 1111
}

void sequenceDisplay(int time){
	LPC_GPIO2->FIOCLR |= (0x000000ff);

	for(int i = 0; i <= 9; i++){
		LPC_GPIO2->FIOSET |= numDisplay[i];
		delay(time);
		LPC_GPIO2->FIOCLR |= numDisplay[i];
	}
}

void delay(int time){
	for(int i; i<=time; i++){}
}
