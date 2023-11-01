/*
 * Configurar la interrupción externa EINT1 para que interrumpa por flanco de bajada y
 * la interrupción EINT2 para que interrumpa por flanco de subida. En la interrupción por
 * flanco de bajada configurar el systick para desbordar cada 25 mseg, mientras que en la
 * interrupción por flanco de subida configurarlo para que desborde cada 60 mseg.
 * Considerar que EINT1 tiene mayor prioridad que EINT2.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

void confEINT();
void confSystic(uint32_t mseg);

int main(void) {

	confEINT();

    while(1) {

    }
    return 0 ;
}

void confEINT(){
	//Puertos como EINT1 y EINT2
	LPC_PINCON->PINSEL4 |= (0b01 << 22);
	LPC_PINCON->PINSEL4 |= (0b01 << 24);

	//Interrupcion externa por flanco
	LPC_SC->EXTMODE |= (0b1 << 1);
	LPC_SC->EXTMODE |= (0b1 << 2);

	//Interrupcion por flaco de bajada para el EINT1 y de subida para EINT2
	LPC_SC->EXTPOLAR |= (0b1<<1);
	LPC_SC->EXTPOLAR &= (0b1<<2);

	//Limpio los flags de interrupcion (por precaucion)
	LPC_SC->EXTINT |= (0b11<<1);

	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);
}

void confSystic(uint32_t mseg){
	// t[ms]*peridClock[ms] (100MHz) - 1
	uint32_t systickReload = mseg/(1000*SystemCoreClock) - 1;

	//Limpio el control del systick
	SysTick->CTRL = 0;
	//Cargo el reload value
	SysTick->LOAD = systickReload;
	//Limpio el valor actual
	SysTick->VAL = 0;
	//Configuro el Systick sin interrupcion y clock interno
	SysTick->CTRL |= (0b101);
}

void EINT1_IRQHandler(){
	confSystic(25);
	//Limpio la interrupcion
	LPC_SC->EXTINT |= (0b1 <<1);
}

void EINT2_IRQHandler(){
	confSystic(60);
	//Limpio la interrupcion
	LPC_SC->EXTINT |= (0b1 <<2);
}
