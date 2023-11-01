/*
 * Una famosa empresa de calzados a incorporado a sus zapatillas 10
 * luces leds comandadas por un microcontrolador LPC1769 y ha pedido a
 * su grupo de ingenieros que diseñen 2 secuencias de luces que cada
 * cierto tiempo se vayan intercalando (secuencia A - secuencia B- secuencia A- ... ).
 * Como todavía no se ha definido la frecuencia a la cual va a funcionar el CPU del microcontrolador,
 * las funciones de retardos que se incorporen deben tener como parametros de entrada
 * variables que permitan modificar el tiempo de retardo que se vaya a utilizar finalmente.
 * Se pide escribir el código que resuelva este pedido, considerando que los leds se
 * encuentran conectados en los puertos P0,0 al P0.9. Adjuntar el archivo .c bajo el nombre
 * "eje1ApellidoDelEstudiante.c"
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void delay(int time);
void configPorts();
void startSequence1(int retarde);
void startSequence2(int retarde);

int main(void) {
	configPorts();

    while(1) {
    	startSequence1(10000);
    	startSequence2(10000);
    }
    return 0 ;
}

void delay(int time){
	for(int i; i<=time; i++){}
}

void configPorts(){
	//P0.0 - P0.9 en GPIO
	LPC_PINCON->PINSEL0 &= ~(0x003ffff);//xx11 1111 1111 1111 1111
	//P0.0 - P0.9 como salida
	LPC_GPIO0->FIODIR |= (0x000003ff);//0b11 1111 1111
}


//Secuencia acendente
void startSequence1(int retarde){
	LPC_GPIO0->FIOCLR |= (0x000003ff);

	for(int i = 0; i < 10; i++){
		delay(retarde);
		LPC_GPIO0->FIOSET |= (0b1 << i);
	}
}

//Secuencia decendente bit a bit
void startSequence2(int retarde){
	LPC_GPIO0->FIOCLR |= (0x000003ff);

		for(int i = 9; i >= 0; i--){
			LPC_GPIO0->FIOSET |= (0b1 << i);
			delay(retarde);
			LPC_GPIO0->FIOCLR |= (0b1 << i);
		}
}
