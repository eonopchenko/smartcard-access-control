/**
* @file core.cpp
* @brief Core functions implementation
*/
#include "core.hpp"


/**
* @brief Interrupt handler registration
* @param irqn - interrupt number
* @param handler - interrupt handler pointer
* @param priority - interrupt priority
*/
void Core::RegIrqHandler(IRQn_Type irqn, IrqHandler_t handler, int32_t priority)
{
	/// Write vector to the table
	IrqHandler_t* table = (IrqHandler_t* )(SRAM_BASE + 0x40);
	*(table + irqn) = handler;
	
	/// Set priority
	if(priority != -1)
	{
		NVIC_SetPriority(irqn, priority);
	}
	
	/// Enable interrupt
	NVIC_EnableIRQ((IRQn_Type)irqn);
}


/**
* @brief Interrupt handler unregistration
* @param irqn - interrupt number
*/
void Core::UnregIrqHandler(IRQn_Type irqn)
{
	NVIC_DisableIRQ((IRQn_Type)irqn);
}
