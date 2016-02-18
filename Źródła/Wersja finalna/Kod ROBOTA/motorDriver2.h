#include "MKL46Z4.h"

// zmienna trzymajÄ…ca przejechany dystans
extern volatile uint8_t distance;

// initialize motor driver
void motorDriverInit(void);
void TPM0_IRQHandler(void);

void wait4(void); // tymczasowo 4
void wait2 (void);
void wait3 (void);
void driveStopLeft(void);
void driveStopRight(void);
void driveStop(void);

void driveForwardLeftTrack( int predkosc);
void driveForwardDistLeftTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]
void driveForwardRightTrack( int predkosc);
void driveForwardDistRightTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]
void driveReverseLeftTrack( int predkosc);
void driveReverseDistLeftTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]
void driveReverseRightTrack( int predkosc);
void driveReverseDistRightTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]

void driveForward(int predkosc); // predkosc intiger 0 - 100 , nonlimit way
void driveForwardDist( int predkosc , int droga); // predkosc intiger 0 - 100
																									// droga intiger [ cm ] 
void driveReverse(int predkosc ); //  predkosc intiger 0 - 100 , nonlimit way
void driveReverseDist( int predkosc , int droga); // predkosc intiger 0 - 100
																									// droga intiger [ cm ] 

void acceleration( int oile); // oile is intiger 0 - 100 

// predkosc dorobie gdy zrobie enkoder
void turnLeft( int kat ); // kat jest w stopniach , podanie kat > 360 , zostanie wykonany obrót 
void turnRight( int kat );// kat jest w stopniach , podanie kat > 360 , zostanie wykonany obrót
