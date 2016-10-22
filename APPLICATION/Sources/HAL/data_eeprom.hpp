/**
* @file data_eeprom.hpp
* @brief Data EEPROM interface header
*/

#ifndef __DATA_EEPROM_HPP
#define __DATA_EEPROM_HPP

#include "stm32l1xx.h"                  // Device header





/**
* @brief Data EEPROM interface class
*/
class DataEeprom
{
	public:
		static bool Write(uint32_t address, char* data, uint32_t count);	/// Data writing
	
	private:
		/** 
		* @brief FLASH Status  
		*/
		typedef enum
		{
			FLASH_BUSY = 1,
			FLASH_ERROR_WRP,
			FLASH_ERROR_PROGRAM,
			FLASH_COMPLETE,
			FLASH_TIMEOUT
		} FLASH_Status;
		
		static FLASH_Status GetStatus();									/// 
};

#endif /* __DATA_EEPROM_HPP */
