/*

			sonar.h
			

*/
#ifndef sonar_H_
#define sonar_H_

#include "MKL46Z4.h"



/* KONFIGRACJA WEJŚCIA  */

#define ECHO 12
#define ECHO_MASK (1<<12)

#define ECHO_LEFT_PORT PORTB
#define ECHO_LEFT_NUMBER 18
#define ECHO_LEFT_MUX 3
#define ECHO_LEFT_GPIO PTB

#define ECHO_RIGHT_PORT PORTB
#define ECHO_RIGHT_NUMBER 19
#define ECHO_RIGHT_MUX 3
#define ECHO_RIGHT_GPIO PTB

#define TRIGGER_LEFT_PORT PORTC
#define TRIGGER_LEFT_NUMBER 10
#define TRIGGER_LEFT_MUX 1
#define TRIGGER_LEFT_GPIO PTC

#define TRIGGER_RIGHT_PORT PORTC
#define TRIGGER_RIGHT_NUMBER 11
#define TRIGGER_RIGHT_MUX 1
#define TRIGGER_RIGHT_GPIO PTC

#define LEFT_TRIGGER_HIGH TRIGGER_LEFT_GPIO->PSOR |= (1<<TRIGGER_LEFT_NUMBER)
#define LEFT_TRIGGER_LOW TRIGGER_LEFT_GPIO->PCOR |= (1<<TRIGGER_LEFT_NUMBER)

#define RIGHT_TRIGGER_HIGH TRIGGER_RIGHT_GPIO->PSOR |= (1<<TRIGGER_RIGHT_NUMBER)
#define RIGHT_TRIGGER_LOW TRIGGER_RIGHT_GPIO->PCOR |= (1<<TRIGGER_RIGHT_NUMBER)






/* DEKLARACJE FUNKCJI */
void SONAR_init(void);			// inicjalizacja sonaru
void SONAR_trigger(void);		// wyzwolenie pomiaru
uint32_t SONAR_left_distance(void);
uint32_t SONAR_right_distance(void);


#endif




