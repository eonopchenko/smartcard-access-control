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
	ISO7816_RX_BUFFER_SIZE = 64,			///< TX buffer size
	ISO7816_TX_BUFFER_SIZE = 64,			///< RX buffer size
	ISO7816_ETU = 372,						///< Elementary Time Unit (ISO7816-3 3.1.a)
	ISO7816_FREQUENCY = 3000000,			///< Basic frequency (Hz) (baudrate 3600000 / 372 = 9677 baud)
	ISO7816_T3_TICKS = 40000,				///< Delay before reset procedure (tact count) (t3 (ISO7816-3 3.2.b))
	ISO7816_BIT_CONVETNTION_DIRECT = 0x3B,	///< Data polarity - direct
	ISO7816_PROTOCOL_T0 = 0x00,				///< Protocol - T0 (asynchronous, half-duplex, character)
};


/// Fi table
const uint16_t ISO7816_FI[] = 
{
	0,
	512,
	768,
	1024,
	1536,
	2048,
	0,
	0,
};


/// Di table
const uint8_t ISO7816_DI[] = 
{
	0,
	0,
	2,
	4,
	8,
	16,
	32,
	64,
};


/// Instructions list
enum INS_t
{
	SELECT_FILE		= 0xA4,
	READ_BINARY		= 0xB0,
	GET_RESPONSE	= 0xC0,
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
		__IO uint8_t data = USART2->DR;
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
	{
		RxBuffer->Reset();
	}
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
	{
		RxBuffer->Delete(count);
	}
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
* @brief Smart card activation (cold reset + interface adjustment)
* @param pAtr - ATR structure pointer (is used if ATR return needed)
* @return true, if operation successful
*/
bool ISO7816::ActivateCard(ATR_t* pAtr)
{
	RCC->APB2RSTR |= RCC_APB1RSTR_USART2RST;
	RCC->APB2RSTR &= ~RCC_APB1RSTR_USART2RST;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	Core::RegIrqHandler(USART2_IRQn, ISO7816_1_Handler);
	
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_PCE | USART_CR1_M | USART_CR1_UE;
	USART2->CR2 = USART_CR2_LBCL | USART_CR2_CLKEN | USART_CR2_STOP;
	USART2->CR3 = USART_CR3_NACK | USART_CR3_SCEN;
	
	/// Guard Time 16 bits (GTPR = 16 << 8)
	/// Clock last bit (USART_CR2_LBCL = 0)
	/// Polarity low (USART_CR2_CPHA = 0)
	/// Phase on front (USART_CR2_CPOL = 0)
	/// Frame length 9 bits (8 data bits + 1 parity) (USART_CR1_M = 1)
	uint32_t baud = ISO7816_FREQUENCY / ISO7816_ETU;
	uint32_t integerDivider = HSE_VALUE / (16 * baud);
	uint32_t fractionalDivider = (10000 * (HSE_VALUE - (16 * integerDivider))) / (16 * baud);
	USART2->BRR = (integerDivider << 4) | (fractionalDivider & 0x0F);
	USART2->GTPR = (16 << 8) | ((HSE_VALUE / ISO7816_FREQUENCY) / 2);
	
	/// ATR reading cycle
	ATR_t atr;
	
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
		return false;
	}
	
	/// Read ATR
	memset(&atr, 0, sizeof(ATR_t));
	if(!GetReceivedChar((char *)&atr.TS))							return false;
	if(!GetReceivedChar((char *)&atr.T0))							return false;
	atr.Hlength = atr.T0 & 0x0F;
	if((atr.T0 & (1 << 4)) && !GetReceivedChar((char *)&atr.TA1))	return false;
	if((atr.T0 & (1 << 5)) && !GetReceivedChar((char *)&atr.TB1))	return false;
	if((atr.T0 & (1 << 6)) && !GetReceivedChar((char *)&atr.TC1))	return false;
	if((atr.T0 & (1 << 7)) && !GetReceivedChar((char *)&atr.TD1))	return false;
	if((atr.TD1 & (1 << 4)) && !GetReceivedChar((char *)&atr.TA2))	return false;
	if((atr.TD1 & (1 << 5)) && !GetReceivedChar((char *)&atr.TB2))	return false;
	if((atr.TD1 & (1 << 6)) && !GetReceivedChar((char *)&atr.TC2))	return false;
	if((atr.TD1 & (1 << 7)) && !GetReceivedChar((char *)&atr.TD2))	return false;
	
	/// Read historical bytes (H)
	if(atr.Hlength)
	{
		bool response = true;
		for(uint8_t index = 0; index < atr.Hlength; index++)
		{
			if(!GetReceivedChar((char *)&atr.H[index]))
			{
				response = false;
				break;
			}
		}
		
		if(!response)
		{
			return false;
		}
	}
	
	if((atr.TS != ISO7816_BIT_CONVETNTION_DIRECT) || ((atr.TD1 & 0x0F) != ISO7816_PROTOCOL_T0))
	{
		return false;
	}
	
	/// No TA2 in ATR (4th digit in TD1)
	/// Negotiable mode
	if(!(atr.TD1 & (1 << 4)))
	{
		char buffer1[4];
		/*	PTSS	*/	buffer1[0] = 0xFF;
		/*	PTS0	*/	buffer1[1] = (1 << 4) | (atr.T0 & (1 << 7) ? (atr.TD1 & 0x0F) : 0x00);
		/*	PTS1	*/	buffer1[2] = atr.TA1;
		/*	PCK		*/	buffer1[3] = CalcCK(buffer1, 3);
		
		Transmit(buffer1, 4);
		
		char buffer2[4];
		memset(buffer2, 0, 4);
		if(!GetReceivedChar(&buffer2[0]))	return false;
		if(!GetReceivedChar(&buffer2[1]))	return false;
		if(!GetReceivedChar(&buffer2[2]))	return false;
		if(!GetReceivedChar(&buffer2[3]))	return false;
		
		/// Compare transmitted and received data
		if(memcmp((char *)&buffer1, buffer2, 4))
		{
			/// If data is different, return error
			return false;
		}
		
		/// Get Fi and Di parameters
		uint16_t fi = ISO7816_FI[(atr.TA1 >> 4) & 0x07];
		uint8_t di = ISO7816_DI[atr.TA1 & 0x07];
		
		/// Reconfigure USART
		baud = ISO7816_FREQUENCY / (fi / di);
		integerDivider = HSE_VALUE / (16 * baud);
		fractionalDivider = (10000 * (HSE_VALUE - (16 * integerDivider))) / (16 * baud);
		USART2->BRR = (integerDivider << 4) | (fractionalDivider & 0x0F);
		
		if(pAtr)
		{
			memcpy(pAtr, &atr, sizeof(ATR_t));
		}
		
		return true;
	}
	
	return false;
}


/**
* @brief Smart card deactivation
* @return true, if operation successful
*/
bool ISO7816::DeactivateCard()
{
	/// Deactivate interface
	RCC->APB2RSTR |= RCC_APB1RSTR_USART2RST;
	RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
	Core::UnregIrqHandler(USART2_IRQn);
	Board::Set_ISO7816_RST_Low();
	Board::Set_ISO7816_VCC_Low();
	return true;
}


/**
* @brief Send TDPU
* @param tpdu - TPDU structure pointer
* @param data - source data pointer
* @param count - bytes count
* @param exchangeCase - exchange case (see enum Case_t)
* @return SW1, SW2 result of operation
*/
uint16_t ISO7816::SendTPDU(const TPDU_t* tpdu, const void* data, uint8_t count, Case_t exchangeCase)
{
	/// Choose case, assemble frame and transmit
	uint16_t result = 0;
	switch(exchangeCase)
	{
		case CASE_1:
		case CASE_2:
		{
			Transmit((const char *)tpdu, sizeof(*tpdu));
			Transmit((const char *)&count, 1);
			
			/// Get status word
			uint8_t sw[2];
			if(!GetReceivedChar((char *)&sw[0]))	return 0;
			if(!GetReceivedChar((char *)&sw[1]))	return 0;
			
			result = (int16_t)((sw[0] << 8) | sw[1]);
			
			break;
		}
		
		case CASE_3:
		case CASE_4:
		{
			Transmit((const char *)tpdu, sizeof(*tpdu));
			Transmit((const char *)&count, 1);
			
			/// Get procedure byte
			uint8_t pb = 0;
			if(!GetReceivedChar((char *)&pb))
			{
				return 0;
			}
			
			/// Wrong procedure byte
			if(pb != tpdu->INS)
			{
				/// Return error
				return 0;
			}
			
			Transmit((const char *)data, count);
			
			/// Get status word
			uint8_t sw[2];
			if(!GetReceivedChar((char *)&sw[0]))	return 0;
			if(!GetReceivedChar((char *)&sw[1]))	return 0;
			
			result = (int16_t)((sw[0] << 8) | sw[1]);
			
			break;
		}
		
		default:
		{
			return 0;
		}
	}
	
	return result;
}


/**
* @brief SELECT FILE
* @param cla - class byte (0xA0 for SIM-cards, 0x00 for general) (ISO 7816-4 5.4.1 Class byte)
* @param p1 - P1 APDU parameter
* @param p2 - P2 APDU parameter
* @param fileName - requested file name
* @param nameLength - requested file name length
* @return true, if operation successful
*/
bool ISO7816::SelectFile(uint8_t cla, uint8_t p1, uint8_t p2, const char* fileName, uint8_t nameLength)
{
	TPDU_t tpdu;
	
	tpdu.CLA = cla;
	tpdu.INS = SELECT_FILE;
	tpdu.P1 = p1;
	tpdu.P2 = p2;
	
	uint16_t result = SendTPDU(&tpdu, fileName, nameLength, CASE_3);
	
	/// Check result
	if(((result & 0xFF00) != 0x9000) && ((result & 0xFF00) != 0x9F00))
	{
		return false;
	}
	
	return true;
}


/**
* @brief READ BINARY
* @param cla - class byte (0xA0 for SIM-cards, 0x00 for general) (ISO 7816-4 5.4.1 Class byte)
* @param buffer - destination buffer pointer
* @param count - requested bytes count
* @return read bytes count or -1 if error
*/
int_fast8_t ISO7816::ReadBinary(uint8_t cla, void* buffer, uint8_t count)
{
	TPDU_t tpdu;
	tpdu.CLA = cla;
	tpdu.INS = READ_BINARY;
	tpdu.P1 = 0x00;
	tpdu.P2 = 0x00;
	
	Transmit((const char *)&tpdu, sizeof(tpdu));
	Transmit((const char *)&count, 1);
	
	/// Get procedure byte
	uint8_t pb = 0;
	if(!GetReceivedChar((char *)&pb))
	{
		return -1;
	}
	
	/// Wrong procedure byte
	if(pb != tpdu.INS)
	{
		/// Return error
		return -1;
	}
	
	int_fast8_t size = count;
	char* ptr = (char *)buffer;
	while(count--)
	{
		if(!GetReceivedChar(ptr++))
		{
			return -1;
		}
	}
	
	/// Get status word
	uint8_t sw[2];
	if(!GetReceivedChar((char *)&sw[0]))	return -1;
	if(!GetReceivedChar((char *)&sw[1]))	return -1;
	
	uint16_t result = (int16_t)((sw[0] << 8) | sw[1]);
	
	/// Check result
	if(((result & 0xFF00) != 0x9000) && ((result & 0xFF00) != 0x9F00))
	{
		return -1;
	}
	
	return size;
}


/**
* @brief GET RESPONSE
* @param cla - class byte (0xA0 for SIM-cards, 0x00 for general) (ISO 7816-4 5.4.1 Class byte)
* @param buffer - destination buffer pointer
* @param count - requested bytes count
* @return read bytes count or -1 if error
*/
int_fast8_t ISO7816::GetResponse(uint8_t cla, void* buffer, uint8_t count)
{
	TPDU_t tpdu;
	tpdu.CLA = cla;
	tpdu.INS = GET_RESPONSE;
	tpdu.P1 = 0x00;
	tpdu.P2 = 0x00;
	
	Transmit((const char *)&tpdu, sizeof(tpdu));
	Transmit((const char *)&count, 1);
	
	/// Get procedure byte
	uint8_t pb = 0;
	if(!GetReceivedChar((char *)&pb))
	{
		return -1;
	}
	
	/// Wrong procedure byte
	if(pb != tpdu.INS)
	{
		/// Return error
		return -1;
	}
	
	int_fast8_t size = count;
	char* ptr = (char *)buffer;
	while(count--)
	{
		if(!GetReceivedChar(ptr++))
		{
			return -1;
		}
	}
	
	/// Get status word
	uint8_t sw[2];
	if(!GetReceivedChar((char *)&sw[0]))	return -1;
	if(!GetReceivedChar((char *)&sw[1]))	return -1;
	
	uint16_t result = (int16_t)((sw[0] << 8) | sw[1]);
	
	/// Check result
	if(((result & 0xFF00) != 0x9000) && ((result & 0xFF00) != 0x9F00))
	{
		return -1;
	}
	
	return size;
}


/**
* @brief Calculate TCK and PCK
* @param data - input data pointer
* @param count - bytes count
* @return TCK or PCK
*/
uint8_t ISO7816::CalcCK(char* data, uint8_t count)
{
	char* ptr = data;
	uint8_t ck = *ptr++;
	
	while(ptr - data < count)
	{
		ck ^= *ptr++;
	}
	
	return ck;
}
