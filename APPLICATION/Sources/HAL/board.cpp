/**
* @file board.cpp
* @brief Target board implementation
*/
#include "board.hpp"
#include "stm32l1xx.h"                  // Device header


/**
* @brief Target board initalization
*/
void Board::Init()
{
	/// Run peripheral modules
	RCC->AHBRSTR |= RCC_AHBRSTR_GPIOARST;
	RCC->AHBRSTR &= ~RCC_AHBRSTR_GPIOARST;
	RCC->APB1RSTR |= RCC_APB1RSTR_PWRRST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_PWRRST;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	
	///--- USART1 ---///
	/// PA8 - 485R/W (MODE = 01)
	/// Output, Push-Pull, Max. output speed 2 MHz
	GPIOA->MODER &= ~GPIO_MODER_MODER8;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_8;
	GPIOA->BSRR = GPIO_BSRR_BR_8;
	
	/// PA9 - 485TX (MODE = 10)
	GPIOA->MODER &= ~GPIO_MODER_MODER9;
	GPIOA->MODER |= GPIO_MODER_MODER9_1;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFRH1;
	GPIOA->AFR[1] |= 7 << ((9 - 8) * 4);
	
	/// PA10 - 485RX (MODE = 10)
	GPIOA->MODER &= ~GPIO_MODER_MODER10;
	GPIOA->MODER |= GPIO_MODER_MODER10_1;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_10;
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFRH2;
	GPIOA->AFR[1] |= 7 << ((10 - 8) * 4);
	
	
	///--- USART2 ---///
	/// PA2(USART2_TX) - ISO7816 IO (MODE = 10)
	GPIOA->MODER &= ~GPIO_MODER_MODER2;
	GPIOA->MODER |= GPIO_MODER_MODER2_1;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_2;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL2;
	GPIOA->AFR[0] |= 7 << ((2 - 0) * 4);
	
	/// PA3(USART2_RX) - not connected, but tune as UART
	GPIOA->MODER &= ~GPIO_MODER_MODER3;
	GPIOA->MODER |= GPIO_MODER_MODER3_1;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_3;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL3;
	GPIOA->AFR[0] |= 7 << ((3 - 0) * 4);
	
	/// PA4(USART2_CK) - ISO7816 CLK
	GPIOA->MODER &= ~GPIO_MODER_MODER4;
	GPIOA->MODER |= GPIO_MODER_MODER4_1;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_4;
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL4;
	GPIOA->AFR[0] |= 7 << ((4 - 0) * 4);
	
	/// PA5 - ISO7816 VCC (MODE = 01)
	/// Output, Push-Pull, Max. output speed 2 MHz
	GPIOA->MODER &= ~GPIO_MODER_MODER5;
	GPIOA->MODER |= GPIO_MODER_MODER5_0;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5;
	GPIOA->BSRR = GPIO_BSRR_BR_5;
	
	/// PA6 - ISO7816 RST (MODE = 01)
	/// Output, Push-Pull, Max. output speed 2 MHz
	GPIOA->MODER &= ~GPIO_MODER_MODER6;
	GPIOA->MODER |= GPIO_MODER_MODER6_0;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_6;
	GPIOA->BSRR = GPIO_BSRR_BR_6;
}


/**
* @brief RS485 Read (RE receiver enable)
*/
void Board::SetRead485()
{
	GPIOA->BSRR = GPIO_BSRR_BR_8;
}


/**
* @brief RS485 Write (DE driver enable)
*/
void Board::SetWrite485()
{
	GPIOA->BSRR = GPIO_BSRR_BS_8;
}


/**
* @brief Set ISO-7816 VCC high
*/
void Board::Set_ISO7816_VCC_High()
{
	GPIOA->BSRR = GPIO_BSRR_BS_5;
}


/**
* @brief Set ISO-7816 VCC low
*/
void Board::Set_ISO7816_VCC_Low()
{
	GPIOA->BSRR = GPIO_BSRR_BR_5 >> 16;
}


/**
* @brief Set ISO-7816 RST high
*/
void Board::Set_ISO7816_RST_High()
{
	GPIOA->BSRR = GPIO_BSRR_BS_6;
}


/**
* @brief Set ISO-7816 RST low
*/
void Board::Set_ISO7816_RST_Low()
{
	GPIOA->BSRR = GPIO_BSRR_BR_6;
}
