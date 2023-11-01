/*
 * Escribir un programa en C que permita realizar un promedio movil con los últimos 8
 * datos ingresados por el puerto 1. Considerar que cada dato es un entero signado y
 * está formado por los 16 bits menos significativos de dicho puerto.
 * El resultado, también de 16 bits, debe ser sacado por los pines P0.0 al P0.11 y P0.15 al P0.18.
 * Recordar que en un promedio movil primero se descarta el dato mas viejo de los 8 datos guardados,
 * se ingresa un nuevo dato proveniente del puerto y se realiza la nueva operación de promedio con esos 8
 * datos disponibles, así sucesivamente. Considerar el uso de un retardo antes de tomar una
 * nueva muestra por el puerto.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void show_prom_ouput(int8_t value);
void save_current_value_input();
int8_t calculate_prom();
void configure_ports();
void delay(int time);

int16_t current_values [8] = {0, 0, 0, 0, 0, 0, 0, 0};
int8_t point_value = 0;

int main(void) {

	configure_ports();

    while(1) {
    	int8_t prom = calculate_prom();
    	show_prom_ouput(prom);

    	delay(100000);

    	save_current_value_input();
    }
    return 0 ;
}

void show_prom_ouput(int8_t value){
	int32_t value_show = (value & 0xfff)
						& ((value << 3) & 0x00078000);

	LPC_GPIO0->FIOCLR |= 0x00078fff;

	LPC_GPIO0->FIOSET |= value_show;
}

void save_current_value_input(){
	int16_t value = (LPC_GPIO1->FIOPIN & 0x3)//0-1 ...0000 0000 00xx x000 xxx0 xx00
					& (LPC_GPIO1->FIOPIN & 0x10)>>2//2 ...0000 0000 00xx x000 xxxx x000
					& (LPC_GPIO1->FIOPIN & 0x700)>> 5//3-4-5 ...0000 0000 00xx xxxx xx00 0000
					& (LPC_GPIO1->FIOPIN & 0xffc000)>> 8;//6-15 ...0000 0000 0000 0000

	current_values[point_value] = value;

	if(point_value == 7)
		point_value = 0;
	else
		point_value ++;
}

int8_t calculate_prom(){
	int8_t prom = 0;

	for(int i = 0;i < 8; i++){
		prom += current_values[i];
	}

	return prom;
}

void configure_ports(){
	//P1.0, P1.1, P1.4, P1.8, P1.9, P1.10, P1.14, P1.15, P1.16-P1.23 como entrada GPIO
	LPC_PINCON->PINSEL2 &= ~(0xf03f030f);// 0000 xxxx xx00 0000 xxxx xx00 xxxx 0000
	LPC_PINCON->PINSEL3 &= ~(0x0000ffff);

	LPC_GPIO1->FIODIR &= ~(0x00ffc713);// ...0000 0000 00xx x000 xxx0 xx00

	//P0.0 - P0.11 y P0.15 - P0.18 como salida GPIO
	LPC_PINCON->PINSEL0 &= ~(0xc0ffffff);
	LPC_PINCON->PINSEL1 &= ~(0x0000003f);

	LPC_GPIO0->FIODIR |= (0x00078fff);
}

void delay(int time){
	for (int i = 0; i<=time ;i++){}
}
