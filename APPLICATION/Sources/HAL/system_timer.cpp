/**
* @file system_timer.cpp
* @brief System timer implementation
*/

#include "system_timer.hpp"
#include "core.hpp"
#include "stm32l1xx.h"                  // Device header


uint32_t SystemTimer::SecCounter;

/**
* @brief System timer initialization
*/
void SystemTimer::Init()
{
	/// Enable clocking
	RCC->APB2RSTR |= RCC_APB2RSTR_TIM9RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM9RST;
	RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
	
	/// Tune prescalers and interrupt
	TIM9->PSC = (HSE_VALUE / 1000) - 1;
	TIM9->ARR = 1000;
	TIM9->CNT = 0;
	TIM9->CR1 |= TIM_CR1_ARPE;
	TIM9->EGR |= TIM_EGR_UG;
	TIM9->SR &= ~TIM_SR_UIF;
	TIM9->CR1 |= TIM_CR1_CEN;
	TIM9->DIER |= TIM_DIER_UIE;
	Core::RegIrqHandler(TIM9_IRQn, SystemTimer::Handler);
}


/**
* @brief Get current system timer value
* @return current system timer value (s.)
*/
uint32_t SystemTimer::GetTime()
{
	return SecCounter;
}


/**
* @brief System timer interrupt handler
*/
void SystemTimer::Handler()
{
	if(TIM9->SR & TIM_SR_UIF)
	{
		TIM9->SR &= ~TIM_SR_UIF;
		SecCounter++;
	}
}
