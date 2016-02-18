#ifndef _MAIN_H_
#define _MAIN_H_
#include "MKL46Z4.h"

#include "SysTick.h"
#include "UART.h"
#include "IR.h"
#include "motorDriver2.h"
#include "RFID.h"
#include "SONAR.h"


/* DEFINICJE -------------------------------------------------- */
#define ENGINE 18  // w % mocy silników
#define DIVIDER 42
#define REGULATION_LIMIT 25


#define FRONT_WALL 0x01
#define LEFT_WALL 0x02
#define RIGHT_WALL 0x04

#define CELL_SIZE 20  // wielkość jednej komórki labiryntu podwana w [cm]
#define TURN_ANGLE 80 

/* ENUMERACJE ------------------------------------------------- */
enum sensors {LEFT,RIGHT,FRONT};		// enumeracja kierunków pomiaru odległości oraz w którym kierunku ma jechać robot
enum {check_wall,drive_forward,drive_forward_timer,turn_left,turn_left_timer,turn_right,turn_right_timer,stop,turn_around,turn_around_timer,get_closer,read_rfid,go_backward,print_wall}; 			// typ wyliczeniowy nazwyjacy stany maszyny stanów

/* GLOBAL VARIABLE -------------------------------------------- */
// software timer
volatile uint16_t timer;
volatile uint16_t timer2;
volatile uint16_t timer3;
volatile uint16_t rfid_timer;
volatile uint16_t motor_timer;

// zmienne trzymające odległości zmierzone przez czjniki 
uint32_t front_sensor;
uint32_t right_sensor;
uint32_t left_sensor;
uint32_t left_sonar;
uint32_t right_sonar;

int diff;
int diff_old;
int rozn;
int odl;  							 // odleglosc od sciany służąca kalbibracji ustawienia robota w labiryncie
uint8_t wall_position;   // trzyma rozkład ścian w aktualnej komórce

uint8_t start=0;  // zmienna pomocnicza
uint8_t dzielnik=DIVIDER;		// zmienna regulująca siłę regulatora P 




// received bufor
char command[256];  // bufor na odebrane dane do zmniejszenia i dopracowania 



/* DEKLARACJE FUNKCJI ----------------------------------------- */
void check_distance(uint16_t period); 		 // check distance and get new command from user 
void send_data_to_user(uint16_t period);	 // send chosen data to user through bluetooth 

uint8_t check_wall_position(void);				// sprawdzamy w jakiej odległości są ściany
void movement_algorithm(void);

#endif