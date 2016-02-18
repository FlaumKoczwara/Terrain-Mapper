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
uint8_t	RFID_request(uint8_t *tag_type);
uint8_t RFID_get_card_serial(uint8_t *serial_out);
uint8_t RFID_show_database(void);
uint8_t RFID_delete_card_from_database(void);
uint8_t RFID_add_card_to_database(void);






#endif

