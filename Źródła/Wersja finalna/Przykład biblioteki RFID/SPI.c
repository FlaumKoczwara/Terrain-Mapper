


#include "MKL46Z4.h"
#include "SPI.h"
#include "RFID.h"

/**
* \brief Inicjalizuje sprzętowy moduł SPI0.
*/
void SPI_init(void)
{
	/* WŁĄCZENIE TAKTOWANIA */
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK
						 |  SIM_SCGC5_PORTB_MASK
						 |  SIM_SCGC5_PORTC_MASK
						 |  SIM_SCGC5_PORTD_MASK
						 |  SIM_SCGC5_PORTE_MASK;
						 
	SIM->SCGC4 |= SIM_SCGC4_SPI0_MASK;
	
	/* KONFIGURACJA PINÓW */ 
	MISO_PORTx->PCR[MISO_NUMBER] |= PORT_PCR_MUX(MISO_MUX);
	MOSI_PORTx->PCR[MOSI_NUMBER] |= PORT_PCR_MUX(MOSI_MUX);
	SCK_PORTx->PCR[SCK_NUMBER] |= PORT_PCR_MUX(SCK_MUX);	
	SS_PORTx->PCR[SS_NUMBER] |= PORT_PCR_MUX(SS_MUX);
	SS_PTx->PDDR |= (1<<SS_NUMBER);
	
	/* KONFIGURACJA SPI */
	SPI0->C1 |= SPI_C1_MSTR_MASK;								// SPI w trybie master 
	SPI0->C1 &= ~SPI_C1_CPHA_MASK;
	SPI0->BR = SPI_BR_SPPR(2) | SPI_BR_SPR(3);	// baud rate == 500[kbps]				
	
	/* WŁĄCZENIE SPI */
	SPI0->C1 |= SPI_C1_SPE_MASK;
					 
}

/**
* \brief Dokonuje transferu jednego bajtu na magistrali SPI.
*
* \param uint8_t data - bajt do wysłania
* \returns Zwraca bajt odebrany od urządzenia
*
* Wysłanie jednego bajtu jest równoważne z odbraniem bajtu nadawanego z urzadzenia SLAVE
*/
uint8_t SPI_transmit(uint8_t data)
{
	uint8_t data_temp=0;
	data_temp=SPI0->S;
	SPI0->DL=data;
	while(!(SPI0->S & SPI_S_SPRF_MASK));
	data_temp=SPI0->S;
	data_temp=SPI0->DL;
	return data_temp;
	
}


