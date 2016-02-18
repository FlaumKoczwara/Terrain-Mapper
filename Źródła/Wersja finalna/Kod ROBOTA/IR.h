#include "MKL46Z4.h"
#ifndef IR_h
#define IR_h
#define N 50			/*! Number of avaraged samples*/
#define IR_MASK(x) 1<<x 			/*! Mask used to rewrite active IR's to an array*/
static uint32_t IRN=0;				/*! Current sensor number*/
static uint32_t inc=0;				/*! Index variable used for avaraging*/
static uint32_t Mode[3];			/*! Mode selection array: 0 - single, 1 - continuous*/ 
static uint32_t IR[3]={0,0,0};/*! Sensor selection array: IR[0] - PTE20, IR[1]- PTE 16, IR[2]- PTE 18*/
static volatile uint32_t Flag_conv[3]={0,0,0};			/*! Flag used for waiting for the completion of conversion in single mode*/
static volatile uint32_t Flag_curr[3]={0,0,0};			/*! Flag used to sygnalize that the interrupts incoming are from a specific sensor*/
static volatile uint32_t Flag_next[3]={0,0,0};			/*! Flag that sygnalizes that a sensor wants to start a conversion*/
static uint32_t Distance[3];			/*! Final output variables*/
static uint32_t tab[N];						

void Constant_measure (uint32_t IRn, uint32_t Dist);
void IRInitialize(uint32_t IRn);
uint32_t Get_distance(uint32_t IRn);
static void ADCInitialize (uint32_t channel, uint32_t resolution);
void Start_cont (uint32_t IRn);
void Stop_cont (uint32_t IRn);

#endif

