/**
* @file iso7816.cpp
* @brief ISO-7816 driver implementation
*/

#include "iso7816.hpp"
#include "board.hpp"
#include "system_timer.hpp"
#include <string.h>


ISO7816 ISO7816_1;


/// Module options
enum Options_t
{
	ISO7816_RX_BUFFER_SIZE = 512,			///< TX buffer size
	ISO7816_TX_BUFFER_SIZE = 512,			///< RX buffer size
	ISO7816_ETU = 372,						///< Elementary Time Unit (ISO7816-3 3.1.a)
	ISO7816_FREQUENCY = 3000000,			///< Basic frequency (Hz) (baudrate 3600000 / 372 = 9677 baud)
	ISO7816_T3_TICKS = 40000,				///< Delay before reset procedure (tact count) (t3 (ISO7816-3 3.2.b))
	ISO7816_BIT_CONVETNTION_DIRECT = 0x3B,	///< Data polarity - direct
	ISO7816_PROTOCOL_T0 = 0x00,				///< Protocol - T0 (asynchronous, half-duplex, character)
};


/**
* @brief Coustructor
*/
ISO7816::ISO7816()
{
	RxBuffer = new CircularBuffer(ISO7816_RX_BUFFER_SIZE);
	TxBuffer = new CircularBuffer(ISO7816_TX_BUFFER_SIZE);
}


/**
* @brief Interrupt handler
*/
void ISO7816::ISO7816_1_Handler()
{
	ISO7816_1.Handler();
}


/**
* @brief Interrupt handler
*/
void ISO7816::Handler()
{
	/// If receiver is not empty, put data into the buffer
	if(USART2->SR & USART_SR_RXNE)
	{
		volatile uint8_t data = USART2->DR;
		RxBuffer->PutChar(data);
	}
	
	/// If transmitter is empty, transmit byte from the queue
	if(USART2->SR & USART_SR_TXE)
	{
		uint8_t data;
		
		/// Check parity error
		if((USART2->SR & USART_SR_PE) /*&& (USART6->CR1 & USART_CR1_TXEIE)*/)
		{
			USART2->DR = BackupChar;
		}
		else if(TxBuffer->GetChar(&data))
		{
			BackupChar = data;
			USART2->DR = data;
		}
		else if(USART2->CR1 & USART_CR1_TXEIE)
		{
			/// If there is no data in the queue and interrupts on transmission buffer devastation are enabled,
			/// cancel them and allow interrupt on transmission completion
			USART2->CR1 &= ~USART_CR1_TXEIE;
			USART2->CR1 |= USART_CR1_TCIE;
		}
		else if((USART2->CR1 & USART_CR1_TCIE) && (USART2->SR & USART_SR_TC))
		{
			/// If interrupts on transmission completion are allowed and transmission is completed,
			/// cancel interrupts on transmission completion
			USART2->CR1 &= ~USART_CR1_TCIE;
		}
	}
}


/**
* @brief Data transmission
* @param data - source data pointer
* @param count - bytes count
*/
void ISO7816::Transmit(const void* data, uint16_t count)
{
    NVIC_DisableIRQ(USART2_IRQn);
    RxBuffer->Reset();
    NVIC_EnableIRQ(USART2_IRQn);
	
	/// Transmit data
	TxBuffer->Put(data, count);
	USART2->CR1 |= USART_CR1_TXEIE;
	
	/// Wait for transmission complete
	for(uint32_t waitTo = SystemTimer::GetTime() + 1; SystemTimer::GetTime() < waitTo;)
	{
		if(RxBuffer->GetByteCount() >= count)
		{
			break;
		}
	}
	
    NVIC_DisableIRQ(USART2_IRQn);
    RxBuffer->Delete(count);
    NVIC_EnableIRQ(USART2_IRQn);
}


/**
* @brief Charater reading
* @param chr - destination data pointer
* @return true, if the character is read
*/
bool ISO7816::GetReceivedChar(char* chr)
{
	for(uint32_t index = 0; index < HSE_VALUE; index++)
	{
		if(RxBuffer->GetChar(chr))
		{
			return true;
		}
	}
	return false;
}


/**
* @brief ATR request
* @param atr - destination data buffer
* @return true, if ATR is read
*/
bool ISO7816::GetATR(ATR_t* atr)
{
	/// Guard Time 16 bits (GTPR = 16 << 8)
	/// Clock last bit (USART_CR2_LBCL = 0)
	/// Polarity low (USART_CR2_CPHA = 0)
	/// Phase on front (USART_CR2_CPOL = 0)
	/// Frame length 9 bits (8 data bits + 1 parity) (USART_CR1_M = 1)
	
	RCC->APB2RSTR |= RCC_APB1RSTR_USART2RST;
	RCC->APB2RSTR &= ~RCC_APB1RSTR_USART2RST;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	Core::RegIrqHandler(USART2_IRQn, ISO7816_1_Handler);
	
	uint32_t apbClock = HSE_VALUE;
	uint32_t clk = ISO7816_FREQUENCY;
	uint32_t baud = ISO7816_FREQUENCY / ISO7816_ETU;
	uint32_t integerDivider = apbClock / (16 * baud);
	uint32_t fractionalDivider = (10000 * (apbClock - (16 * integerDivider))) / (16 * baud);
	USART2->BRR = (integerDivider << 4) | (fractionalDivider & 0x0F);
	USART2->GTPR = (16 << 8) | ((apbClock / clk) / 2);
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_PCE | USART_CR1_M | USART_CR1_UE;
	USART2->CR2 = USART_CR2_LBCL | USART_CR2_CLKEN | USART_CR2_STOP;
	USART2->CR3 = USART_CR3_NACK | USART_CR3_SCEN;
	
	/// ATR reading cycle
	uint8_t result = 0;
	for(; result < 1; result++)
	{
		Board::Set_ISO7816_RST_Low();
		Board::Set_ISO7816_VCC_High();
		
		/// Wait t3 interval
		for(uint32_t index = 0; index < ISO7816_T3_TICKS * (HSE_VALUE / ISO7816_FREQUENCY); index++);
		
		Board::Set_ISO7816_RST_High();
		
		/// If no answer for t3 interval, return error
		bool response = false;
		for(uint32_t index = 0; index < ISO7816_T3_TICKS * (HSE_VALUE / ISO7816_FREQUENCY); index++)
		{
			if(!RxBuffer->IsEmpty())
			{
				response = true;
				break;
			}
		}
		if(!response)
		{
			break;
		}
		
		/// Read ATR
		memset(atr, 0, sizeof(ATR_t));
		if(!GetReceivedChar((char *)&atr->TS))								break;
		if(!GetReceivedChar((char *)&atr->T0))								break;
		atr->Hlength = atr->T0 & 0x0F;
		if((atr->T0 & (1 << 4)) && !GetReceivedChar((char *)&atr->TA1))		break;
		if((atr->T0 & (1 << 5)) && !GetReceivedChar((char *)&atr->TB1))		break;
		if((atr->T0 & (1 << 6)) && !GetReceivedChar((char *)&atr->TC1))		break;
		if((atr->T0 & (1 << 7)) && !GetReceivedChar((char *)&atr->TD1))		break;
		if((atr->TD1 & (1 << 4)) && !GetReceivedChar((char *)&atr->TA2))	break;
		if((atr->TD1 & (1 << 5)) && !GetReceivedChar((char *)&atr->TB2))	break;
		if((atr->TD1 & (1 << 6)) && !GetReceivedChar((char *)&atr->TC2))	break;
		if((atr->TD1 & (1 << 7)) && !GetReceivedChar((char *)&atr->TD2))	break;
		
		/// Read historical bytes (H)
		if(atr->Hlength)
		{
			bool response = true;
			for(uint8_t index = 0; index < atr->Hlength; index++)
			{
				if(!GetReceivedChar((char *)&atr->H[index]))
				{
					response = false;
					break;
				}
			}
			
			if(!response)
			{
				break;
			}
		}
		
		if((atr->TS != ISO7816_BIT_CONVETNTION_DIRECT) || ((atr->TD1 & 0x0F) != ISO7816_PROTOCOL_T0))
		{
			break;
		}
	}
	
	/// Deactivate interface
	RCC->APB2RSTR |= RCC_APB1RSTR_USART2RST;
	RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
	Core::UnregIrqHandler(USART2_IRQn);
	
	return result;
}
