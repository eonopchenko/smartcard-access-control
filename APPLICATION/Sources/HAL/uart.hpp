/**
* @file uart.hpp
* @brief UART driver header
*/

#ifndef __UART_HPP
#define __UART_HPP

#include "core.hpp"
#include "stm32l1xx.h"                  // Device header
#include <stdint.h>


/**
* @brief UART driver class
*/
class Uart
{
	public:
		enum Options_t
		{
			RX_SIZE = 1024,
			TX_SIZE = 1024,
		};
		
		/// Data transmission
		void Transmit(const char* data, uint16_t count);
		
		/// Received data copying
		uint16_t CopyReceivedData(char* buffer, uint16_t size);
		
		/// Received data deletion
		void DeleteReceivedData(uint16_t count);
		
		/// Checking, whether the receive buffer is empty
		bool IsReceiverEmpty();
		
		/// Receive buffer cleaning
		void Flush();
		
		/// USART1 interrupt handler
		static void UART1_Handler();
		
		/// Constructor
		Uart(uint32_t baudrate);
		
	private:
		char *RxBuffer;	///< Receive buffer
		char *TxBuffer;	///< Transmit buffer
		char* RxHead;	///< Receive buffer head
		
		/// Interrupt handler
		void Handler();
};


extern Uart Uart1;

#endif /* __UART_HPP */
