/**
* @file watchdog_timer.hpp
* @brief Watchdog timer header
*/

#ifndef __WATCHDOG_TIMER_HPP
#define __WATCHDOG_TIMER_HPP


/**
* @brief Watchdog timer class
*/
class WatchdogTimer
{
	public:
		static void Init();	/// Watchdog timer initialization
		static void Kick();	/// Watchdog timer kick
};

#endif /* __WATCHDOG_TIMER_HPP */
