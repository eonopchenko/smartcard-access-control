/**
* @file board.hpp
* @brief Target board header
*/

#ifndef __BOARD_HPP
#define __BOARD_HPP


/**
* @brief Target board class
*/
class Board
{
	public:
		static void Init();					/// Target board initalization
		static void SetRead485();			/// RS485 Read (RE receiver enable)
		static void SetWrite485();			/// RS485 Write (DE driver enable)
		static void Set_ISO7816_VCC_High();	/// Set ISO-7816 VCC high
		static void Set_ISO7816_VCC_Low();	/// Set ISO-7816 VCC low
		static void Set_ISO7816_RST_High();	/// Set ISO-7816 RST high
		static void Set_ISO7816_RST_Low();	/// Set ISO-7816 RST low
};

#endif /* __BOARD_HPP */
