/*
*			Topic: Przyklad wykorzystania biblioteki RFID. Zapisywanie,wyswietlanie oraz usuwanie zapisanych numerow kart seryjnych w bibliotece.
*
*			Author:  Juffre
*
*/

/* BASIC HEADERS ---------------------------------------------- */
#include "MKL46Z4.h"
#include "SysTick.h"
#include "UART.h"
#include "KEY.h"
#include "RFID.h"

/* DEFINITIONS -------------------------------------------------*/	
volatile uint16_t sec1;
volatile uint16_t sec5,sec2,sec3;
volatile uint16_t soft_timer;
uint8_t tab[16][5];
uint8_t serial_number[5];
/* DECLARATIONS ----------------------------------------------- */
void CRLF(void) {UART_send(UART2,"\n\r");}

/* GLOBAL VARIABLE -------------------------------------------- */
KEY klawisz1;		// struktura inicjalizujaca klawisz

uint8_t a;
void key_event(void) {a++;}

/* MAIN FUNCTION ---------------------------------------------- */
int main(void)
{

	/* INITIALIZATION */
	SysTick_init(1000);
	UART2_init(57600);
	RFID_init();

	// inicjalizujemy klawisz
	key_init(&klawisz1,PORTC,PTC,3,100);

	// wyslamy stringa
	UART_send(UART2,"RFID testy ");
	CRLF();

	


	/* INFINITE LOOP */
	while(1)
	{
		// Klikajac przycisk zmieniamy tryb pracy: dodawanie do bazy/pokazywanie bazy/ usuwanie z bazy
		if( !(a%3) )
		{
			if(!sec3)
			{
				// co 150ms wywolujemy funkcje dodajca do bazy 
				sec3=150;
				if(RFID_add_card_to_database()==2)
				{
					UART_send(UART2,"Zapisano karte w bazie danych ");
					CRLF();
				}
			}
		}
		if( (a%3)==1) 
		{
		 if(!sec5)
		 {
			 			 uint8_t a;
			uint8_t i,x;
			sec5=5000;

			 UART_send(UART2,"Zawartosc bazy danych \r\n"); 
			 
			 a=RFID_show_database(tab);
			 for(i=0;i<16;i++)
			 {
				 for(x=0;x<5;x++)
				 {
					 UART_send_int(UART2,tab[i][x]);
				 }
				 CRLF();
			 }
			 UART_send(UART2,"Ilosc zapisanych kart:  "); 
					 UART_send_int(UART2,a);	
			CRLF();			 
		 }		
		
		}
		if( (a%3)==2 )
		{

			if(!sec2)
			{
				sec2=150;
				if(RFID_delete_card_from_database()) {UART_send(UART2,"Usunieto z bazy karte");			CRLF();}
			}

		}
			
		
		

		
		
	} // end of while(1)
	
	
}  // end of main() 




/* INTERRUPTS HANDLERS ---------------------------------------- */
void SysTick_Handler(void)
{
	uint16_t x;
	debounce_1khz(&klawisz1,key_event,key_event);   // drgania styków auto repetycja klawisza
	

	x=sec1;
	if(x) sec1=--x;
	x=sec5;
	if(x) sec5=--x;	
	x=sec2;
	if(x) sec2=--x;	
	x=sec3;
	if(x) sec3=--x;	
	x=soft_timer;
	if(x) soft_timer=--x;		

}


/*

*/

