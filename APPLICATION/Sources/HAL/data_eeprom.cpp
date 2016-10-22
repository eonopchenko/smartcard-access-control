/**
* @file data_eeprom.cpp
* @brief Data EEPROM interface implementation
*/

#include "data_eeprom.hpp"
#include "options.hpp"


#define FLASH_PEKEY1 ((uint32_t)0x89ABCDEF)
#define FLASH_PEKEY2 ((uint32_t)0x02030405)


/**
* @brief Returns the FLASH Status
* @return FLASH_BUSY, FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE
*/
DataEeprom::FLASH_Status DataEeprom::GetStatus()
{
	FLASH_Status FLASHstatus = FLASH_COMPLETE;
	
	if((FLASH->SR & FLASH_SR_BSY) == FLASH_SR_BSY)
	{
		FLASHstatus = FLASH_BUSY;
	}
	else
	{
		if((FLASH->SR & (uint32_t)FLASH_SR_WRPERR)!= (uint32_t)0x00)
		{ 
			FLASHstatus = FLASH_ERROR_WRP;
		}
		else
		{
			if((FLASH->SR & (uint32_t)0x1E00) != (uint32_t)0x00)
			{
				FLASHstatus = FLASH_ERROR_PROGRAM;
			}
			else
			{
				FLASHstatus = FLASH_COMPLETE;
			}
		}
	}
	
	return FLASHstatus;
}


/**
* @brief Data writing
* @param address - destination address (to)
* @param data - source data pointer (from)
* @param count - bytes count
* @return true, if operation successful
* @note Bytes count should be multiple of Word size (4 bytes)!
*/
bool DataEeprom::Write(uint32_t address, char* data, uint32_t count)
{
	if((address < DATA_EEPROM_ADDRESS) || (address > DATA_EEPROM_ADDRESS + 4096) || ((count % sizeof(uint32_t)) != 0))
	{
		return false;
	}
	
	char* ptr = data;
	uint32_t remaining = count;
	
	if((FLASH->PECR & FLASH_PECR_PELOCK) != RESET)
	{
		FLASH->PEKEYR = FLASH_PEKEY1;
		FLASH->PEKEYR = FLASH_PEKEY2;
	}
	
	while(remaining)
	{
		while(GetStatus() == FLASH_BUSY);
		*(__IO uint32_t *)address = *(uint32_t *)ptr;
		address += sizeof(uint32_t);
		ptr += sizeof(uint32_t);
		remaining -= sizeof(uint32_t);
	}
	
	while(GetStatus() == FLASH_BUSY);
	FLASH->PECR |= FLASH_PECR_PELOCK;
	
	return true;
}
