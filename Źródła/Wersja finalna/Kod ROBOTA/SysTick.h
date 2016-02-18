/*
		Temat: SysTick
		Wersja: 0.1
		Autor: Juffre
		Plik: SysTick.h
		
*/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "MKL46Z4.h"



#define FREQUENCY 1000

void SysTick_init(uint16_t frequency);
void SysTick_change_frequency(uint16_t frequency);

/* WZÓR DO PRZEKOPIOWANIA DO MAIN()




void SysTick_Handler(void)
{

}




*/



#endif

