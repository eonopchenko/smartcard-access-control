/**
* @file embedded_flash.hpp
* @brief Embedded flash interface header
*/

#ifndef __EMBEDDED_FLASH_HPP
#define __EMBEDDED_FLASH_HPP

#include "stm32l1xx.h"                  // Device header

/**
* @brief Embedded flash interface class
*/
class EmbeddedFlash
{
	public:
		static void Unlock();												/// Interface unlock
		static void Lock();													/// Interface lock
		static bool IsLocked();												/// Checking, whether the interface is locked
		static void Write(char* data, uint32_t address, uint32_t count);	/// Data writing
		static void Erase(uint32_t address);								/// Sector erase
};

#endif /* __EMBEDDED_FLASH_HPP */
