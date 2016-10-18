/**
* @file startup.cpp
* @brief Start initialization
*/

#include "stm32l1xx.h"                  // Device header

extern "C" void __main();
extern "C" void init() __attribute__((section(".init_section")));

/**
* @brief Start initialization
*/
extern "C" void init(void)
{
	/// Enable HSE
	RCC->CR |= RCC_CR_HSEON;
	
	/// Wait until HSE readiness
	while(!(RCC->CR & RCC_CR_HSERDY));
	
	/// Choose HSE as the main clock source
	RCC->CFGR |= RCC_CFGR_SW_HSE;
	
	/// Enable LSI
	RCC->CSR |= RCC_CSR_LSION;
	
	/// Wait until LSI readiness
	while(!(RCC->CSR & RCC_CSR_LSIRDY));
	
	/// Set prescalers
	RCC->CFGR &= ~(RCC_CFGR_PPRE2 | RCC_CFGR_PPRE1 | RCC_CFGR_HPRE);
	
	/// Enable power interface
	RCC->APB1RSTR |= RCC_APB1RSTR_PWRRST;
	RCC->APB1RSTR &= ~RCC_APB1RSTR_PWRRST;
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	
	/// Allow BDCR register writing
	PWR->CR |= PWR_CR_DBP;
	
	/// Disable LSE
	RCC->CSR &= ~RCC_CSR_LSEON;
	
	/// Enable RTC
	RCC->CSR |= RCC_CSR_RTCRST | RCC_CSR_RTCEN | RCC_CSR_RTCSEL_1 | RCC_CSR_RTCSEL_0;
	RCC->CSR |= RCC_CSR_RMVF;
	
	/// Cancel BDCR register writing
	PWR->CR &= ~PWR_CR_DBP;
	
	/// Enable HSE failure protection
	/// (unmasked NMI interrupt)
	RCC->CR |= RCC_CR_CSSON;
	
	/// Copy vector table to SRAM
	for(uint16_t index = 0; index < 256; index++)
	{
		*(volatile char *)(SRAM_BASE + index) = *(volatile char *)(FLASH_BASE + index);
	}
	
	/// Enable MEMORY MAPPING
	SYSCFG->MEMRMP |= SYSCFG_MEMRMP_MEM_MODE_1 | SYSCFG_MEMRMP_MEM_MODE_0;
	
	/// Start static objects initialization
	__main();
}
