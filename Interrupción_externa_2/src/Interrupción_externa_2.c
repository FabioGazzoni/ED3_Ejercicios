/*
 * Escribir un programa en C que permita sacar por los pines P1.16 al P1.23 el equivalente en
 * ascii de "a" si la interrupción se ha realizado por interrupción de EINT1 y no hay interrupción
 * pendiente de EINT0. Para el caso en el cual se produzca una interrupción por EINT1 y
 * exista una interrupción pendiente de EINT0 sacar por el  puerto el equivalente en ascii de "A".
 * La interrupción de EINT1 es por el flanco de subida producido por un pulsador identificado
 * como "tecla a/A", mientras que la interrupción EINT0 es por el nivel alto de un pulsador
 * etiquetado como "Activación de Mayusculas".
 *
 * Nota: Valerse de la configuración de los niveles de prioridad para que la pulsación
 * conjunta de "Activación de Mayúsculas" con "tecla a/A" de como resultado a la salida el
 * equivalente en ascii "A".
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>


int main(void) {

    while(1) {

    }
    return 0 ;
}
