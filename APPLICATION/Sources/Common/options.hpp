/**
* @file options.hpp
* @brief Options header
*/

#ifndef __OPTIONS_HPP
#define __OPTIONS_HPP

/**
* @brief General options
*/
enum GeneralOptions_t
{
	FIRMWARE_VERSION_MAJOR	= 1,	///< Firmware version (major part)
	FIRMWARE_VERSION_MINOR	= 0,	///< Firmware version (minor part)
	BOOTLOADER_VERSION		= 0,	///< Bootloader version
	DEFAULT_DEVICE_ID		= 0,	///< Device identifier
};


/**
* @brief Memory map
* @note 128 kb
*/
enum MemoryMap_t
{
	SECTOR_SIZE							= 0x0400,		///< Sector size
	FIRMWARE_MAX_SIZE					= 0xE000,		///< Max firmware size
	
	EXPORT_TABLE_ADDRESS				= 0x08000200,	///< Sector 0+0x200 (export table address)
	FIRMWARE_ADDRESS					= 0x08002000,	///< Sector 2 (firmware start address)
	FIRMWARE_IMAGE_ADDRESS				= 0x08003000,	///< Sector 16 (firmware image start address)
};

#endif	/* __OPTIONS_HPP */
