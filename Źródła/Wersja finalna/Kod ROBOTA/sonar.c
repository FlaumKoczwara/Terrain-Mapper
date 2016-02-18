/*
			sonar.c
			
			Inicjalizacja modułów , oraz funkcje sterujące sonarem
*/


#include "MKL46Z4.h"
#include "sonar.h"


volatile uint32_t left_distance;
volatile uint32_t right_distance;

void PIT_init(void)
{
	// jest taktowany z BUS CLOCK a zatem wynosi u nas 24[MHz]
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;		// taktowanie PIT'a
	PIT->MCR &= ~(PIT_MCR_MDIS_MASK);		// włączenie zegara (trzeba skasować bit)
	

	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;		// aktywacja przerwania
	PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK;			// kasacja flagi
	
	NVIC_ClearPendingIRQ(PIT_IRQn);
	NVIC_SetPriority(PIT_IRQn,2);
	NVIC_EnableIRQ(PIT_IRQn);
}

void SONAR_init(void)
{	

	PIT_init();
	/* KONFGURACJA PINU ECHO */
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK
						 |  SIM_SCGC5_PORTB_MASK
						 |  SIM_SCGC5_PORTC_MASK
						 |  SIM_SCGC5_PORTD_MASK
						 |  SIM_SCGC5_PORTE_MASK;
						 
	ECHO_LEFT_PORT->PCR[ECHO_LEFT_NUMBER] |= PORT_PCR_MUX(ECHO_LEFT_MUX);			// pin ECHO wejściem INPUT CAPTURE
	ECHO_LEFT_PORT->PCR[ECHO_LEFT_NUMBER] |= PORT_PCR_PE_MASK;			// pull up włączony
	ECHO_LEFT_PORT->PCR[ECHO_LEFT_NUMBER] |= PORT_PCR_PS_MASK;
	
	ECHO_RIGHT_PORT->PCR[ECHO_RIGHT_NUMBER] |= PORT_PCR_MUX(ECHO_RIGHT_MUX);			// pin ECHO wejściem INPUT CAPTURE
	ECHO_RIGHT_PORT->PCR[ECHO_RIGHT_NUMBER] |= PORT_PCR_PE_MASK;			// pull up włączony
	ECHO_RIGHT_PORT->PCR[ECHO_RIGHT_NUMBER] |= PORT_PCR_PS_MASK;
	
	TRIGGER_LEFT_PORT->PCR[TRIGGER_LEFT_NUMBER] |= PORT_PCR_MUX(1);				
	TRIGGER_LEFT_GPIO->PDDR |= (1<<TRIGGER_LEFT_NUMBER);
	
	TRIGGER_RIGHT_PORT->PCR[TRIGGER_RIGHT_NUMBER] |= PORT_PCR_MUX(1);				
	TRIGGER_RIGHT_GPIO->PDDR |= (1<<TRIGGER_RIGHT_NUMBER);


	/* USTAWIENIE TAKTOWANIA TIMERA */
	MCG->C1 |= MCG_C1_IRCLKEN_MASK;								// włączenie taktowania magistrali MCGIRCLK																	
	MCG->C2 |= MCG_C2_IRCS_MASK;									// taktujemy z wewnętrzenego oscylatora f=2[MHz]
	SIM->SOPT2 |=SIM_SOPT2_TPMSRC(3);						  // moduł timera jest taktowany z MCGIRCLK
	SIM->SCGC6 |=SIM_SCGC6_TPM2_MASK;							// włączenie taktowania Timera0
	
	/* INCJALIZACJA TIMER0 */
	TPM2->SC |= TPM_SC_PS(1);											// f timera == 1[MHz]
	TPM2->SC |= TPM_SC_CMOD(1);										// włączenie timera
	
	
	TPM2->CNT=0;	
	TPM2->MOD=65534;															
	
	/* INICJALIZACJA INPUT CAPTURE */
	TPM2->CONTROLS[0].CnSC |= TPM_CnSC_ELSA_MASK
												 |  TPM_CnSC_ELSB_MASK;	// input capture na obu zboczach
	TPM2->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;	// kasowanie flagi przerwania
	TPM2->CONTROLS[0].CnSC |= TPM_CnSC_CHIE_MASK;	// włączenie przerwania
	
	TPM2->CONTROLS[1].CnSC |= TPM_CnSC_ELSA_MASK
												 |  TPM_CnSC_ELSB_MASK;	// input capture na obu zboczach
	TPM2->CONTROLS[1].CnSC |= TPM_CnSC_CHF_MASK;	// kasowanie flagi przerwania
	TPM2->CONTROLS[1].CnSC |= TPM_CnSC_CHIE_MASK;	// włączenie przerwania	

	/* AKTYWACJA PRZERWANIA */
	NVIC_ClearPendingIRQ(TPM2_IRQn);							// usunięcie oczekujących przerwań
	NVIC_SetPriority(TPM2_IRQn,2);								// ustwiamy priorytet na 2 
	NVIC_EnableIRQ(TPM2_IRQn);										// włączamy przerwanie 

}

/* INTERRUPT HANDLER */
void TPM2_IRQHandler(void)
{
	static uint32_t left_measure_time,right_measure_time;
	if( TPM2->STATUS & TPM_STATUS_CH0F_MASK)
	{
		TPM2->STATUS |= TPM_STATUS_CH0F_MASK;		// skasowanie flagi
		// wystąpiło przerwanie od jakiegoś tam zbocza 
		// aby sprawdzić jakie to było zbocze sprawdzimu stan jaki mamy teraz na naszym pinie
		
		if( ECHO_LEFT_GPIO->PDIR & (1<<ECHO_LEFT_NUMBER) )  	 // jeśli stan wysoki to znaczy że zbocze było narastające 
		{
			TPM2->CNT=0;											 // zeruje licznik
		}
		if( !(ECHO_LEFT_GPIO->PDIR & (1<<ECHO_LEFT_NUMBER) ) )  // jeśli stan niski to znaczy że mieliśmy zbocze opadające
		{
			left_measure_time= TPM2->CNT;						 // zbocze opadające czyli sprawdzamy jak długo czujnik wystawił czas
		}
	}

	if( TPM2->STATUS & TPM_STATUS_CH1F_MASK)
	{
		TPM2->STATUS |= TPM_STATUS_CH1F_MASK;		// skasowanie flagi
		// wystąpiło przerwanie od jakiegoś tam zbocza 
		// aby sprawdzić jakie to było zbocze sprawdzimu stan jaki mamy teraz na naszym pinie
		
		if( ECHO_RIGHT_GPIO->PDIR & (1<<ECHO_RIGHT_NUMBER) )  	 // jeśli stan wysoki to znaczy że zbocze było narastające 
		{
			TPM2->CNT=0;											 // zeruje licznik
		}
		if( !(ECHO_RIGHT_GPIO->PDIR & (1<<ECHO_RIGHT_NUMBER) ) )  // jeśli stan niski to znaczy że mieliśmy zbocze opadające
		{
			right_measure_time= TPM2->CNT;						 // zbocze opadające czyli sprawdzamy jak długo czujnik wystawił czas
		}
	}
	/* przeliczanie odległości, podawana jest w milimetrach */
	left_distance=(left_measure_time*170)/1000;
	right_distance=(right_measure_time*170)/1000;

}

void PIT_IRQHandler(void)
{
	PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK; 		// kasacja flagi
	RIGHT_TRIGGER_LOW;														// stan niski na linii trigger
	LEFT_TRIGGER_LOW;
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;	// wyłączenie timera	
}


void SONAR_trigger(void)
{
	static uint8_t state;
	LEFT_TRIGGER_LOW;
	RIGHT_TRIGGER_LOW;
	


	PIT->CHANNEL[0].LDVAL=239;
	if(!state) {LEFT_TRIGGER_HIGH; state=1;}else {RIGHT_TRIGGER_HIGH;state=0;}
	PIT->CHANNEL[0].TCTRL |=PIT_TCTRL_TEN_MASK;	

}

uint32_t SONAR_left_distance(void)
{
	return left_distance;
}
uint32_t SONAR_right_distance(void)
{
	return right_distance;
}
