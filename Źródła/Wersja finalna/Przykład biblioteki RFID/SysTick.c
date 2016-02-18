/*
		Temat: SysTick
		Wersja: 0.1
		Autor: Juffre
		Plik: SysTick.c
		
*/
/* HEADERS ------------------------------------------------------------------- */
#include "MKL46Z4.h"
#include "SysTick.h"

/* INITIALIZATION ------------------------------------------------------------ */
void SysTick_init(uint16_t frequency)
{
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/frequency);
}
void SysTick_change_frequency(uint16_t frequency)
{
	SysTick_Config(SystemCoreClock/frequency);
}
// that's all :D



