/**
* @file system_timer.hpp
* @brief System timer header
*/

#ifndef	__SYSTEM_TIMER_HPP
#define	__SYSTEM_TIMER_HPP

#include <stdint.h>

/**
* @brief System timer class
*/
class SystemTimer
{
	public:
		static void Init();			/// System timer initialization
		static uint32_t GetTime();	/// Get current system timer value
		static void Handler();		/// System timer interrupt handler
	
	private:
		static uint32_t SecCounter;
};

#endif /* __SYSTEM_TIMER_HPP */
