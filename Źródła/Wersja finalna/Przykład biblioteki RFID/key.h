/*
 * 	key.h
 *  Author: Juffre
 *  
 */

#ifndef KEY_H_
#define KEY_H_

#include "MKL46Z4.h"

// struktura definiująca podłączenie przycisku oraz maszyny stanów w której się ona znajduje
typedef struct{
	PORT_Type  *PIN_PORT;				// PORT przycisku eg. PORTC
	GPIO_Type  *PIN_GPIO;				// GPIO przycisku eg. PTC
	uint8_t PIN_NUMBER;					// nr przycisku w portcie eg. 3
	uint16_t delay;							// zmienna opóźniająca
	uint8_t state;							// stan maszyny stanów
	uint8_t repetition;         // czas co jaki następuje powtórzenie 
} KEY;

/* FUNCKJA INICJALIZUJĄCA KLAWIATURĘ */
void key_init(KEY *init,PORT_Type *port,GPIO_Type *gpio,uint8_t nbr,uint8_t repetition_time );
// inicjalizujemy strukturę opisującą nasz przycisk 
// pokazujemy na jakim pinie dołączony jest przycisk oraz jakie rejestry za niego odpowiadają
// wybieramy czas repetycji


/* FUNKCJA WYWOŁYWANA W PRZERWANIU co 1ms */
void debounce_1khz(KEY *key,void (*single_click)(void),void (*repeat_click)(void));
// KEY *key przekazujemy strukturę naszego przycisku, która została zainicjalizowana w funckji key_init
// void (*single_click)(void)   wskaźnik na funkcje która zostanie wywołana po jednokrotnym wciśnięciu przycisku
// void (*repeat_click)(void)   wskaźnik na funckję która zostanie wywołana po przytrzymaniu przycisku przez dłuższy czas


/* FUNCJA SPRAWDZAJĄCA PRZYCIŚNIĘCIE OKREŚLONEGO PRZYCISKU */
uint8_t push(KEY *push);
// sprawdzamy czy określony przycisk został wciśnięty

#endif /* HEADER_H_ */

/* PRZYKŁAD UŻYCIA BIBLIOTEKI */
/*
	W pliku main.c przed funckją main() definiujemy obiekt typu KEY; 

  KEY klawisz;
		int main()
		{
     // teraz inicjalizujemy nasz przycisk
			key_init(&klawisz,PORTC,PTC,3,50);
		}

	 // przerwanie z częstotliwością 1kHz
   // wywołujemy funkcje obsługująca dany przycisk
		void 1kHz_IRQ_handler(void)
		{
			debounce_1khz(&klawisz);
		}

*/
















/*********** ZAPASOWA WERSJA FUNCKJI DZIAŁAJĄCEJ W PRZERWANIU ******************************* */



/*
void debounce_1khz(KEY *key,void (*single_click)(uint8_t i),void (*repeat_click)(uint8_t) )
{
	// maszyna stanu sterujaca efektem działania przyciśnięcia 
	if( push(key) && key->state==0)
	{
		key->state=1;
		key->delay=0;
	}

	// podzielenie oczekiwania na pojedyncze kliknięcie zapewnia dobrą responsywność systemu 
	if(key->state==1 && key->delay==50)
	{
		key->delay=0;
		if( push(key) ) {key->state=2;}  else {PTE->PTOR |=MASK_LED2; key->state=0;}
	}
	
	if(key->state==2 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=3;}  else {PTE->PTOR |=MASK_LED2; key->state=0;}
	}
	

	if(key->state==3 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=4;}  else {PTE->PTOR |=MASK_LED2; key->state=0;}
	}
	
	
	if(key->state==4 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=5;}  else {PTE->PTOR |=MASK_LED2; key->state=0;}
	}
	
	if(key->state==5 && key->delay==50)
	{
		key->delay=0;
		if(push(key)) {key->state=6;}  else {PTE->PTOR |=MASK_LED2; key->state=0;}
	}
	if(key->state==6 && key->delay==50)  
	{
		key->delay=0;
		if(push(key)) {key->state=7;} else {key->state=0;PTE->PTOR |=MASK_LED2;}
	}
	
	// odczekanie przed przystąpieniem do repetacji 
	if(key->state==7 && key->delay==350)
	{
		key->delay=0;
	if(push(key)) {PTD->PTOR |=MASK_LED1;key->state=8;} else {key->state=0;}
	}
	
	// repetycja klawisza
	if(key->state==7 && key->delay==50)
	{
		key->delay=0;
	if(push(key) ) {PTD->PTOR |=MASK_LED1;} else {key->state=0;}
	}
	key->delay++;
}





*/


