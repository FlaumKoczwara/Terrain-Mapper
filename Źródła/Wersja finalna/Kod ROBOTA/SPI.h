 
/* ------------------------------------------------------------------------------------------------------------------------------------------- *
SPI wiadomo o co kaman  
		
* -------------------------------------------------------------------------------------------------------------------------------------------- */
#ifndef _SPI_H_
#define _SPI_H_

#include "MKL46Z4.h" 


/* KONFIGURACJA ---------------------------------------------------------------------------------------------- */
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





/* FUNKCJE UŻYTKOWNIKA --------------------------------------------------------------------------------------- */
void SPI_init(void);
uint8_t SPI_transmit(uint8_t data);
#endif

