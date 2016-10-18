/**
* @file uart.cpp
* @brief UART driver implementation
*/

#include "uart.hpp"
#include "board.hpp"
#include <string.h>


Uart Uart1(115200);


/**
* @brief Constructor
* @param baudrate
*/
Uart::Uart(uint32_t baudrate)
{
	RxBuffer = new char[RX_SIZE];
	
	TxBuffer = RxBuffer;
	
	/// Enable USART clocking
	RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	
	/// Enable DMA clocking
	RCC->AHBRSTR |= RCC_AHBRSTR_DMA1RST;
	RCC->AHBRSTR &= ~RCC_AHBRSTR_DMA1RST;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	Core::RegIrqHandler(USART1_IRQn, Uart::UART1_Handler);
	
	///--- RX ---///
	/// Max frame size
	DMA1_Channel5->CNDTR = RX_SIZE & DMA_CNDTR1_NDT;
	
	/// Data source address (USART data register)
	DMA1_Channel5->CPAR = ((uint32_t)&USART1->DR) & DMA_CPAR5_PA;
	
	/// Data destination address (SRAM)
	DMA1_Channel5->CMAR = ((uint32_t)RxBuffer) & DMA_CMAR5_MA;
	
	/// DMA RX channel tunning
	DMA1_Channel5->CCR = (
		DMA_CCR5_PL_0 | 		///< Channel priority level = Medium
		DMA_CCR5_MINC | 		///< Memory increment mode = Enabled
		DMA_CCR5_CIRC | 		///< Circular mode = Enabled
		DMA_CCR5_EN);			///< Channel enable = Enabled
	
	
	///--- TX ---///
	/// Switch off transmission
	DMA1_Channel4->CNDTR = 0;
	
	/// Data destination address (USART data register)
	DMA1_Channel4->CPAR = ((uint32_t)&USART1->DR) & DMA_CPAR4_PA;
	
	/// Data source address (SRAM)
	DMA1_Channel4->CMAR = ((uint32_t)TxBuffer) & DMA_CMAR4_MA;
	
	/// DMA TX channel tunning
	DMA1_Channel4->CCR = (
		DMA_CCR4_PL_0 | 		///< Channel priority level = Medium
		DMA_CCR4_MINC | 		///< Memory increment mode = Enabled
		DMA_CCR4_DIR); 			///< Data transfer direction = Read from memory
	
	/// Tune baudrate
	float divider = (float)HSE_VALUE / (16 * baudrate);
	uint32_t integ = divider;
	uint32_t fract = 10 * (divider - integ);
	USART1->BRR = (integ << 4) | (fract & 0x0F);
	
	/// CR1
	USART1->CR1 = 
		USART_CR1_UE | 			///< USART Enable
		USART_CR1_TE | 			///< Transmitter Enable
		USART_CR1_RE;			///< Receiver Enable
	
	/// CR2
	USART1->CR2 = 0;
	
	/// CR3
	USART1->CR3 = (
		USART_CR3_DMAT |		///< DMA enable transmitter
		USART_CR3_DMAR);		///< DMA enable receiver
	
	RxHead = RxBuffer;
}


/**
* @brief USART1 interrupt handler
*/
void Uart::UART1_Handler()
{
	Uart1.Handler();
}


/**
* @brief Interrupt handler
*/
void Uart::Handler()
{
	if(USART1->SR & USART_SR_TC)
	{
		USART1->SR &= ~USART_SR_TC;
		Board::SetRead485();
	}
}


/**
* @brief Data transmission
* @param data - data for transmission
* @param count - bytes count
*/
void Uart::Transmit(const char* data, uint16_t count)
{
	/// Limit data size
	uint16_t remain = count > TX_SIZE ? TX_SIZE : count;
	
	/// Wait until the transfer is complete
	while(DMA1_Channel4->CNDTR & DMA_CNDTR4_NDT);
	
	/// Disable channel
	if(DMA1_Channel4->CCR & DMA_CCR4_EN)
	{
		DMA1_Channel4->CCR &= ~DMA_CCR4_EN;
	}
	
	/// Copy data to buffer, set size
	memcpy(TxBuffer, data, remain);
	DMA1_Channel4->CNDTR = remain & DMA_CNDTR4_NDT;
	
	/// Allow interrupt on data register devastation, enable RS485 driver
	USART1->SR &= ~USART_SR_TC;
	USART1->CR1 |= USART_CR1_TCIE;
	Board::SetWrite485();
	
	/// Start transmission
	DMA1_Channel4->CCR |= DMA_CCR4_EN;
}


/**
* @brief Received data copying
* @param buffer - destination buffer pointer
* @param size - buffer size
*/
uint16_t Uart::CopyReceivedData(char* buffer, uint16_t size)
{
	/// Memorize tail position
	uint32_t tail = (uint32_t)(&RxBuffer[RX_SIZE - DMA1_Channel5->CNDTR]);
	
	/// If the buffer is not looped
	if(tail >= (uint32_t)RxHead)
	{
		/// Copy data
		uint16_t count = tail - (uint32_t)RxHead;
		
		/// If buffer size is insufficient, limit data size
		if(size < count)
		{
			count = size;
		}
		
		memcpy(buffer, RxHead, count);
		return count;
	}
	
	/// If the buffer is looped
	else
	{
		/// Copy a part from the end and a part from the beginning
		uint16_t count1 = &RxBuffer[RX_SIZE] - RxHead;
		uint16_t count2 = tail - (uint32_t)RxBuffer;
		
		/// If buffer size is insufficient, limit data size
		if(size <= count1)
		{
			count1 = size;
			count2 = 0;
			memcpy(buffer, RxHead, count1);
		}
		else if(size < (count1 + count2))
		{
			count2 = (count1 + count2) - size;
			memcpy(&buffer[count1], RxBuffer, count2);
		}
		else
		{
			memcpy(buffer, RxHead, count1);
			memcpy(&buffer[count1], RxBuffer, count2);
		}
		
		return count1 + count2;
	}
}


/**
* @brief Received data deletion
* @param count - bytes count to delete
*/
void Uart::DeleteReceivedData(uint16_t count)
{
	if((RxHead + count) < &RxBuffer[RX_SIZE])
	{
		RxHead += count;
	}
	else
	{
		RxHead = RxBuffer + (count - (&RxBuffer[RX_SIZE] - RxHead));
	}
}


/**
* @brief Checking, whether the receive buffer is empty
* @return true, if the buffer is empty
*/
bool Uart::IsReceiverEmpty()
{
	uint32_t tail = (uint32_t)(&RxBuffer[RX_SIZE - DMA1_Channel5->CNDTR]);
	return ((uint32_t)RxHead == tail);
}


/**
* @brief Receive buffer cleaning
*/
void Uart::Flush()
{
	/// Memorize tail position
	uint32_t tail = (uint32_t)(&RxBuffer[RX_SIZE - DMA1_Channel5->CNDTR]);
	
	uint16_t count = 0;
	
	/// If the buffer is not looped
	if(tail >= (uint32_t)RxHead)
	{
		/// Count data
		count = tail - (uint32_t)RxHead;
	}
	
	/// If the buffer is looped
	else
	{
		/// Count a part from the end and a part from the beginning
		uint16_t count1 = &RxBuffer[RX_SIZE] - RxHead;
		uint16_t count2 = tail - (uint32_t)RxBuffer;
		count = count1 + count2;
	}
	
	DeleteReceivedData(count);
}
