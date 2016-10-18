/**
* @file crc.hpp
* @brief Cyclic redundancy check functions header
*/

#ifndef	__CRC_HPP
#define	__CRC_HPP

#include <stdint.h>

/**
* @brief Cyclic redundancy check class
*/
class Crc
{
	public:
		static uint8_t Calc8(char* data, uint32_t count, uint16_t vector);
		static uint16_t Calc16(char* data, uint32_t count, uint16_t vector);
};

#endif	/* __CRC_HPP */
