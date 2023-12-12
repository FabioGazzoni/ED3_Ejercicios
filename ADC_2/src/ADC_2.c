/*
 * Configurar 4 canales del ADC para que funcionando en modo burst se obtenga una frecuencia de muestreo en cada uno de 50Kmuestras/seg.
 * Suponer un cclk = 100 Mhz
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_adc.h>
#include <lpc17xx_pinsel.h>

void confADC();

uint16_t val0 = 0;
uint16_t val1 = 0;
uint16_t val2 = 0;
uint16_t val3 = 0;

int main(void) {
	confADC();
    while(1) {

    }
    return 0 ;
}

void confADC(){
	PINSEL_CFG_Type pin_adc;
	pin_adc.Portnum = 0;
	pin_adc.Funcnum = 1;
	pin_adc.Pinmode = PINSEL_PINMODE_TRISTATE;
	PINSEL_ConfigPin(&pin_adc);

	for (int i = 0; i <= 3; ++i) {
		pin_adc.Pinnum = i + 23;
		PINSEL_ConfigPin(&pin_adc);
	}

	ADC_Init(LPC_ADC, 200000);//50Khz por muestra * 4 señales -> 200KHz de frec de muestreo
	ADC_BurstCmd(LPC_ADC, 1);

	for (int i = 0; i <= 3; ++i) {
		ADC_ChannelCmd(LPC_ADC, i, ENABLE);
	}

	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, SET);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN1, SET);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN2, SET);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN3, SET);

	NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_IRQHandler(){
	if(ADC_ChannelGetStatus(LPC_ADC, 0, 1)){//Pregunto si el canal 0 termino
		val0 = ADC_ChannelGetData(LPC_ADC, 0);
	}
	if (ADC_ChannelGetStatus(LPC_ADC, 1, 1)){//Pregunto si el canal 1 termino
		val1 = ADC_ChannelGetData(LPC_ADC, 1);
	}
	if (ADC_ChannelGetStatus(LPC_ADC, 2, 1)){//Pregunto si el canal 2 termino
		val2 = ADC_ChannelGetData(LPC_ADC, 2);
	}
	if (ADC_ChannelGetStatus(LPC_ADC, 3, 1)){//Pregunto si el canal 3 termino
		val3 = ADC_ChannelGetData(LPC_ADC, 3);
	}
}

