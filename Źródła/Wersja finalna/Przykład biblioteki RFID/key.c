/*
*  key.c
* Author: Juffre
*/

/* BASIC HEADERS */
#include "MKL46Z4.h"
#include "key.h"



void key_init(KEY *init,PORT_Type *port,GPIO_Type *gpio,uint8_t nbr,uint8_t repetition_time)
{
	// inicjalizacja pól struktury - gdzie jest podpięty przycisk
	init->PIN_PORT=port;
	init->PIN_GPIO=gpio;
	init->PIN_NUMBER=nbr;
	init->repetition=repetition_time;
	
	// włączenie taktowania dla portu na którym mamy podpięty przycisk
	if(init->PIN_PORT == PORTA) SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	if(init->PIN_PORT == PORTB) SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	if(init->PIN_PORT == PORTC) SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	if(init->PIN_PORT == PORTD) SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	if(init->PIN_PORT == PORTE) SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	

	// konfiguracja przycisku 
	(init->PIN_PORT)->PCR[init->PIN_NUMBER] |= PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
			
}

// sprawdzenie czy przycisk jest wciśnięty
uint8_t push(KEY *push)
{
	if( !( ( push->PIN_GPIO)->PDIR & (1<<(push->PIN_NUMBER)) ) ) return 1; else return 0;
}

// interrupt funtion
// trzeba dopisać przekazywanie funckji 
void debounce_1khz(KEY *key,void (*single_click)(),void (*repeat_click)(void) )
{
	// maszyna stanu sterujaca efektem działania przyciśnięcia 
	if( push(key) && key->state==0)
	{
		key->state=1;
		key->delay=0;
	}

	/* podzielenie oczekiwania na pojedyncze kliknięcie zapewnia dobrą responsywność systemu */
	if(key->state==1 && key->delay==50)
	{
		key->delay=0;
		if( push(key) ) {key->state=2;}  else {if(single_click) single_click(); key->state=0;}
	}
	
	if(key->state==2 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=3;}  else {if(single_click) single_click(); key->state=0;}
	}
	

	if(key->state==3 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=4;}  else {if(single_click) single_click(); key->state=0;}
	}
	
	
	if(key->state==4 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=5;}  else {if(single_click) single_click(); key->state=0;}
	}
	
	if(key->state==5 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=6;}  else {if(single_click) single_click(); key->state=0;}
	}
	if(key->state==6 && key->delay==50)  
	{
		key->delay=0;
		if(push(key)) {key->state=7;} else {key->state=0;if(single_click) single_click();}
	}
	
	// odczekanie przed przystąpieniem do repetacji 
	if(key->state==7 && key->delay==350)
	{
		key->delay=0;
	if(push(key)) {if(repeat_click) repeat_click();key->state=8;} else {key->state=0;}
	}
	
	// repetycja klawisza
	if(key->state==7 && key->delay==key->repetition)
	{
		key->delay=0;
	if(push(key) ) {if(repeat_click) repeat_click();} else {key->state=0;}
	}
	key->delay++;
}




