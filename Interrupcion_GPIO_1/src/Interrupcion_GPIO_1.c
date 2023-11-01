/*
 * Realizar un programa que configure el puerto P2.0 y P2.1
 * para que provoquen una interrupción por flanco de subida para el primer pin
 * y por flanco de bajada para el segundo. Cuando la interrupción sea por P2.0 se enviará
 * por el pin P0.0 la secuencia de bits 010011010. Si la interrupción es por P2.1 se enviará
 * por el pin P0.1 la secuencia 011100110. Las secuencias se envían únicamente cuando se produce
 * una interrupción, en caso contrario la salida de los pines tienen valores 1 lógicos.
 * ¿que prioridad tienen configuradas por defecto estas interrupciones?
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#define SIZE_SEQ 9

int16_t sequence1 = 0b010011010;
int16_t sequence2 = 0b011100110;


void confgPorts();
void delay(int time);

int main(void) {
	confgPorts();

    while(1) {

    }
    return 0 ;
}

void confgPorts(){
	//P0.0 y P0.1 como salida
	LPC_PINCON->PINSEL0 &= ~(0b1111);
	LPC_GPIO0->FIODIR |= (0b11);
	//P2.0 y P2.1 como entrada con interrupcion
	LPC_PINCON->PINSEL4 &= ~(0b1111);
	LPC_GPIO2->FIODIR &= ~(0b11);

	//Intrerrupcuiones
	LPC_GPIOINT->IO2IntEnR |= (0b1);
	LPC_GPIOINT->IO2IntEnF |= (0b1<<1);

	NVIC_EnableIRQ(EINT3_IRQn);
}

void delay(int time){
	for(int i = 0; i <= time; i++){}
}

void EINT3_IRQHandler(){
	//Verifico si la interrupcion fue por flanco de subida en P2.0
	if(LPC_GPIOINT->IO2IntStatR & 0b1){

		for(int i = (SIZE_SEQ - 1); i >= 0; i--){
			LPC_GPIO0->FIOCLR |= 0b1;
			LPC_GPIO0->FIOSET |= ((sequence1 >> i) & 0b1);
			delay(100000);
		}

		LPC_GPIO0->FIOSET |= 0b1;
		//Limpio el flag de interrupcion del P2.0
		LPC_GPIOINT->IO2IntClr |= (0b1);

	//Verifico si la interrupcion fue por flanco de bajada en P2.1
	}else if((LPC_GPIOINT->IO2IntStatF & (0b1 << 1))){

		for(int i = (SIZE_SEQ - 1); i >= 0; i--){
			LPC_GPIO0->FIOCLR |= 0b1<<1;
			LPC_GPIO0->FIOSET |= (((sequence1 >> i) << 1) & 0b1 << 1);
			delay(100000);
		}

		LPC_GPIO0->FIOSET |= 0b1 << 1;
		//Limpio el flag de interrupcion del P2.1
		LPC_GPIOINT->IO2IntClr |= (0b1 << 1);
	}
}
