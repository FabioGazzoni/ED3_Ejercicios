/*
 * Configurar el pin P0.4 como entrada digital con resistencia de pull down
 * y utilizarlo para decidir si el valor representado por los pines P0.0 al P0.3
 * van a ser sumados o restados al valor guardado en la variable "acumulador".
 * El valor inicial de "acumulador" es 0.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void configPorts();

int32_t acumulador = 0;

int main(void) {
	configPorts();

    while(1) {
    	//En caso de que P0.4 es 1, sumar lo que hat en P0.0-P0.3
    	if(LPC_GPIO0->FIOPIN & (0b1<<4)){
    		acumulador += (LPC_GPIO0->FIOPIN & 0b111);
    	}else
    		acumulador -= (LPC_GPIO0->FIOPIN & 0b111);
    }
    return 0 ;
}

void configPorts(){
	//P0.0 - P0.4 como GPIO
	LPC_PINCON->PINSEL0 &= ~(0xff);
	//P0.0 - P0.4 como entrada
	LPC_GPIO0->FIODIR &= ~(0b1111);
	//P0.4 con resistencia pull down
	LPC_PINCON->PINMODE0 |= (0b11<<8);
}
