/*
*			Projekt : Robot terrain mapper
*
*			Autorzy:  Maciej Flaum
*								Tomasz Koczwara
*			Platforma: Zestaw uruchomieniowy KL46Z ( ARM Cortex M0+ )




	Zajęte piny:
	PTA6 - jeden z enkoderów
	PTA22 TX UART
	PTA23 RX UART od bluetooth
	PTE16 - IR[1]
	PTE18 - IR[2]
	PTE20 - IR[0]
	

  MISO PTA 17
  MOSI PTA16
	SCK PTA15
	SS PTA14



*/

/* DOŁĄCZENIE PLIKÓW NAGŁÓWKOWYCH --------------------------- */
#include "MKL46Z4.h"
#include "main.h"
#include "mapper.h"










/* MAIN FUNCTION ---------------------------------------------- */
int main(void)
{
	/* INICJALIZACJA */
	SysTick_init(1000);			// konfiguracja licznika systemowego, częstotliwość == 1[kHz]
	UART2_init(38400);			// inicjlaizacja UART
	RFID_init();						// incijalizacja modułu RFID
	motorDriverInit();			// inicjalizacja napędu
	IRInitialize(7);				// inicjalizuje wszytkie 3 czujniki
	SONAR_init();
	
	// startuje konwersje dla 3 czujników
	Start_cont(0);
	Start_cont(1);	
	Start_cont(2);




	/* INFINITE LOOP */
	while(1)
	{

		/* POBIERAMY DYSTANS ZMIERZONY PRZEZ CZUJNIKI */
	  check_distance(50);
		/* WYSYŁAMY DANE DO USERA PRZEZ BLUETOOTH */
	//	send_data_to_user(1500);		// wysyłamy dane z sensorów etc.
	  /* ODCZYTaNIE TAGA RFID */


		/* LOGIKA GŁÓWNA - program jest sterowany komendami przysłanym przez usera */
		switch(command[0])
		{

			/* POPRZESŁANIU KOMENDY W OD USERA ROBOT ZACZYNA JECHAĆ */
			case 'w':
				if(!start)
				{		// rozpoczęcie rozwiązywania labiryntu
				 mapper_init();
				 start=1;														// ta część wykona się tylko 1 raz
				 UART_send(UART2,"Start...\r\n");		// wysyłamy informację do użytkownika
				 odl=left_sensor-right_sensor;			// dokonujemy kalibracji ustawienia robota w labiryncie
				 distance=0;												// zerujemy licznik przejechanej odległości ( może się zmienić przypadkowo przy ustawianiu robota w labiryncie
				}  
				movement_algorithm();
			break;

			/* KOMENDA Q WYŁĄCZA RUCH ROBOTA */
			case 'q':
						start=0;
						driveStop();
			break;
			
			/* UMOŻLIWIA ZMIANĘ PARAMETRÓW REGULATORA PROPORCJONALNEGO STERUJĄCEGO SILNIKAMI */
			default:
			if(command[0]>0 && command[0]<70) dzielnik=command[0];
			break;
		}


}
}



/* PRZERWANIE TIMERA SYSTEMOWEGO ---------------------------------------- */
void SysTick_Handler(void)
{
	uint16_t x;
	x=timer;
	if(x) timer=--x;
	x=timer2;
	if(x) timer2=--x;
	x=timer3;
	if(x) timer3=--x;
	x=rfid_timer;
	if(x) rfid_timer=--x;
	x=motor_timer;
	if(x) motor_timer=--x;	
}

void movement_algorithm(void)
{			static uint8_t state;				// maszyna stanu realizująca algorytm jazdy po labiryncie
			int32_t difference=0;				// rożnica w odczytach pomiędzy czujnikami, służy do stabilizacji jazdy robota w labiryncie
			static	int32_t diff_old=0;
			static int32_t diff=0;
			static uint8_t next_turn;   // gdzie na skręcić robot 
			int lewy=0;									// regulator P 
			int prawy=0;								// regulator P
	
			switch(state)
				{
					// sprawdzamy gdzie znajdują się ściany 
					case check_wall:	
						if(  (front_sensor<1600) ) state=get_closer; else
						if( !(wall_position & FRONT_WALL) ) state=drive_forward; else
						if( !(wall_position & LEFT_WALL)  ) state=turn_left; else						
						if( !(wall_position & RIGHT_WALL) ) state=turn_right; else 
						state=turn_around;
						distance=0;
					break;
					case stop:

					break;
					// jazda robota do przodu, jedziemy dokładnie tyle centymetrów ile wynosi stała CELL_SIZE
					case drive_forward:		
					
					  if(distance<2*CELL_SIZE && front_sensor>700) 	// jeśli jeszcze nie przejechaliśmy odległości równej długości jednej komórki i maamy miejsce żeby jechać
						{

							state=drive_forward; 
							diff_old=diff;
							/* REGULATOR PROPORCJONALNY */
							diff=left_sensor-right_sensor;
							if(diff_old>900) difference=0; else difference=diff_old;

							// stan oznaczający że robot jest bliżej prawej sciany - silnik prawy musi działać mocniej 
							if(difference>0)
							{ 
								difference -=odl;							// odejmujemy od uchybu wartość kalibracji
								prawy=(difference)/dzielnik;  // ustalamy wartość regulacji 
								lewy=(difference*-1)/dzielnik;								
								if(prawy>REGULATION_LIMIT) {prawy=REGULATION_LIMIT;lewy=REGULATION_LIMIT*(-1);}
								if( rozn>120 )   {lewy=0;prawy=0;} else 
								if( difference > 1000 && (wall_position & LEFT_WALL) && (wall_position & RIGHT_WALL) ) {lewy=0;prawy=0; } else
								if( (!(wall_position & RIGHT_WALL)) ){lewy=0;prawy=0;} else
								if( (!(wall_position & LEFT_WALL)) ){lewy=0;prawy=0;} 
								if( right_sensor<600 ) {prawy+=8; lewy-=8;}
							}  
							// stan oznaczający że robot jest bliżej lewej sciany - silnik lewy musi działać mocniej 							
							if(difference<0)
							{

								difference *=-1;
								difference +=odl;           // dodajemy do uchybu wartość kalibracji
								lewy=(difference)/dzielnik; // ustalamy wartość kalibracji
								prawy=(difference*-1)/dzielnik;
								if(lewy>REGULATION_LIMIT) {lewy=REGULATION_LIMIT;prawy=REGULATION_LIMIT*-1;}
								if( rozn>120 )   {lewy=0;prawy=0;} else 
								if( difference > 1000 && (wall_position & LEFT_WALL) && (wall_position & RIGHT_WALL) ) {lewy=0;prawy=0; } else
								if( (!(wall_position & RIGHT_WALL)) ){lewy=0;prawy=0;} else
								if( (!(wall_position & LEFT_WALL)) ){lewy=0;prawy=0;} 								
								if( left_sensor<600 ) {prawy-=8; lewy+=8;}
							}											
							if(!difference) { lewy=0; prawy=0; }		// jezeli jestesmy w takiej samej odleglosci od sciany to nie regulujemy
							driveForwardLeftTrack(ENGINE+lewy);
							driveForwardRightTrack(ENGINE-3+prawy);
							
						} 
							

						else   // jeśli przejechaliśmy dystans jednej komórki 
						{
							add_wall();
							state=drive_forward_timer;  // idziemy do stanu oczekiwania 
							motor_timer=1500;						// 1,5 s przerwy
							distance=0;									// czyścimy licznik przejechanego dystansu
							driveStop();								// wyłączamy silniki 
						}
					break;

					// stan odmierzający opóźnienie przy jeżdzie robota
					case drive_forward_timer:
						if(!motor_timer) state=check_wall;
					break;
					
					// stan odpowiadający za obrót robota o 90 stopni w lewo
					case turn_left:
						change_direction_after_turn_left();
						turnLeft(TURN_ANGLE);						
						motor_timer=5000; 	// opóźnienie na zakończenie obrotu przez robota
						state=turn_left_timer;		// czekamy 

					break;
					
					// stan oznaczający że robot się już obrócił teraz musi przejechać do kolejnej komórki 
					case turn_left_timer:
						if(!motor_timer)
						{	
							state=drive_forward;
							distance=0;	  // kasujemy odległość przejechaną podczas zakręcania						
						}
					break;
					
					// stan odpowiadający za obrót robota o 90 stopni w prawo
					case turn_right:
						change_direction_after_turn_right();
						motor_timer=5000; 	// opóźnienie na zakończenie obrotu przez robota
						state=turn_right_timer;		// czekamy 
 					  turnRight(TURN_ANGLE);
					break;
					
					// stan oznaczający że robot się już obrócił teraz musi przejechać do kolejnej komórki 
					case turn_right_timer:
						if(!motor_timer)
						{
							state=drive_forward;
							distance=0;	  // kasujemy odległość przejechaną podczas zakręcania						
						}
					break;
					
					// stan oznaczający że robot ma się obrócić wokół własnej osi 
					case turn_around:
					  change_direction_after_turn_around();
						motor_timer=5000; 	// opóźnienie na zakończenie obrotu przez robota
						state=turn_around_timer;		// czekamy 
						
 					  if(right_sonar>left_sonar) turnRight(2*TURN_ANGLE); else turnLeft(2*TURN_ANGLE);
					break;
					
					// stan oznaczający że robot się już obrócił teraz musi przejechać do kolejnej komórki 
					case turn_around_timer:
						if(!motor_timer)
						{
							state=drive_forward;
							distance=0;	  // kasujemy odległość przejechaną podczas zakręcania						
						}
					break;
					
					// podjeżdżamy do ściany aby odczytać tag
					case get_closer:
						if(front_sensor>600) driveForward(13); else {state=read_rfid;driveStop();}
					break;
					
					// czytamy tag
					case read_rfid:
						if(!rfid_timer)
						{
							uint8_t byte=0;
							uint8_t temp_array[5]={0};
							timer=500;
							
							// sekwencja odczytu karty
							byte=RFID_request(temp_array);

							// gdy znaleźliźmy karte
							if(byte==CARD_FOUND)
							{
								uint8_t i;
								RFID_get_card_serial(temp_array);	
								UART_send(UART2,"Znaleziono karte: ");
								for(i=0;i<5;i++) {UART_send_int(UART2,temp_array[i]); UART_send(UART2,"\t");}
								UART_send(UART2,"\r\n");
								state = go_backward; /////////// tymczasowo 
								// w zalezmosci od danych zapisanych w karcie wybieramy w ktorym kieunku skrecamy
								if(temp_array[4]==185) state=print_wall;  else            // karta stop
								if(temp_array[1]%2) next_turn=1; else next_turn=2;	//  skrecanie w lewo badz w prawo
								
							}	
							else {state=go_backward;next_turn=0;}
						}
					break;
					
					// wycofujemy sie 
					case go_backward:
						if(front_sensor<900) driveReverse(13);
						else {
									driveStop();
									if(next_turn==1 && !(wall_position & LEFT_WALL)) state=turn_left; else state=turn_around;
									if(next_turn==2 && !(wall_position & RIGHT_WALL)) state=turn_right; else state=turn_around;
									if(next_turn==0)
									{
										if( !(wall_position & LEFT_WALL)  ) state=turn_left; else						
										if( !(wall_position & RIGHT_WALL) ) state=turn_right;else state=turn_around;
										distance=0; 
									}
								}
					break;		
					
					// wyświetla trase robota
					case print_wall:
						UART_send(UART2,"Jazda zakończona \r\n");
						UART_send(UART2,"MAPA LABIRYNTU   MAPA LABIRYNTU \r\n");						
						print_map();
						UART_send(UART2,"\r\n");									
						state=check_wall;
						command[0]='q';
					break;
				}
}

void check_distance(uint16_t period)
{
	if(!timer)
	{
		timer=period; 								 // update timer
		UART_get(UART2,command);			 


	 /* odczytujemy odległość z czujników optycznych */
	 front_sensor = Get_distance(2);
	 right_sensor = Get_distance(0);
	 left_sensor = Get_distance(1);
	
	 diff_old=diff;
	 diff=left_sensor-right_sensor;
	 
	 rozn=diff-diff_old;
	 

	/* wyzwalamy sonary */

	 SONAR_trigger();
	 /* odczytujemy wartości odległości z sonarów */
	 left_sonar=SONAR_left_distance();
	 right_sonar=SONAR_right_distance();
	 
	 wall_position = check_wall_position();
	 

	}
}

void send_data_to_user(uint16_t period)
{

	if(!timer3)
  {
	 timer3=period;
	 UART_send(UART2,"F: ");
	 UART_send_int(UART2,(int)front_sensor);
	 UART_send(UART2,"\t");
	 UART_send(UART2,"L: ");
	 UART_send_int(UART2,(int)left_sensor);
	 UART_send(UART2,"\t");
	 UART_send(UART2,"R: ");
	 UART_send_int(UART2,(int)right_sensor);
	 UART_send(UART2,"\t");	 
//	 UART_send(UART2,"Lewy: ");
//	 UART_send_int(UART2,(int)left_sonar);
//	 UART_send(UART2,"\t");
//	 UART_send(UART2,"Prawy: ");
//	 UART_send_int(UART2,(int)right_sonar);	 
 //  UART_send(UART2,"\r\n");
//	 UART_send(UART2,"Widoczne sciany: ");
//   UART_send(UART2,"\r\n");	 
//	 if(wall_position & FRONT_WALL) UART_send(UART2,"Sciana przednia \r\n"); else    UART_send(UART2,"\r\n");	 
//	 if(wall_position & LEFT_WALL) UART_send(UART2,"Sciana lewa \r\n"); else    UART_send(UART2,"\r\n");	 
//	 if(wall_position & RIGHT_WALL) UART_send(UART2,"Sciana prawa \r\n"); else    UART_send(UART2,"\r\n");	 	
	 UART_send(UART2,"Diff: ");
	 UART_send_int(UART2,left_sensor-right_sensor);	
   UART_send(UART2,"\r\n");	
//	 UART_send(UART2,"Roznicza: ");
//	 UART_send_int(UART2,rozn);	
//   UART_send(UART2,"\r\n");	 	 
	}
}


uint8_t check_wall_position(void)
{
  uint8_t temp;
	if(front_sensor < 1500) temp |= FRONT_WALL; else temp &=~(FRONT_WALL);
	if(left_sonar <120 )    temp |= LEFT_WALL; else temp &=~(LEFT_WALL);
	if(right_sonar <120 )   temp |= RIGHT_WALL; else temp &=~(RIGHT_WALL);
	return temp;
}
