/**
 * Se quiere utilizar el modulo PWM como generador de señales.
 * la frecuencia de la señal PWM debe ser de 1KHz con un clock de 100MHz.
 * se debe utilizar como Double Edge PWM.
 * Se debe utilizar el canal 2 del modulo PWM.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_pwm.h>

void confPin();
void confPWM();

int main(void) {
	confPin();
	confPWM();
    while(1) {
    }
    return 0 ;
}

void confPin() {
    PINSEL_CFG_Type pinCfg;
    pinCfg.Funcnum = 1; // Funcion PWM1.2
    pinCfg.OpenDrain = 0;
    pinCfg.Pinmode = PINSEL_PINMODE_TRISTATE; // Tri-state (no pull-up ni pull-down)
    pinCfg.Pinnum = 1; // P2.1
    pinCfg.Portnum = 2;
    PINSEL_ConfigPin(&pinCfg); // Configurar pin como PWM1.2
}


void confPWM() {   
    PWM_TIMERCFG_Type pwmCfg;
    pwmCfg.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL; // Prescale en ticks
    pwmCfg.PrescaleValue = 250; // 25MHz / 250 = 1MHz
    PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void*)&pwmCfg); // Inicializar PWM1 como temporizador

    PWM_MATCHCFG_Type pwmMatch1CfgCH2;
    pwmMatch1CfgCH2.IntOnMatch = DISABLE; // No hay interrupcion al hacer match
    pwmMatch1CfgCH2.MatchChannel = 1; // Canal 1
    pwmMatch1CfgCH2.ResetOnMatch = DISABLE; // No reiniciar contador al hacer match
    pwmMatch1CfgCH2.StopOnMatch = DISABLE; // No detener contador al hacer match
    PWM_ConfigMatch(LPC_PWM1, &pwmMatch1CfgCH2); // Configurar match

    PWM_MATCHCFG_Type pwmMatchCfgCH2;
    pwmMatchCfgCH2.IntOnMatch = DISABLE; // No hay interrupcion al hacer match
    pwmMatchCfgCH2.MatchChannel = 2; // Canal 2
    pwmMatchCfgCH2.ResetOnMatch = DISABLE; // No reiniciar contador al hacer match
    pwmMatchCfgCH2.StopOnMatch = DISABLE; // No detener contador al hacer match
    PWM_ConfigMatch(LPC_PWM1, &pwmMatchCfgCH2); // Configurar match
    PWM_ChannelCmd(LPC_PWM1, 2, ENABLE); // Habilitar canal 2

    PWM_MATCHCFG_Type pwmMatchCfgCH0;
    pwmMatchCfgCH0.IntOnMatch = DISABLE; // Interrupcion al hacer match
    pwmMatchCfgCH0.MatchChannel = 0; // Canal 0
    pwmMatchCfgCH0.ResetOnMatch = ENABLE; // Reiniciar contador al hacer match
    pwmMatchCfgCH0.StopOnMatch = DISABLE; // No detener contador al hacer match
    PWM_ConfigMatch(LPC_PWM1, &pwmMatchCfgCH0); // Configurar match

    PWM_ChannelConfig(LPC_PWM1, 2, PWM_CHANNEL_DUAL_EDGE); // Configurar canal 2 como double edge
    PWM_MatchUpdate(LPC_PWM1, 0, 1000, PWM_MATCH_UPDATE_NOW); // 1KHz
    PWM_MatchUpdate(LPC_PWM1, 2, 500, PWM_MATCH_UPDATE_NOW); // 50% de ciclo de trabajo (500/1000)
    PWM_MatchUpdate(LPC_PWM1, 1, 800, PWM_MATCH_UPDATE_NOW); // 80% de ciclo de trabajo (800/1000)
    PWM_ResetCounter(LPC_PWM1); // Reiniciar contador
    PWM_CounterCmd(LPC_PWM1, ENABLE); // Habilitar contador
    PWM_Cmd(LPC_PWM1, ENABLE); // Habilitar PWM1
}
