#include "MKL46Z4.h" 
#include "IR.h"
#include "math.h"

///////////////////////////// Sensor initialization ///////////////////////////// 
void IRInitialize (uint32_t IRn){			/*! Sensor initialization function. Bit x of the IRn[2:0] initializes sensor number x, IR0-PTE20, IR1- PTE16, IR2-PTE18*/
	int i;
	for (i=0 ; i<3; i++){
		if ((IRn&IR_MASK(i))>0)		/*! \param IRn Decides whitch sensors to initialize, if Bit x of this number is set, IRx will be initialized.  */
		IR[i]=1;									// The IR array is filled with information about the sensors activity
		SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;			// Enable Clock to Port E 
	}
	if (IR[0]==1){
		PORTE->PCR[20] = PORT_PCR_MUX(0u);			// Setting pins function to ADC 
		ADCInitialize (0,1); 										// Initialization of ADC, channel - IRx, resolution - 12 bit	
	}	
	if (IR[1]==1){
		PORTE->PCR[16] = PORT_PCR_MUX(0u);
		ADCInitialize (1,1);			
	}
	if (IR[2]==1){
		PORTE->PCR[18] = PORT_PCR_MUX(0u);
		ADCInitialize (2,1);			
	}
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;				// ADC interupts enabled
}

///////////////////////////// Single meeasurement ///////////////////////////// 
uint32_t Get_distance (uint32_t IRn){			/*! Function that starts a single measurement on IRn and returns a distance, if executed on a channel running in continuous mode, will set the mode to single. If executed in an interrupt, the interrupt must have priority lower than one.*/
	int i,b=0;
								/*! \returns Measured distance multiplied by 100  */
	Mode[IRn]=0;	/*! \param IRn ID of the sensor that should perform a mesurement  */
	Flag_conv[IRn]=1;			// Sygnalize a conversion in progres
	Flag_next[IRn]=1;			// Sygnalize that the sensor wants to convert
	for (i=0 ; i<3; i++){
		b += Mode[i];
	}
	if ( b==0){
		Flag_curr[IRn]=1;
		ADC0->SC1[0] = ADC_SC1_ADCH(IRn)|ADC_SC1_AIEN_MASK;		// Starting the conversion 
	}
	while(Flag_conv[IRn]==1){			// Wait for the completion of the conversion
		
	}
	if(Flag_next[(IRN+1)%3]==1){	//Find the next sensor that wants to measure, start next conversion and give it the Flag_curr
		Flag_curr[(IRN+1)%3]=1;
		ADC0->SC1[0] = ADC_SC1_ADCH((IRN+1)%3)|ADC_SC1_AIEN_MASK;	
	}
	else if(Flag_next[(IRN+2)%3]==1){
		Flag_curr[(IRN+2)%3]=1;
		ADC0->SC1[0] = ADC_SC1_ADCH((IRN+2)%3)|ADC_SC1_AIEN_MASK;
	}
	else if(Flag_next[IRN]==1){
		Flag_curr[IRN]=1;
		ADC0->SC1[0] = ADC_SC1_ADCH(IRN)|ADC_SC1_AIEN_MASK;
	}
	
	return(Distance[IRn]);	// Return the distance stored in the global variable by the interrupt
}

///////////////////////////// ADC Interrupt Handler ///////////////////////////// 
void ADC0_IRQHandler (void) { /*! ADC Interrupt Handler, converts the measurements to distance, avarages them, and limits the output to 50cm. Controlls the ADC acces algorythm.
	In single mode, singal that measurement is complete. In continuous mode, execute the constant measure procedure and initiate next conversion. */
	
	int IRn,i;
	for (IRn=0; IRn<3;IRn++){	// Check all sensors
		if(((IR[IRn])&&(Flag_curr[IRn]))){		// If the sensor is active and was given the permission to measure
			IRN=IRn;						// Store current IRn
			if(inc<N) {			// When the buffer is not full
				int tmp = ((ADC0->R[0]*3*1000)/4096);
				tmp= 1086000/(tmp-40);
				tab[inc] = tmp;				// Get the output from ADC, convert it to distance and put it in the buffer
				inc++;
				ADC0->SC1[0] = ADC_SC1_ADCH(IRn)|ADC_SC1_AIEN_MASK;			// Initiate next conversion 
			}
			if (inc==N) {			// When the buffer is full 
				for(i =0; i<N; i++)
					Distance[IRn]+=tab[i];
				Distance[IRn]/=N;			// Avarage the measurements 
				if (Distance[IRn] >5000)
					Distance[IRn]=9999;			// If outside of the sensor range, display 99.99 
				if (Mode[IRn]==0){			// In single mode, singal that measurement is complete, clear all the flags
					
					inc=0;
					Flag_conv[IRn]=0;
					Flag_curr[IRn]=0;
					Flag_next[IRn]=0;
				}
				else if (Mode[IRn]==1) {			// In continuous mode, execute the constant measure procedure, transfer the flags and initiate next conversion
					inc=0;
					Constant_measure(IRn, Distance[IRn]);
					Flag_curr[IRn]=0;
					if(Flag_next[(IRN+1)%3]==1){				//Find the next sensor that wants to measure, start next conversion and give it the Flag_curr
						Flag_curr[(IRN+1)%3]=1;
						ADC0->SC1[0] = ADC_SC1_ADCH((IRN+1)%3)|ADC_SC1_AIEN_MASK;
					}
					else if(Flag_next[(IRN+2)%3]==1){
						Flag_curr[(IRN+2)%3]=1;
						ADC0->SC1[0] = ADC_SC1_ADCH((IRN+2)%3)|ADC_SC1_AIEN_MASK;
					}
					else if(Flag_next[IRN]==1){
						Flag_curr[IRN]=1;
						ADC0->SC1[0] = ADC_SC1_ADCH(IRN)|ADC_SC1_AIEN_MASK;
					}
				}
			}		
		}
	}
	ADC0->R[0]; // Clear the COCO by reading adc value.
}

///////////////////////////// Constant measure procedure ///////////////////////////// 
__weak void Constant_measure (uint32_t IRn, uint32_t Dist)	 {			/*! Routine done after every measurement in continuus mode, can be redefined by user thank to the " _weak" keyword. Current distance and IRn are given to this function. */
	/*! \param IRn ID of the sensor that performet the measurement  */
	/*! \param Dist Measured distance myltiplied by 100 */
}

void Start_cont (uint32_t IRn ) {			/*! Start continuous measurement on a selected sensor */
	int i,b=0;	/*! \param IRn ID of the sensor  */
	Flag_next[IRn]=1;
	Mode[IRn]=1;
	for (i=0 ; i<3; i++)
		b += Mode[i];
	if ( b==1){
		ADC0->SC1[0] = ADC_SC1_ADCH(IRn)|ADC_SC1_AIEN_MASK;		// Starting the conversion 
		Flag_curr[IRn]=1;
	}
}


void Stop_cont (uint32_t IRn) {			/*! Stop continuous measurement on a selected sensor */
	Mode[IRn]=0;	/*! \param IRn ID of the sensor  */
	Flag_next[IRn]=0;
}

///////////////////////////// ADC initialization ///////////////////////////// 
static void ADCInitialize(uint32_t channel,uint32_t resolution){ /*! Not avaliable to the user. Initialization of the ADC, takes channel and resolution as arguments. Resolution :0-8bit, 1-12bit, 2-10bit, 3-16 bit*/
	SIM->SCGC6 |=  SIM_SCGC6_ADC0_MASK; 		// Enable clock for ADC
	ADC0->CFG1 |= ADC_CFG1_ADICLK(1)|ADC_CFG1_ADIV(2);		
	ADC0->CFG2 |= ADC_CFG2_ADHSC_MASK|ADC_CFG2_ADLSTS(1); 
	ADC0->SC2 = 0;
	ADC0->SC3 |= ADC_SC3_AVGE_MASK|ADC_SC3_AVGS(3)|ADC_SC3_CAL_MASK; // Set avaraging options and run calibration
	while(ADC0->SC3&ADC_SC3_CAL_MASK) {	// Wait till calibration over
	
	}
	if(ADC0->SC3&ADC_SC3_CALF_MASK) {
		ADC0->SC3 |= ADC_SC3_CAL_MASK;
		while(ADC0->SC3&ADC_SC3_CAL_MASK) {	// If calibration failed calibrate again
	
		}
	}
	ADC0->PG=((ADC0->CLP0+ADC0->CLP1+ADC0->CLP2+ADC0->CLP3+ADC0->CLP4+ADC0->CLPS+ADC0->CLPD)/2)|(1u<<15);
	ADC0->MG=((ADC0->CLM0+ADC0->CLM1+ADC0->CLM2+ADC0->CLM3+ADC0->CLM4+ADC0->CLMS+ADC0->CLMD)/2)|(1u<<15); // Move the values received in the calibration to proper registers
	ADC0->CFG1 |= ADC_CFG1_ADICLK(1)|ADC_CFG1_ADIV(3)|ADC_CFG1_MODE(resolution); // Set clock(1.5Mhz) and resolution 
	ADC0->CFG1 |= ADC_CFG1_ADLSMP_MASK;	// Enable long sampling time
	NVIC_ClearPendingIRQ(ADC0_IRQn);		// Enable interrupt
	NVIC_EnableIRQ(ADC0_IRQn);
	NVIC_SetPriority (ADC0_IRQn, 1);
	ADC0->SC1[0] &= ADC_SC1_DIFF_MASK;
	ADC0->SC1[0] = ADC_SC1_ADCH(1111); // Disable the ADC, wait for it to be started by user
	
}	
