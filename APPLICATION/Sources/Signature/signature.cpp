/**
* @file signature.cpp
* @brief Firmware signature
*/
#include <stdint.h>
#include "options.hpp"

const uint32_t __attribute__((at((FIRMWARE_ADDRESS) + 0x00004000))) FWV_MAJ = (uint32_t)FIRMWARE_VERSION_MAJOR;
const uint32_t __attribute__((at((FIRMWARE_ADDRESS) + 0x00004004))) FWV_MIN = (uint32_t)FIRMWARE_VERSION_MINOR;
