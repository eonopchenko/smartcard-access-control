/**
* @file vectors.cpp
* @brief Interrupts vector table
* @note Debug only
*/
#include "system_timer.hpp"
#include "uart.hpp"

extern unsigned Image$$ARM_LIB_STACK$$ZI$$Limit;
extern unsigned init;

/// Vector table
unsigned const VectTable[] __attribute__ ((section ("RESET"))) = 
{
	(unsigned)&Image$$ARM_LIB_STACK$$ZI$$Limit,	// Top of Stack
	(unsigned)&init,							// Reset Handler
};
