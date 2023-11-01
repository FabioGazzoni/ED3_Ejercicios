/*
 * Algoritmo de antirrebote de un pulsador: Escribir un programa en C que ante la interrupción por
 * flanco de subida del pin P0.1, configurado como entrada digital con pull-down interno, se incremente
 * un contador de un dígito, se deshabilite esta interrupción y se permita la interrupción por systick cada
 * 20 milisegundos. En cada interrupción del systick se testeará una vez el pin P0.1.
 *
 * Solo para el caso de haber testeado 3 estados altos seguidos se sacará por los pines del puerto P2.0 al
 * P2.7 el equivalente en ascii del valor del contador, se desactivará las interrupción por systick y
 * se habilitará nuevamente la interrupción por P0.1.
 *
 * Por especificación de diseño se pide que los pines del
 * puerto 2 que no sean utilizados deben estar enmascarados por hardware.
 * Considerar que el CPU se encuentra funcionando con el oscilador interno RC (4Mhz).
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void confGPIO();
void confSysTick();


uint8_t count_no_bounce = 0;
int count_port_interrupt = 0;

int main(void) {
	confGPIO();
	confSysTick();

    while(1) {

    }
    return 0 ;
}

void confGPIO(){
	//Puerto P0.1 como GPIO entrada con resistencia de pull-down
	LPC_PINCON->PINSEL0 &= ~(0b11<<2);
	LPC_GPIO0->FIOSET &= ~(0b1 << 1);
	LPC_PINCON->PINMODE0 |= (0b11 << 2);

	//Habil de interrupcion por GPIO P0.1 por flanco de subida
	LPC_GPIOINT->IO0IntEnR |= (0b1<<1);
	NVIC_EnableIRQ(EINT3_IRQn);

	//Puertos del P2.0 al P2.7 como salida, demas enmascarados
	LPC_PINCON->PINSEL4 &= ~(0xFFFF);
	LPC_GPIO2->FIOSET |= (0xFF);
	LPC_GPIO2->FIOMASK |= (0xFFFFFF00);
}

void confSysTick(){
	//Reload = 20*10⁻³ * 4*10³ - 1 = 79
	SysTick->CTRL = 0b011; //SysTick apagado, Interrup encendida y clock del micro
	SysTick->LOAD = 79;
	NVIC_EnableIRQ(SysTick_IRQn);
}

void EINT3_IRQHandler(){
	//Desactivo la interrupcion por P0.1 y empiezo a interrumpir por SysTick
	LPC_GPIOINT->IO0IntEnR &= ~(0b1<<1);

	SysTick->CTRL |= 1;//Inicio el SysTick
	SysTick->VAL = 0;//Lo reinicio
}

void SysTick_Handler(){
	//Si en el P0.1 tengo un alto sumo uno a la cuenta de anti salto, sino lo reinicio en 0
	if(LPC_GPIO0->FIOPIN & (0b10)){
		count_no_bounce++;
	}else{
		count_no_bounce = 0;
	}

	//Si llego a 3, paso el anti rebote
	//rehabilito la interrupcion del P0.1 y asigno al P2.0-2.7 el valor de la cuenta
	//y saco la interrup por systick
	if(count_no_bounce == 3){
		SysTick->CTRL &= ~(0b1);//Apago el SysTick
		LPC_GPIOINT->IO0IntEnR |= (0b1<<1);//Habilito la interrupcion por el puerto

		count_no_bounce = 0;
		count_port_interrupt++;

		//Salida por puerto P2.0-7
		LPC_GPIO2->FIOCLR = 0xFFFFFFFF;
		LPC_GPIO2->FIOSET |= count_port_interrupt;
	}

	SysTick->CTRL &= SysTick->CTRL;//Limpio el flag de interrupcion del sistick
}
