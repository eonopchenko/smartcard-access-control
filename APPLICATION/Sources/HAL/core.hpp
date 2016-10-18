/**
* @file core.hpp
* @brief Core functions header
*/

#ifndef __CORE_HPP
#define __CORE_HPP

#include "stm32l1xx.h"                  // Device header

/// Interrupt handler
typedef void (*IrqHandler_t)();

/**
* @brief Core functions class
*/
class Core
{
	public:
		/// Interrupt handler registration
		static void RegIrqHandler(IRQn_Type irqn, IrqHandler_t handler, int32_t priority = -1);
		
		/// Interrupt handler unregistration
		static void UnregIrqHandler(IRQn_Type irqn);
};

#endif /* __CORE_HPP */
