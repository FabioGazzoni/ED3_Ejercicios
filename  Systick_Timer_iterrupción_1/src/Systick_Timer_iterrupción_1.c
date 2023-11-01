/*
 * 1.- Configure el Systick Timer de modo que genere una forma de onda llamada PWM
 * tal como la que se muestra en la figura adjunta. Esta señal debe ser sacada por el pin P0.22
 * para que controle la intensidad de brillo del led. El periodo de la señal debe ser de 10 mseg
 * con un duty cycle de 10%. Configure la interrupción externa EINT0 de modo que cada vez que se
 * entre en una rutina de interrupción externa el duty cycle incremente en un 10% (1 mseg).
 * Esto se repite hasta llegar al 100%, luego, si se entra nuevamente a la interrupción externa,
 * el duty cycle volverá al 10%.
 *
 * 2.- Modificar los niveles de prioridad para que la interrupción por systick tenga mayor prioridad
 * que la interrupción externa.
 *
 * 3. Adjuntar el .C  del código generado.
 *
 * ~/Documents/MCUXpresso_11.7.1/Ejercicios_LEV/ Systick_Timer_iterrupción_1
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void confGPIO();
void confExtInt();
void confSystick();
void modiflyPrioriti();

uint32_t duty_cycle_step_limit = 0;
uint32_t current_time_step = 0;

int main(void) {
	confGPIO();
	modiflyPrioriti();
	confExtInt();
	confSystick();

    while(1) {

    }
    return 0 ;
}

void confGPIO(){
	//P0.22 como GPIO
	LPC_PINCON->PINSEL1 &= ~(0b11<<12);
	//P0.22 como salida
	LPC_GPIO0->FIOSET |= (0b1<<22);
}

void confExtInt(){
	//P2.10 como EINT0
	LPC_PINCON->PINSEL4 &= ~(0b1<<21);
	LPC_PINCON->PINSEL4 |= (0b1<<20);
	//Interrupcion por flanco de subida
	LPC_SC->EXTMODE |= 1;
	LPC_SC->EXTPOLAR |= 1;

	NVIC_EnableIRQ(EINT0_IRQn);
}

void confSystick(){
	//SysTick apagado (por defecto) con interrupcion y clock del micro
	SysTick->CTRL |= 0b110;
	//SysTick interrumpe cada 10 ms
	SysTick->LOAD = 100000 - 1;
	//SysTick inicia en 0
	SysTick->VAL = 0;
	//Inicio
	SysTick->CTRL |= 1;
}

void modiflyPrioriti(){
	NVIC_SetPriority(SysTick_IRQn,0);
	NVIC_SetPriority(EINT0_IRQn,1);

}


void EINT0_IRQHandler(){
	//Incremento un paso mas el limite de duty
	duty_cycle_step_limit += 1;

	if(duty_cycle_step_limit > 10){
		duty_cycle_step_limit = 0;
	}

	//Limpio el flag
	LPC_SC->EXTINT |= 0b1;
}

void SysTick_Handler(){
	//Verifico si ya termine un periodo de PWM
	current_time_step += 1;
	if(current_time_step == 10)
		current_time_step = 0;

	//Verifico si estoy en la cuenta de 1 o 0
	if(current_time_step < duty_cycle_step_limit){
		LPC_GPIO0->FIOSET |= (0b1<<22);
	}else{
		LPC_GPIO0->FIOSET &= ~(0b1<<22);
	}

	//Limpio el flag
	SysTick->CTRL &= SysTick->CTRL;
}

