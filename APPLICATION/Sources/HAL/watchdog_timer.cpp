/**
* @file watchdog_timer.cpp
* @brief Watchdog timer implementation
*/
#include "watchdog_timer.hpp"
#include "stm32l1xx.h"                  // Device header


/**
* @brief Watchdog timer initialization
*/
void WatchdogTimer::Init()
{
	/// Tune prescaler (32)
	/// 0: divider /4		0.1	409.6
	/// 1: divider /8		0.2	819.2
	/// 2: divider /16	0.4	1638.4
	/// 3: divider /32	0.8	3276.8
	/// 4: divider /64	1.6	6553.6	<--
	/// 5: divider /128	3.2	13107.2
	/// 6: divider /256	6.4	26214.4
	/// 7: divider /256	6.4	26214.4
	IWDG->KR = 0x5555;
	IWDG->PR = 4;
	
	/// Tune counter value
	IWDG->KR = 0x5555 & IWDG_KR_KEY;
	IWDG->RLR = 0xFFF & IWDG_RLR_RL;
	
	/// Run timer
	IWDG->KR = 0xCCCC & IWDG_KR_KEY;
	
	/// Wait for register readiness
	while((IWDG->SR & IWDG_SR_PVU) || (IWDG->SR & IWDG_SR_RVU));
}


/**
* @brief Watchdog timer kick
*/
void WatchdogTimer::Kick()
{
	IWDG->KR = 0xAAAA & IWDG_KR_KEY;
}
