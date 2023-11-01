/*
 * 1.- Adjuntar la foto de una hoja en donde se demuestre que el valor a pasar como parámetro
 * a la función SysTick_Config(), es la frecuencia del reloj del CPU multiplicado por el
 * intervalo de tiempo entre interrupciones o lo que es lo mismo la frecuencia del reloj dividido la
 * frecuencia de interrupción. Nombre de los archivos.
 *
 * ¿el valor pasado como parámetro de entrada a la función SysTick_Config()
 * es el valor que finalmente se carga en el registro RELOAD? Nota:
 * Para responder esta pregunta vea que hace la función con este parámetro de entrada.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

uint32_t timems = 100000;
/*
 * Reload = time[ms]*ClockFrecuenci-1
 */
int main(void) {

	SysTick_Config(1/SystemCoreClock * timems);

    while(1) {

    }
    return 0 ;
}

void SysTick_Handler(){
	SysTick->CTRL &= (0b111);
	//Rutina
	SysTick->CTRL |= (0b011);

	//O al leer o escribir en SysTick->CTRL se borra el bit de interrupcion, entonces:
	//SysTick->CTRL &= SysTick->CTRL;
}
