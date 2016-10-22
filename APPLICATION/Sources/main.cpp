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
#include "data_eeprom.hpp"
#include "uart.hpp"
#include "iso7816.hpp"
#include "options.hpp"
#include "stm32l1xx.h"                  // Device header
#include <string.h>
#include <stdio.h>


const char MF[] = {0x3F, 0x00};
const char EFiccid[] = {0x2F, 0xE2};


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
	
	char printbuffer[64];
	memset(printbuffer, 0, sizeof(printbuffer));
	
	if(ISO7816_1.ActivateCard())
	{
		for(uint8_t result = 0; result < 1; result++)
		{
			if(!ISO7816_1.SelectFile(0xA0, 0x00, 0x00, MF, sizeof(MF)))
			{
				sprintf(printbuffer, "MF file selection error!\r\n");
				break;
			}
			
			if(!ISO7816_1.SelectFile(0xA0, 0x00, 0x00, EFiccid, sizeof(EFiccid)))
			{
				sprintf(printbuffer, "EFiccid file selection error!\r\n");
				break;
			}
			
			char iccid[9];
			memset(iccid, 0, sizeof(iccid));
			if(ISO7816_1.ReadBinary(0xA0, iccid, 9) == -1)
			{
				sprintf(printbuffer, "ICCID reading error!\r\n");
				break;
			}
			
			char* ptr = printbuffer;
			for(uint8_t index = 0; index < 9; index++)
			{
				ptr += sprintf(ptr, "%u%u", iccid[index] & 0x0F, iccid[index] >> 4);
			}
			
			ptr += sprintf(ptr, "\r\n");
		}
	}
	else
	{
		sprintf(printbuffer, "Card activation error!\r\n");
	}
	
	ISO7816_1.DeactivateCard();
	
	Uart1.Transmit(printbuffer, strlen(printbuffer));
	
	while(1)
	{
#ifndef __DEBUG__
		WatchdogTimer::Kick();
#endif
	}
}
