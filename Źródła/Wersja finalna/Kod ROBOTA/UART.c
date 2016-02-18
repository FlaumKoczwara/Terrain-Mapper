/*
		Temat: Biblioteka do zaawansowanej obsługi UART ( 1,2 ) w przerwaniach, z możliwością wysyłania/odbierania tablic znakowych
		Wersja: 0.2
		Autor: Juffre
		Data: 19.8.2015
		Plik: UART.c
		
*/


/* BASIC HEADERS --------------------------------------------------------------------------------------- */
#include "MKL46Z4.h"
#include "string.h"
#include "UART.h"


/* DECLARATIONS OF INTERNAL FUNCTIONS ------------------------------------------------------------------ */
void UART_GPIO_init(PORT_Type *port,uint8_t number,uint8_t mux);
void int2string(int n, char s[]);

/* DEFINITIONS ----------------------------------------------------------------------------------------- */
#define BUS_CLOCK 24000000u

#define UART1_ENABLE_TX 	UART1->C2 |= UART_C2_TE_MASK;			
#define UART1_DISABLE_TX 	UART1->C2 &= ~UART_C2_TE_MASK;		
#define UART1_ENABLE_RX 	UART1->C2 |= UART_C2_RE_MASK;			
#define UART1_DISABLE_RX 	UART1->C2 &= ~UART_C2_RE_MASK;

#define UART2_ENABLE_TX 	UART2->C2 |= UART_C2_TE_MASK;			
#define UART2_DISABLE_TX 	UART2->C2 &= ~UART_C2_TE_MASK;		
#define UART2_ENABLE_RX 	UART2->C2 |= UART_C2_RE_MASK;			
#define UART2_DISABLE_RX 	UART2->C2 &= ~UART_C2_RE_MASK;

/* VARIABLES ------------------------------------------------------------------------------------------- */
volatile char UART1_TX_bufor[TX_BUFOR_SIZE];
volatile char UART1_RX_bufor[RX_BUFOR_SIZE];
volatile uint8_t UART1_head;
volatile uint8_t UART1_tail;
volatile uint8_t UART1_receive_flag;

volatile char UART2_TX_bufor[TX_BUFOR_SIZE];
volatile char UART2_RX_bufor[RX_BUFOR_SIZE];
volatile uint8_t UART2_head;
volatile uint8_t UART2_tail;
volatile uint8_t UART2_receive_flag;



/* BODY OF FUNCTIONS (EXTERNAL) ------------------------------------------------------------------------ */
void UART1_init(uint32_t BAUD_RATE)
{
	uint16_t sbr=0; 															// auxiliary variable to store SBR register value
	
	/* PORT CONFIG */
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK						// clock gating for UART module pins
						 |  SIM_SCGC5_PORTB_MASK
						 |  SIM_SCGC5_PORTC_MASK
						 |  SIM_SCGC5_PORTD_MASK
						 |  SIM_SCGC5_PORTE_MASK;
	
	SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;						// clock gating for UART module
	
	UART_GPIO_init(UART1_TX);
	UART_GPIO_init(UART1_RX);
	
	/* SBR */
	sbr = (BUS_CLOCK)/(16*BAUD_RATE);
	
	/* UART MODULE CONFIG */
	UART1->BDH |= UART_BDH_SBR(sbr>>8);						// writing to higher register 
	UART1->BDL = UART_BDL_SBR(sbr);								// writing to lower register

	UART1->C2 |= UART_C2_TIE_MASK;								// enabling transmit interrupt									
	UART1->C2 |= UART_C2_RIE_MASK;								// enabling receiver interrupt
	UART1_ENABLE_TX;															// enabling transmiter
	UART1->C2 |= UART_C2_RE_MASK;									// enabling receiver
	
	/* INTERRUPT CONFIG */
	NVIC_ClearPendingIRQ(UART1_IRQn);
	NVIC_SetPriority(UART1_IRQn,3);
	NVIC_EnableIRQ(UART1_IRQn);
}

void UART2_init(uint32_t BAUD_RATE)
{
	uint16_t sbr=0; 															// auxiliary variable to store SBR register value
	
	/* PORT CONFIG */
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK						// clock gating for UART module pins
						 |  SIM_SCGC5_PORTB_MASK
						 |  SIM_SCGC5_PORTC_MASK
						 |  SIM_SCGC5_PORTD_MASK
						 |  SIM_SCGC5_PORTE_MASK;
	
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;						// clock gating for UART module
	
	UART_GPIO_init(UART2_TX);
	UART_GPIO_init(UART2_RX);
	
	/* SBR */
	sbr = (BUS_CLOCK)/(16*BAUD_RATE);
	
	/* UART MODULE CONFIG */
	UART2->BDH |= UART_BDH_SBR(sbr>>8);						// writing to higher register 
	UART2->BDL = UART_BDL_SBR(sbr);								// writing to lower register

	UART2->C2 |= UART_C2_TIE_MASK;								// enabling transmit interrupt									
	UART2->C2 |= UART_C2_RIE_MASK;								// enabling receiver interrupt
	UART2_ENABLE_TX;															// enabling transmiter
	UART2->C2 |= UART_C2_RE_MASK;									// enabling receiver
	
	/* INTERRUPT CONFIG */
	NVIC_ClearPendingIRQ(UART2_IRQn);
	NVIC_SetPriority(UART2_IRQn,3);
	NVIC_EnableIRQ(UART2_IRQn);
}

void UART_send(UART_Type *UARTx,char *string)
{
	// copy data to right TX bufor
	while( *string ) 
	{
		if(UARTx == UART1) { UART1_TX_bufor[UART1_tail++]=*string; string++;}
		if(UARTx == UART2) { UART2_TX_bufor[UART2_tail++]=*string; string++;}	
	}
	// if TX bufor is empty -> start transmission
	if(UARTx == UART1) 
	{
		if( UART1_head == 0 ) {UART1_ENABLE_TX; UARTx->D = UART1_TX_bufor[UART1_head++];}
	}
	if(UARTx == UART2) 
	{
		if( UART2_head == 0 ) {UART2_ENABLE_TX; UARTx->D = UART2_TX_bufor[UART2_head++];}
	}
	
}




/* FUNCTION WHICH IS RECEIVING DATA */
void UART_get(UART_Type *UARTx, char *string )
{
	uint8_t i;
	if( UARTx == UART1)
	{
			if(UART1_receive_flag==1)														// if we received all frame 								
			{
				UART1_receive_flag=0;															// clear flag
		
				for(i=0;i<RX_BUFOR_SIZE-1;i++)										// copy data from UART bufor to the user array 
				{
					if( UART1_RX_bufor[i] !='\r') *(string++) = UART1_RX_bufor[i];
					else {*string='\0';break;}											// NULL charakter end copying
				}
			}
	}
		if( UARTx == UART2)
	{
			if(UART2_receive_flag==1)														// if we received all frame 								
			{
				UART2_receive_flag=0;															// clear flag
		
				for(i=0;i<RX_BUFOR_SIZE-1;i++)										// copy data from UART bufor to the user array 
				{
					if( UART2_RX_bufor[i] !='\r') *(string++) = UART2_RX_bufor[i];
					else {*string='\0';break;}											// NULL character end copying
				}
			}
	}

}

void UART_send_int(UART_Type *UARTx, int count)
{
	char bufor[11];
	int2string(count,bufor);
	UART_send(UARTx,bufor);
}
	
	
/* INTERNAL FUNCTION ------------------------------------------------------------------------------------ */
void UART_GPIO_init(PORT_Type *port,uint8_t number,uint8_t mux)
{
	port->PCR[number] |= PORT_PCR_MUX(mux);
}

void int2string(int n, char s[]) 
{ 
   char i,c,j; 
   int sign; 
   if ((sign = n) < 0) n = -n; 
   i = 0; 
   do { 
      s[i++] = n % 10 + '0'; 
   } while ((n /= 10) > 0); 
   if (sign < 0) 
      s[i++] = '-'; 
   s[i] = 0; 
   for (i = 0, j = strlen(s)-1; i < j; i++, j--) { 
      c = s[i]; 
      s[i] = s[j]; 
      s[j] = c; 
   } 
} 




/* INTERRUPT HANDLER ------------------------------------------------------------------------------------ */
void UART1_IRQHandler(void)
{
	// empty bufor interrupt
	if( UART1->S1 & UART_S1_TDRE_MASK)
	{
		if( !UART1_tail ) {UART1_DISABLE_TX;}  																					   // all data has been send, TX bufor empty
		if( UART1_head < UART1_tail-1) {	UART1->D = UART1_TX_bufor[UART1_head++]; }			 // send next byte from TX bufor
			else {                                                                           // send last byte
						UART1->D = UART1_TX_bufor[UART1_head];
						// bufor is empty, clear pointers
						UART1_head=0;
						UART1_tail=0;
					 }				
	}
	
	// receive interrupt
	if( UART1->S1 & UART_S1_RDRF_MASK)
	{
		static uint8_t cnt=0;																							// count next byte
		if( UART1->D == '\r')																							// CR indicates end of received frame 
		{
			UART1_RX_bufor[cnt]= '\0';																			// push NULL at the end of received table 
			cnt=0;																													// clear counter
			UART1_receive_flag=1;																						// set received flag
		} 	
		else if( UART1->D != '\r' ) {UART1_RX_bufor[cnt++] = UART1->D;}		// push next byte into the array
	}
}

void UART2_IRQHandler(void)
{
	// empty bufor interrupt
	if( UART2->S1 & UART_S1_TDRE_MASK)
	{
		if( !UART2_tail ) {UART2_DISABLE_TX;}																							// all data has been send, TX bufor empty
		if( UART2_head < UART2_tail-1) {	UART2->D = UART2_TX_bufor[UART2_head++]; }			// send next byte from TX bufor
			else {																																					// send last byte
						UART2->D = UART2_TX_bufor[UART2_head];
						// bufor is empty, clear pointers
						UART2_head=0;		
						UART2_tail=0;
					 }				
	}
	
	// receive interrupt
	if( UART2->S1 & UART_S1_RDRF_MASK)
	{
		static uint8_t cnt=0;																							// count next byte
		if( UART2->D == '\r')																							// CR indicates end of received frame 
		{
			UART2_RX_bufor[cnt]= '\0';																			// push NULL at the end of received table 
			cnt=0;																													// clear counter
			UART2_receive_flag=1;																						// set received flag
		} 	
		else if( UART2->D != '\r' ) {UART2_RX_bufor[cnt++] = UART2->D;}		// push next byte into the array
	}
}


