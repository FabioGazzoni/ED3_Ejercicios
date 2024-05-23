/**
 * Se plantea utilizar el DMA para almacenar los datos leidos por GPIO0.0 a una frecuencia de 100Hz
 * utilizando el MATCH0.0 como request.
 * Luego se repite esta secuencia enviada a travez del DMA por otro canal hacia GPIO0.1 utilizando
 * tambien el MATCH0.0 como request.
 * Un LED por GPIO2.0 indica el momento en el que se esta guardando el dato de GPIO0.0.
 *
 * Como punto extra se utilizan listas linqueadas simples
 */
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <lpc17xx_pinsel.h>
#include <lpc17xx_gpio.h>
#include <lpc17xx_gpdma.h>
#include <lpc17xx_timer.h>
#include <lpc17xx_clkpwr.h>

#define SIZE_VALUES_MEMORY 500

void confGPIO();
void confTimer();
void confLLIs();
void confDMA();

uint8_t array[1000];
GPDMA_LLI_Type LLI0CH0Struct;
GPDMA_LLI_Type LLI0CH1Struct;

int main(void) {
	for (int i = 0; i < 1000; ++i) {
		array[i] = 0;
	}
	confGPIO();
	confTimer();
	confLLIs();
	confDMA();

    while(1) {
    }
    return 0 ;
}

void confGPIO(){
	PINSEL_CFG_Type pinP00Struct;
	pinP00Struct.Portnum = 0;
	pinP00Struct.Pinnum = 0;
	pinP00Struct.Funcnum = PINSEL_FUNC_0;
	pinP00Struct.Pinmode = PINSEL_PINMODE_PULLDOWN;
	PINSEL_ConfigPin(&pinP00Struct);

	GPIO_SetDir(0, 0, 0);//GPIO0.0 como entrada

	PINSEL_CFG_Type pinP01Struct;
	pinP01Struct.Portnum = 0;
	pinP01Struct.Pinnum = 1;
	pinP01Struct.Funcnum = PINSEL_FUNC_0;
	pinP01Struct.Pinmode = PINSEL_PINMODE_PULLDOWN;
	PINSEL_ConfigPin(&pinP01Struct);

	GPIO_SetDir(0, 0b10, 1);//GPIO0.1 como salida

	LPC_GPIO0->FIOMASK = 0xFFFFFFFC;//Todos los pines enmascarados exepto GPIO0.0,1

	//LED indicativo de REC
	PINSEL_CFG_Type pinP20Struct;
	pinP20Struct.Portnum = 2;
	pinP20Struct.Pinnum = 0;
	pinP20Struct.Funcnum = PINSEL_FUNC_0;
	pinP20Struct.Pinmode = PINSEL_PINMODE_PULLDOWN;
	PINSEL_ConfigPin(&pinP20Struct);

	GPIO_SetDir(2, 1<<0, 1);//GPIO2.0 como salida
	GPIO_ClearValue(2, 1<<0);
}

void confTimer(){
	TIM_TIMERCFG_Type timerStruct;
	timerStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
	timerStruct.PrescaleValue = 1;//Sin preescaler
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerStruct);//Timer0 en modo timer y pconp en 1

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_TIMER0, CLKPWR_PCLKSEL_CCLK_DIV_1);//PCLK = 1

	TIM_MATCHCFG_Type matchStruct;
	matchStruct.MatchChannel = 0;
	matchStruct.IntOnMatch = DISABLE;
	matchStruct.StopOnMatch = DISABLE;
	matchStruct.ResetOnMatch = ENABLE;
	matchStruct.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	matchStruct.MatchValue = 1000000;//match cada 0.01s
	TIM_ConfigMatch(LPC_TIM0, &matchStruct);

	TIM_Cmd(LPC_TIM0, ENABLE);
}

void confLLIs(){
//	----------------------Canal 0------------------------------
	LLI0CH0Struct.SrcAddr = (uint32_t) &(LPC_GPIO0->FIOPIN0);
	LLI0CH0Struct.DstAddr = (uint32_t) &array[SIZE_VALUES_MEMORY - 1];
	LLI0CH0Struct.NextLLI = 0;
	LLI0CH0Struct.Control = SIZE_VALUES_MEMORY //Tamaño de trransferencia
							| ( 0 << 18 )		//Tamaño de dato en fuente de 8 bits
							| ( 0 << 21 )		//Tamaño de dato en destino de 8 bits
							| ( 1 << 27 )		//Autoincremento en destino
							| ( 1 << 31 );		//Interrupcion por terminar

//	----------------------Canal 1------------------------------
	LLI0CH1Struct.SrcAddr = (uint32_t) &array[SIZE_VALUES_MEMORY - 1];
	LLI0CH1Struct.DstAddr = (uint32_t) &(LPC_GPIO0->FIOPIN0);
	LLI0CH1Struct.NextLLI = 0;
	LLI0CH1Struct.Control = SIZE_VALUES_MEMORY 	//Tamaño de trransferencia
							| ( 0 << 18 )		//Tamaño de dato en fuente de 8 bits
							| ( 0 << 21 )		//Tamaño de dato en destino de 8 bits
							| ( 1 << 26 )		//Autoincremento en fuente
							| ( 1 << 31 );		//Interrupcion por terminar
}

void confDMA(){
	GPDMA_Init();
//	----------------------General------------------------------
	LPC_GPDMA->DMACConfig = 0x01;

	LPC_GPDMA->DMACSync &= ~(1 << 10);//Logica de sincronización por MATCH0.0
	LPC_SC->DMAREQSEL |= 1 << 0;//Request por MATCH0.0

//	----------------------Canal 0------------------------------
	LPC_GPDMACH0->DMACCSrcAddr = (uint32_t) &(LPC_GPIO0->FIOPIN0);//Fuente como GPIO0.0
	LPC_GPDMACH0->DMACCDestAddr = (uint32_t) &array[0];//Destino array[]
	LPC_GPDMACH0->DMACCLLI =(uint32_t) &LLI0CH0Struct;

	LPC_GPDMACH0->DMACCControl = SIZE_VALUES_MEMORY //Tamaño de trransferencia
								| ( 0 << 18 )		//Tamaño de dato en fuente de 8 bits
								| ( 0 << 21 )		//Tamaño de dato en destino de 8 bits
								| ( 1 << 27 )		//Autoincremento en destino
								| ( 0 << 31 );		//No interrumpe por terminar, lo hace al terminar el ultimo LLI

	LPC_GPDMACH0->DMACCConfig = (1 << 0)   		//Habilitar canal DMA
	                          | (8 << 1)   		//SrcPeripheral: Timer 0 Match 0
	                          | (0 << 6)   		//DestPeripheral: No usado, destino es memoria
	                          | (0b010 << 11)  	//TransferType: Peripheral to Memory (010)
	                          | (1 << 14)  		//Habilitar interrupción de error
	                          | (1 << 15); 		//Habilitar interrupción de terminal count

	GPIO_SetValue(2, 1<<0);//Led indicativo de REC

//	----------------------Canal 1------------------------------
	LPC_GPDMACH1->DMACCSrcAddr = (uint32_t) &array[0];//Fuente como array[]
	LPC_GPDMACH1->DMACCDestAddr = (uint32_t) &(LPC_GPIO0->FIOPIN0);//Destino GPIO0.0
	LPC_GPDMACH1->DMACCLLI = (uint32_t) &LLI0CH1Struct;

	LPC_GPDMACH1->DMACCControl = SIZE_VALUES_MEMORY //Tamaño de trransferencia
								| ( 0 << 18 )		//Tamaño de dato en fuente de 8 bits
								| ( 0 << 21 )		//Tamaño de dato en destino de 8 bits
								| ( 1 << 26 )		//Autoincremento en fuente
								| ( 0 << 31 );		//No interrumpe por terminar, lo hace al terminar el ultimo LLI

	LPC_GPDMACH1->DMACCConfig = (0 << 0)   		//Canal deshabilitado DMA
		                      | (0 << 1)   		//SrcPeripheral: No usado, fuente es memoria
		                      | (8 << 6)   		//DestPeripheral: Timer 0 Match 0
		                      | (0b001 << 11)  	//TransferType: Memory to Peripheral (001)
		                      | (1 << 14)  		//Habilitar interrupción de error
		                      | (1 << 15); 		//Habilitar interrupción de terminal count


	NVIC_EnableIRQ(DMA_IRQn);
}

void DMA_IRQHandler(void) {

    if (LPC_GPDMA->DMACIntTCStat & (1 << 0)) {//Terminal CH0
    	GPIO_ClearValue(2, 1<<0);
        LPC_GPDMA->DMACIntTCClear = (1 << 0); // Limpiar interrupción de terminal count
    	for (int i = 0; i < 1000; ++i) {
    		array[i] = array[i]<<1;
    	}
        LPC_GPDMACH0->DMACCConfig &= ~(0b1);//Deshabilita el canal 0
        LPC_GPDMACH1->DMACCConfig |= 0b1;//Habilita el canal 1

    }else if(LPC_GPDMA->DMACIntTCStat & (1 << 1)){//Terminal CH1
    	confDMA();
    }

    if (LPC_GPDMA->DMACIntErrStat & (1 << 0) || LPC_GPDMA->DMACIntErrStat & (1 << 1)) {
    	while(1){}
    }
}
