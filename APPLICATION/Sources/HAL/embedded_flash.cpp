/**
* @file embedded_flash.cpp
* @brief Embedded flash interface implementation
*/

#include "embedded_flash.hpp"
		

/**
* @brief Interface unlock
*/
void EmbeddedFlash::Unlock()
{
	/// Unlock FLASH_PECR
	FLASH->PEKEYR = 0x89ABCDEF;
	FLASH->PEKEYR = 0x02030405;
	
	/// Allow write/erase operations
	FLASH->PRGKEYR = 0x8C9DAEBF;
	FLASH->PRGKEYR = 0x13141516;
}


/**
* @brief Interface lock
*/
void EmbeddedFlash::Lock()
{
	FLASH->PECR |= FLASH_PECR_PRGLOCK;
	FLASH->PECR |= FLASH_PECR_PELOCK;
}


/**
* @brief Checking, whether the interface is locked
* @return true, if interface is locked
*/
bool EmbeddedFlash::IsLocked()
{
	return (FLASH->PECR & FLASH_PECR_PRGLOCK) || 
		(FLASH->PECR & FLASH_PECR_PELOCK);
}


/**
* @brief Data writing
* @param data - source data pointer (from)
* @param address - destination address (to)
* @param count - bytes count
*/
void EmbeddedFlash::Write(char* data, uint32_t address, uint32_t count)
{
}


/**
* @brief Sector erase
* @param address - sector address
*/
void EmbeddedFlash::Erase(uint32_t address)
{
}
