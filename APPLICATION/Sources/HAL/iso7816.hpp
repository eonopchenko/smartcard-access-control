/**
* @file iso7816.hpp
* @brief ISO-7816 driver header
*/

#ifndef __ISO7816_HPP
#define __ISO7816_HPP

#include "core.hpp"
#include "stm32l1xx.h"                  // Device header
#include "circular_buffer.hpp"
#include <stdint.h>


#pragma pack(1)

/// ATR
struct ATR_t
{
	uint8_t TS;
	uint8_t T0;
	uint8_t TA1;
	uint8_t TB1;
	uint8_t TC1;
	uint8_t TD1;
	uint8_t TA2;
	uint8_t TB2;
	uint8_t TC2;
	uint8_t TD2;
	uint8_t Hlength;
	uint8_t H[20];
};

/// TPDU (link layer) (P3 will be transmit separated)
struct TPDU_t
{
	uint8_t CLA;
	uint8_t INS;
	uint8_t P1;
	uint8_t P2;
};

/// Exchange cases
enum Case_t
{
	CASE_1,	///< No data in command, no data in response
	CASE_2,	///< No data in command, data in response
	CASE_3,	///< Data in command, no data in response
	CASE_4,	///< Data in command, data in response
	
};
#pragma pack()


/**
* @brief ISO-7816 driver class
*/
class ISO7816
{
	public:
		/// Activate card
		bool ActivateCard(ATR_t* pAtr = 0);
		
		/// Deactivate card
		bool DeactivateCard();
		
		/// Send TPDU
		uint16_t SendTPDU(const TPDU_t* tpdu, const void* data, uint8_t count, Case_t exchangeCase);
		
		/// SELECT FILE
		bool SelectFile(uint8_t cla, uint8_t p1, uint8_t p2, const char* fileName, uint8_t nameLength);
		
		/// READ BINARY
		int_fast8_t ReadBinary(uint8_t cla, void* buffer, uint8_t count);
		
		/// GET RESPONSE
		int_fast8_t GetResponse(uint8_t cla, void* buffer, uint8_t count);
		
		ISO7816();
		
	private:
		static void ISO7816_1_Handler();					/// Interrupt handler
		void Handler();										/// Interrupt handler
		
		CircularBuffer* RxBuffer;							/// Receive buffer
		CircularBuffer* TxBuffer;							/// Transmit buffer
		uint8_t BackupChar;									/// Char backup
		
		bool GetReceivedChar(char* chr);					/// Get received char
		void Transmit(const void* data, uint16_t count);	/// Transmit data
		uint8_t CalcCK(char* data, uint8_t length);			/// Calculate CK
};


extern ISO7816 ISO7816_1;


#endif /* __UART_HPP */
