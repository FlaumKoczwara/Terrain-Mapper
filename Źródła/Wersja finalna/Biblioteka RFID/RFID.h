/**
*	 Biblioteka do obsługi czujnika RC522.
* \author Maciej Flaum Tomasz Koczwara
*
*
*/
#ifndef _RFID_H_
#define _RFID_H_

#include "MKL46Z4.h" 


/* KONFIGURACJA ---------------------------------------------------------------------------------------------- */
/* Biblioteka wykorzystuje moduł SPI */
#define MISO_PORTx PORTA
#define MISO_PTx PTA
#define MISO_NUMBER 17
#define MISO_MUX 2

#define MOSI_PORTx PORTA
#define MOSI_PTx PTA
#define MOSI_NUMBER 16
#define MOSI_MUX 2

#define SCK_PORTx PORTA
#define SCK_PTx PTA
#define SCK_NUMBER 15
#define SCK_MUX 2

#define SS_PORTx PORTA
#define SS_PTx PTA
#define SS_NUMBER 14
#define SS_MUX 1         // GPIO MODE

#define SS_HIGH SS_PTx->PSOR |= (1<<SS_NUMBER)
#define SS_LOW  SS_PTx->PCOR |= (1<<SS_NUMBER)

#define RESET_PORTx  PORTB
#define RESET_PTx		 PTB
#define RESET_NUMBER 9

#define CARD_FOUND			1
#define CARD_NOT_FOUND	2
#define ERROR						3

#define MAX_LEN			16					
#define CARD_ARRAY_SIZE 16			

#define FREE 0		
#define BUSY 1

enum DATABASE_STATE {CARD_ADDED,CARD_DELETED,CARD_IN_BASE,CARD_UNKNOWN};

/** 
*	@struct  T_card
*	\brief Struktura przechowująca dane karty RFID.
*/
typedef struct 
{	
	/** \brief Przechowuje nazwe karty.	*/
	char name[20];    
	/** \brief Przechowuje unikalny numer seryjny karty.	*/	
	uint8_t serial_number[5];
	/** \brief Przechowuje informacje o kierunku jazdy robota.	*/
	uint8_t direction;
	/** \brief Status zajętości struktury.	*/
	uint8_t busy_flag;
}
T_card;

extern T_card MIFARE_CARD_DATABASE[CARD_ARRAY_SIZE];


void RFID_init(void);
uint8_t RFID_get_serial_number(uint8_t *serial_number);           // funkcja niskiego poziomu odczytuje numer seryjny karty
uint8_t RFID_search_database(uint8_t *serial_number);		          // funkcja niskiego poziomu przeszukuje bazę danych w poszukiwaniu danego numeru seryjnego
uint8_t RFID_show_database(uint8_t tab[CARD_ARRAY_SIZE][5]);			// funkcja wysokiego poziomu kopiuje do tablicy użytkownika zawartosc bazy danych
uint8_t RFID_delete_card_from_database(void);											// funkcja wysokiego dodaje do bazy danych odczytany numer seryjny
uint8_t RFID_add_card_to_database(void);													// funkcja wysokiego poziomu usuwa z bazy danych odczytany numer seryjny


#endif

