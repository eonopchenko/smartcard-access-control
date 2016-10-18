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


/**
* @brief ISO-7816 driver class
*/
class ISO7816
{
	public:
		bool GetATR(ATR_t* atr);			/// ATR request
		ISO7816();							/// Constructor
		
	private:
		static void ISO7816_1_Handler();	/// Interrupt handler
		void Handler();						/// Interrupt handler
		
		void Transmit(const void* data, uint16_t count);
		bool GetReceivedChar(char* chr);
		
		CircularBuffer* RxBuffer;			/// Receive buffer
		CircularBuffer* TxBuffer;			/// Transmit buffer
		uint8_t BackupChar;					/// Char backup
};


extern ISO7816 ISO7816_1;


#endif /* __UART_HPP */
