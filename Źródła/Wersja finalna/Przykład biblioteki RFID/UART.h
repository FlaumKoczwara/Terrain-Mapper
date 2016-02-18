
/* ------------------------------------------------------------------------------------------------------------------------------------------- *
		Temat: Obsługa UART 1/2 (do wyboru)
		Wersja: 0.2
		Autor: Juffre
		Data: 19.8.2015
		Wymagania: BUS_CLOCK == 24[MHz]
		Transmisja: 8bit 1bit stop, brak bitów parzystości 
		Plik: UART.h		
	
	Ficzery: 0.2 ( wersja rozwojowa )
		- automatyczne ustawianie BAUD RATE
		- bufor umoźliwiający wysłanie wiele znaków na raz
		- odbiór wielu znaków na raz
		
		Do zrobienia:
		- bufor cykliczny  ( w funkcji wysyłającej 
		- 
		
* -------------------------------------------------------------------------------------------------------------------------------------------- */
#ifndef UART_H_
#define UART_H_

#include "MKL46Z4.h" 


/* KONFIGURACJA ---------------------------------------------------------------------------------------------- */
#define UART1_TX PORTE,0,3			// wybieramy pin nadajnika UART    port,numer pinu,wartość multipleksera
#define UART1_RX PORTE,1,3			// wybieramy pin odbiornika UART   port,numer pinu,wartość multipleksera

#define UART2_TX PORTE,22,4			// wybieramy pin nadajnika UART    port,numer pinu,wartość multipleksera
#define UART2_RX PORTE,23,4			// wybieramy pin odbiornika UART   port,numer pinu,wartość multipleksera

#define TX_BUFOR_SIZE 128
#define RX_BUFOR_SIZE 128






/* FUNKCJE UŻYTKOWNIKA --------------------------------------------------------------------------------------- */

// funckja inicjalizująca moduł UART
void UART1_init(uint32_t BAUD_RATE);
void UART2_init(uint32_t BAUD_RATE);

void UART_send(UART_Type *UARTx,char *string);
void UART_get(UART_Type *UARTx,char *string); 
void UART_send_int(UART_Type *UARTx, int count);



#endif

