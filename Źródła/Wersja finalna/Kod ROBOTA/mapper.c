#include "MKL46Z4.h"
#include "mapper.h"
#include "sonar.h"
#include "IR.h"
#include "UART.h"
/* DEFINICJE -------------------------------------------------- */
#define  WEST    1     // binarnie 00000001 
#define  SOUTH   2    // binarnie 00000010 
#define  EAST    4     // binarnie 00000100 
#define  NORTH   8    // binarnie 00001000



/* ENUMERACJE ------------------------------------------------- */

/* GLOBAL VARIABLE -------------------------------------------- */
uint8_t current_direction=NORTH;	// aktualny kierunek jazdy robota
uint16_t map[25]={0};			// trzyma wygląd komórek 

uint8_t indeks=0;					// current index in the map array
/* DEKLARACJE FUNKCJI ----------------------------------------- */
void mapper_init(void)
{
	uint8_t i=0;
	for(i=0;i<25;i++) map[i]=0;
	map[0]=6;
	current_direction=NORTH;
	indeks=0;
	
}

void add_wall(void)
{
	uint32_t front_sensor = Get_distance(2);
	uint32_t left_sonar=SONAR_left_distance();
	uint32_t right_sonar=SONAR_right_distance();
	
	if(current_direction==NORTH)
	{
		indeks +=5;
		if(front_sensor<1500) map[indeks] |=NORTH;
		if(left_sonar<120)    map[indeks] |=WEST;
		if(right_sonar<120)   map[indeks] |=EAST;		
	}
	if(current_direction==EAST)
	{
		indeks +=1;
		if(front_sensor<1500) map[indeks] |=EAST;
		if(left_sonar<120)    map[indeks] |=NORTH;
		if(right_sonar<120)   map[indeks] |=SOUTH;		
	}
	if(current_direction==SOUTH)
	{
		indeks -=5;
		if(front_sensor<1500) map[indeks] |=SOUTH;
		if(left_sonar<120)    map[indeks] |=EAST;
		if(right_sonar<120)   map[indeks] |=WEST;		
	}
	if(current_direction==WEST)
	{
		indeks -=1;
		if(front_sensor<1500) map[indeks] |=WEST;
		if(left_sonar<120)    map[indeks] |=SOUTH;
		if(right_sonar<120)   map[indeks] |=NORTH;		
	}	
}

void change_direction_after_turn_left(void)
{
	if(current_direction==NORTH) current_direction=WEST; else
	if(current_direction==WEST) current_direction=SOUTH; else
	if(current_direction==SOUTH) current_direction=EAST; else
	if(current_direction==EAST) current_direction=NORTH; 	
}

void change_direction_after_turn_right(void)
{
	if(current_direction==NORTH) current_direction=EAST; else
	if(current_direction==WEST) current_direction=NORTH; else
	if(current_direction==SOUTH) current_direction=WEST; else
	if(current_direction==EAST) current_direction=SOUTH; 	
}

void change_direction_after_turn_around(void)
{
	if(current_direction==NORTH) current_direction=SOUTH; else
	if(current_direction==WEST) current_direction=EAST; else
	if(current_direction==SOUTH) current_direction=NORTH; else
	if(current_direction==EAST) current_direction=WEST; 	
}

void print_map(void)
{
	uint8_t i=0;
	uint8_t y=0;
	UART_send(UART2,"Mapa przejechanej przestrzeni");
	UART_send(UART2,"\r\n");	
	for(i=0;i<5;i++)
	{
		for(y=5;y>0;y--) 
		{	
			UART_send_int(UART2,map[25-y-(5*i)]);
			UART_send(UART2,"\t");
		}
		 UART_send(UART2,"\r\n");
	}
}
