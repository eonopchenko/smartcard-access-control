/**
 * @file main.cpp
 *
 * @brief Smartcard Access Control System
 *
 * @author Evgeny Onopchenko
 * @version 1.0
 */


/**
* @file main.cpp
* @brief Main application module
*/

#include "board.hpp"
#include "watchdog_timer.hpp"
#include "system_timer.hpp"
#include "embedded_flash.hpp"
#include "uart.hpp"
#include "iso7816.hpp"
#include "stm32l1xx.h"                  // Device header
#include <string.h>
#include <stdio.h>

/// ATR
ATR_t ATR;


/**
* @brief Main application procedure
*/
int main(void)
{
	Board::Init();
	
#ifndef __DEBUG__
	WatchdogTimer::Init();
#endif
	
	SystemTimer::Init();
	
	uint32_t time1 = SystemTimer::GetTime();
	while(1)
	{
#ifndef __DEBUG__
		WatchdogTimer::Kick();
#endif
		
		uint32_t time2 = SystemTimer::GetTime();
		char printbuffer[256];
		if((time2 - time1) >= 3)
		{
			time1 = time2;
			
			if(ISO7816_1.GetATR(&ATR))
			{
				char* ptr = printbuffer;
				ptr += sprintf(ptr, "TS = %02Xh\r\nT0 = %02Xh\r\nTA1 = %02Xh\r\nTB1 = %02Xh\r\nTC1 = %02Xh\r\nTD1 = %02Xh\r\nTA2 = %02Xh\r\nTB2 = %02Xh\r\nTC2 = %02Xh\r\nTD2 = %02Xh\r\n",
				ATR.TS,
				ATR.T0,
				ATR.TA1,
				ATR.TB1,
				ATR.TC1,
				ATR.TD1,
				ATR.TA2,
				ATR.TB2,
				ATR.TC2,
				ATR.TD2);
				
				for(uint8_t index = 0; index < ATR.Hlength; index++)
				{
					ptr += sprintf(ptr, "%02X ", ATR.H[index]);
				}
				
				ptr += sprintf(ptr, "\r\n");
				
				Uart1.Transmit(printbuffer, strlen(printbuffer));
			}
			else
			{
				Uart1.Transmit(printbuffer, sprintf(printbuffer, "Error reading ATR!\r\n"));
			}
		}
	}
}
