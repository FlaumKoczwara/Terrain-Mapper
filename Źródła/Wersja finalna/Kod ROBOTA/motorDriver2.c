#include "motorDriver2.h"


#define PRIORITY_INTERRUPT_DRIVE 1 
#define V_MOD 1023
#define LEFT (1u<<13)
#define RIGHT (1u<<9)

volatile uint32_t tmp1 = 0;
volatile uint32_t tmp2 = 0;
volatile uint8_t  tmp3 = 0;

volatile uint32_t ACTUAL_DISTANCE = 0; // srednia arytmetyczna LEFT i RIGHT
volatile uint32_t ACT_DIST_LEFT = 0;
volatile uint32_t ACT_DIST_RIGHT = 0;

volatile uint8_t distance;

uint8_t FLAG_LIMIT_DIST = 0; // nie uzywane
uint8_t FLAG_LIMIT_DIST_LEFT = 0;
uint8_t FLAG_LIMIT_DIST_RIGHT = 0;

uint32_t LIMIT = 0; // nie uzywane
uint32_t LIMIT_LEFT = 0;
uint32_t LIMIT_RIGHT = 0;

uint8_t FLAG_ENCODER = 0; // nie uzywane

// 1. Zastanowic sie czy dokladnosc 1cm wystarczy czy koniecznie trzeba 0.5 cm , bo wtedy trzeba wprowadzic zmienna double
// 2. Dlaczego zumo skreca samoczynnie w lewo
// 3. Slizganie sie
// 4. Kompas
// 5. 

void motorDriverInit(void){

	// CLOCK_SETUP 1
	// 1 ... Multipurpose Clock Generator (MCG) in PLL Engaged External (PEE) mode
  //       Reference clock source for MCG module is an external crystal 8MHz
  //       Core clock = 48MHz, BusClock = 24MHz
	
	//
	SIM -> SCGC5 |= SIM_SCGC5_PORTA_MASK // 
	              | SIM_SCGC5_PORTC_MASK 
	              | SIM_SCGC5_PORTD_MASK;
	
	//
	SIM -> SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	//
	PORTA ->PCR[6] |= PORT_PCR_MUX(3); // TPM0_CH3 - encoder
	PORTA ->PCR[13] |= PORT_PCR_MUX(1); // PHASE - Left
	PORTC ->PCR[9] |= PORT_PCR_MUX(1); // PHASE - Right
	PORTD ->PCR[2] |= PORT_PCR_MUX(4); // TPM0_CH2 - PWM - Right
	PORTD ->PCR[4] |= PORT_PCR_MUX(4); // TPM0_CH4 - PWM - Left
	PORTD ->PCR[5] |= PORT_PCR_MUX(4); //TPM0_CH5 - encoder / to tez dioda zielona
	
	// OUTPUT pin
	PTA->PDDR |= (1u<<13);
	PTC->PDDR |= (1u<<9);
	
	
	
	////////////////////// PWM /////////////////////////////////
	
	
	//select source reference TMP0

	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // ?set 'MCGFLLCLK clock or MCGPLLCLK/2'
	
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;// set "MCGPLLCLK clock with  fixed divide by two"
	
	// set "up-counting"
	TPM0->SC &= ~TPM_SC_CPWMS_MASK; // default set
	
	// divide by 1
	TPM0->SC &= ~TPM_SC_PS_MASK; // the same TPM_SC_PS(0)
	
	// clear counter
	TPM0->CNT = 0x00; 
	
	// set MOD for PWM period equal 1023 ( 10 bit)
	TPM0->MOD = V_MOD;
	
	//////////CHANNEL ENGINE ////////////////////////////////
	//Right engine
	// set TPM0 channel 2 - "Edge-aligned PWM High-true pulses"
	TPM0->CONTROLS[2].CnSC |= TPM_CnSC_MSB_MASK |	
													  TPM_CnSC_ELSB_MASK;
	// Default value for Right engine
	TPM0->CONTROLS[2].CnV = 0; // STOP
	
	//Left engine
	// set TPM0 channel 4 - "Edge-aligned PWM High-true pulses"
	TPM0->CONTROLS[4].CnSC |= TPM_CnSC_MSB_MASK |	
													  TPM_CnSC_ELSB_MASK;
	// Default value for Left engine
	TPM0->CONTROLS[4].CnV = 0; // STOP
	
	//////////CHANNEL ENCODER////////////////////////////////
	//Encoder for Left Engine
	// set TPM0 channel 3 - "Edge-aligned PWM High-true pulses"
	TPM0->CONTROLS[3].CnSC |= TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK ;	// licze oba zbocza
													 
	// enable interrupt for channel 3
	TPM0->CONTROLS[3].CnSC |= TPM_CnSC_CHIE_MASK;
	
	//Encoder for Right Engine
	// set TPM0 channel 3 - "Edge-aligned PWM High-true pulses"
	TPM0->CONTROLS[5].CnSC |= TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK ; // licze oba zbocza 
													 												
														
	// enable interrupt for channel 5
	TPM0->CONTROLS[5].CnSC |= TPM_CnSC_CHIE_MASK; 
		
	
	NVIC_ClearPendingIRQ(TPM0_IRQn);				/* Clear NVIC any pending interrupts on TPM0 */
	NVIC_EnableIRQ(TPM0_IRQn);							/* Enable NVIC interrupts source for TPM0 module */
	
	NVIC_SetPriority (TPM0_IRQn, PRIORITY_INTERRUPT_DRIVE);	// priority interrupt
	
	// enable counter
	TPM0->SC |= TPM_SC_CMOD(1);
	
}

void TPM0_IRQHandler(void){
	
	/////////LEFT////////////
	if(FLAG_LIMIT_DIST_LEFT)
		{
		
		if( LIMIT_LEFT <= ACT_DIST_LEFT)
		{
			driveStopLeft();
			FLAG_LIMIT_DIST_LEFT = 0;
		}
	}
		
	////////////RIGHT////////////
		if(FLAG_LIMIT_DIST_RIGHT)
		{
		
		if( LIMIT_RIGHT <= ACT_DIST_RIGHT)
		{
			driveStopRight();
			FLAG_LIMIT_DIST_RIGHT = 0;
		}
	}
	////////GENERAL///////////// // NIE UZYWAM TEGO
	if(FLAG_LIMIT_DIST)
		{
		
		if( LIMIT <= ACTUAL_DISTANCE)
		{
			driveStop();
			FLAG_LIMIT_DIST = 0;
		}
	}
	//////////////////////////////////////////////////	
		
 // Left
	if(TPM0->CONTROLS[3].CnSC & TPM_CnSC_CHF_MASK){
	
	   ACT_DIST_LEFT++;
		 distance++;
	}
	
	// Right
	if(TPM0->CONTROLS[5].CnSC & TPM_CnSC_CHF_MASK){
		
			ACT_DIST_RIGHT++;
		}
	
		ACTUAL_DISTANCE = (ACT_DIST_LEFT + ACT_DIST_RIGHT)/2 ;
		
	// ustawiajac tam jedynke na Flage, tak naprawde czyscimy ja
	TPM0->CONTROLS[3].CnSC |= TPM_CnSC_CHF_MASK;
	//TPM0 ->STATUS |= TPM_STATUS_CH3F_MASK;
	
  TPM0->CONTROLS[5].CnSC |= TPM_CnSC_CHF_MASK;
	//TPM0 ->STATUS |= TPM_STATUS_CH5F_MASK;
	
}

void driveForwardLeftTrack( int predkosc){
	
	ACT_DIST_LEFT = 0;
	
	if( predkosc >=0){
		
	PTA->PCOR = LEFT; // clear , set 0 mean forward
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = V_MOD * predkosc/100;
	
         }
	}
	
}

void driveForwardDistLeftTrack( int predkosc , int droga){
	
	ACT_DIST_LEFT= 0;
	
	if( predkosc >=0){
		
	FLAG_LIMIT_DIST_LEFT = 1;
	LIMIT_LEFT = droga; // info dla enkodera	
		
	PTA->PCOR = LEFT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = V_MOD * predkosc/100;
	
         }
				
		
				 
				 
	}
		
}

void driveForwardRightTrack( int predkosc){
	
	ACT_DIST_RIGHT = 0;
	
	if( predkosc >=0){
		
	PTC->PCOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = V_MOD * predkosc/100;
	
         }
	}
	
}

void driveForwardDistRightTrack( int predkosc , int droga){
	
	ACT_DIST_RIGHT = 0;
	
	if( predkosc >=0){
		
	FLAG_LIMIT_DIST_RIGHT = 1;				 
	LIMIT_RIGHT = droga; // info dla enkodera
				 	
	PTC->PCOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = V_MOD * predkosc/100;
	
         }
		
    
				 
	}
		
}

void driveStopLeft(void){
	
	
	TPM0->CONTROLS[4].CnV = 0; // stop LEFT
	
	
}
void driveStopRight(void){
	
	
	TPM0->CONTROLS[2].CnV = 0; // stop RIGHT
		
	
}

void driveStop(void){
	
	
	TPM0->CONTROLS[2].CnV = 0; // stop RIGHT
	TPM0->CONTROLS[4].CnV = 0; // stop LEFT
	
	
}


void driveReverseLeftTrack( int predkosc){
	
	ACT_DIST_LEFT = 0;
	
	if( predkosc >=0){
		
	PTA->PSOR = LEFT; // clear , set 0 mean forward
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = V_MOD * predkosc/100;
	
         }
	}
	
}

void driveReverseDistLeftTrack( int predkosc , int droga){
	
	ACT_DIST_LEFT = 0;
	
	if( predkosc >=0){
		
	  FLAG_LIMIT_DIST_LEFT = 1;				 
		LIMIT_LEFT = droga; // info dla enkodera
				 
	PTA->PSOR = LEFT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = V_MOD * predkosc/100;
	
         }
		

				 
	}
		
}

void driveReverseRightTrack( int predkosc){
	
	ACT_DIST_RIGHT = 0;
	
	if( predkosc >=0){
		
	PTC->PSOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = V_MOD * predkosc/100;
	
         }
	}
	
}

void driveReverseDistRightTrack( int predkosc , int droga){
	
	ACT_DIST_RIGHT = 0;
	
	if( predkosc >=0){
		
	FLAG_LIMIT_DIST_RIGHT = 1;		
	LIMIT_RIGHT = droga; // info dla enkodera
					
	PTC->PSOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = V_MOD * predkosc/100;
	
         }
				
		 
				 
	}
		
}


void driveForward(int predkosc){
	driveForwardLeftTrack(predkosc);
	driveForwardRightTrack(predkosc);

}

void driveForwardDist( int predkosc , int droga){
	
	
	driveForwardDistLeftTrack( predkosc , droga);
	driveForwardDistRightTrack( predkosc , droga);
	
}


void driveReverse(int predkosc){
	
	driveReverseLeftTrack(predkosc);
	driveReverseRightTrack(predkosc);
	
	
	
}

void driveReverseDist( int predkosc , int droga){
	
	driveReverseDistLeftTrack( predkosc , droga);
	driveReverseDistRightTrack( predkosc , droga);
	
	
}

void acceleration( int oile){
		
	//Left ENGINE
	tmp1 = ((TPM0->CONTROLS[4].CnV * 100) / V_MOD ) ;
	
	//Right ENGINE
	tmp2 = ((TPM0->CONTROLS[2].CnV * 100 )/ V_MOD) ;
	
	for( tmp3 =0; tmp3 < oile ; tmp3++){
		
	      if((tmp1  + oile) >= 100){
		
		    TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
				if((tmp2 + oile )>=100 ){
					
					TPM0->CONTROLS[2].CnV = V_MOD + 1;
				}
				if((tmp1 + oile) <= 0 ){
				
					TPM0->CONTROLS[4].CnV = 0;
				}
          if((tmp2 + oile) <= 0 ){
						
						TPM0->CONTROLS[2].CnV = 0;
					}
					
				if( (tmp1 + oile)>0 && (tmp1 + oile)<100){
		
	       TPM0->CONTROLS[4].CnV = ((tmp1 + tmp3)*V_MOD/100);
	
         }
				
				 if( (tmp2 + oile)>0 && (tmp2 + oile)<100){
		
	       TPM0->CONTROLS[2].CnV = ((tmp2 + tmp3)*V_MOD/100);
	
         }
	
				 wait4();
			 }
	
}

/////////////////////////////dorzucic sprawdzanie kompasem //////////////////////////////
void turnLeft( int kat ){
	
	
	tmp3 = kat/5 -2 ; ////// doswiadczalnie okreslilem ze jesli mu podam kat 90 , to obroci mi sie ladnie o 90 stopni
	
	driveReverseDistLeftTrack(25 , tmp3 ); // wydaje mi sie ze minimalna moc przy skrecaniu musi byc 40-50
	driveForwardDistRightTrack(25 , tmp3); // bo tam jest duze tarcie
	
	

}

void turnRight( int kat ){
	
	tmp3 = kat/5 -2 ;  ////// doswiadczalnie okreslilem ze jesli mu podam kat 90 , to obroci mi sie ladnie o 90 stopni
	
	driveReverseDistRightTrack(25 , tmp3 ); // wydaje mi sie ze minimalna moc przy skrecaniu musi byc 40-50 
	driveForwardDistLeftTrack(25 , tmp3 );  // bo tam jest duze tarcie

	
}

void wait4(void){
	
	int i = 0;
	//int j = 0;
	for( i=0 ; i<999990; i++){
	//for( j=0 ; j<5; j++);
		
	}
	
	
}

void wait2 (void){
	
	int i = 0;
	//int j = 0;
	for( i=0 ; i<999999; i++){
	//for( j=0 ; j<5; j++);
		
	}
	
	
}
void wait3 (void){
	
	int i = 0;
	//int j = 0;
	for( i=0 ; i<9999999; i++){
	//for( j=0 ; j<5; j++);
		
	}
	
	
}


