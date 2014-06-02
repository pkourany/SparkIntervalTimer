/* Copyright (c) 2014 Paul Kourany, based on work by Dianel Gilbert

Copyright (c) 2013 Daniel Gilbert, loglow@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "SparkIntervalTimer.h"

// ------------------------------------------------------------
// static class variables need to be reiterated here before use
// ------------------------------------------------------------
bool IntervalTimer::SIT_used[];
IntervalTimer::ISRcallback IntervalTimer::SIT_CALLBACK[];

// ------------------------------------------------------------
// Define interval timer ISR hooks for three available timers
// TIM2, TIM3 and TIM4 with callbacks to user code.
// These default as infinite loop stubs by Spark
// ------------------------------------------------------------
extern "C" void Wiring_TIM2_Interrupt_Handler_override()
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		//ISR for TIM2 code here
		IntervalTimer::SIT_CALLBACK[0]();
	}
}

extern "C" void Wiring_TIM3_Interrupt_Handler_override()
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		//ISR code for TIM3 here
		IntervalTimer::SIT_CALLBACK[1]();
	}
}

extern "C" void Wiring_TIM4_Interrupt_Handler_override()
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		//ISR code for TIM3 here
		IntervalTimer::SIT_CALLBACK[2]();
	}
}



// ------------------------------------------------------------
// this function inits and starts the timer, using the specified
// function as a callback and the period provided. must be passed
// the name of a function taking no arguments and returning void.
// make sure this function can complete within the time allowed.
// attempts to allocate a timer using available resources,
// returning true on success or false in case of failure.
// Period units is defined by scale, where scale = uSec or hmSec
// and = 1-65535 microsecond (uSec)
// or 1-65535 0.5ms increments (hmSec)
// ------------------------------------------------------------
bool IntervalTimer::beginCycles(void (*isrCallback)(), uint16_t newValue, bool scale) {

	// if this interval timer is already running, stop it
	if (status == TIMER_SIT) {
		stop_SIT();
		status = TIMER_OFF;
	}
	// store callback pointer
	myISRcallback = isrCallback;

	// attempt to allocate this timer
	if (allocate_SIT(newValue, scale)) status = TIMER_SIT;
	else status = TIMER_OFF;

	// check for success and return
	if (status != TIMER_OFF) return true;
	return false;

}


// ------------------------------------------------------------
// stop the timer if it's currently running, using its status
// to determine what hardware resources the timer may be using
// ------------------------------------------------------------
void IntervalTimer::end() {
	if (status == TIMER_SIT) stop_SIT();
	status = TIMER_OFF;
}



// ------------------------------------------------------------
// enables the SIT clock if not already enabled, then checks to
// see if any SITs are available for use. if one is available,
// it's initialized and started with the specified value, and
// the function returns true, otherwise it returns false
// ------------------------------------------------------------
bool IntervalTimer::allocate_SIT(uint16_t newValue, bool scale) {

	// check for an available SIT, and if so, start it
	for (uint8_t id = 0; id < NUM_SIT; id++) {
		if (!SIT_used[id]) {
			SIT_id = id;
			start_SIT(newValue, scale);
			SIT_used[id] = true;
			return true;
		}
	}
	// no SIT available
	return false;
}



// ------------------------------------------------------------
// configuters a SIT's TIMER registers, etc and enables
// interrupts, effectively starting the timer upon completion
// ------------------------------------------------------------
void IntervalTimer::start_SIT(uint16_t newValue, bool scale) {

	TIM_TimeBaseInitTypeDef timerInitStructure;
    NVIC_InitTypeDef nvicStructure;
	uint16_t prescaler;
	TIM_TypeDef* TIMx;
	
	//use SIT_id to identify TIM#
	switch (SIT_id) {
	case 0:		// TIM2
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
		TIMx = TIM2;
		break;
	case 1:		// TIM3
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
		TIMx = TIM3;
		break;
	case 2:		// TIM4
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		nvicStructure.NVIC_IRQChannel = TIM4_IRQn;
		TIMx = TIM4;
		break;
	}
	
	// Initialize Timer
	switch (scale) {
		case uSec:
			prescaler = SIT_PRESCALERu;	// Set prescaler for 1MHz clock, 1us period
			break;
		case hmSec:
			prescaler = SIT_PRESCALERm;	// Set prescaler for 2Hz clock, .5ms period
			break;
		default:
			scale == uSec;				// Default to microseconds
			prescaler = SIT_PRESCALERu;
			break;
	}

	timerInitStructure.TIM_Prescaler = prescaler;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = newValue;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIMx, &timerInitStructure);
	TIM_Cmd(TIMx, ENABLE);
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);

	// point to the correct SIT ISR
	SIT_CALLBACK[SIT_id] = myISRcallback;

	//Enable Timer Interrupt
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}


// ------------------------------------------------------------
// stops an active SIT by disabling its interrupt and TIMER
// and freeing up its state for future use.
// ------------------------------------------------------------
void IntervalTimer::stop_SIT() {

    NVIC_InitTypeDef nvicStructure;
	TIM_TypeDef* TIMx;


	//use SIT_id to identify TIM#
	switch (SIT_id) {
	case 0:		// TIM2
		nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
		TIMx = TIM2;
		break;
	case 1:		// TIM3
		nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
		TIMx = TIM3;
		break;
	case 2:		// TIM4
		nvicStructure.NVIC_IRQChannel = TIM4_IRQn;
		TIMx = TIM4;
		break;
	}
	// disable counter
	TIM_Cmd(TIMx, DISABLE);
	
	// disable interrupt
    nvicStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&nvicStructure);
	
	// disable timer peripheral
	TIM_DeInit(TIMx);
	
	// free SIT for future use
	SIT_used[SIT_id] = false;
}

// ------------------------------------------------------------
// Enables or disables an active SIT's interrupt without
// removing the SIT.
// ------------------------------------------------------------
void IntervalTimer::interrupt_SIT(action ACT)
{
    NVIC_InitTypeDef nvicStructure;
	TIM_TypeDef* TIMx;

	//use SIT_id to identify TIM#
	switch (SIT_id) {
	case 0:		// TIM2
		nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
		TIMx = TIM2;
		break;
	case 1:		// TIM3
		nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
		TIMx = TIM3;
		break;
	case 2:		// TIM4
		nvicStructure.NVIC_IRQChannel = TIM4_IRQn;
		TIMx = TIM4;
		break;
	}

	switch (ACT) {
	case INT_ENABLE:
		//Enable Timer Interrupt
		nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
		nvicStructure.NVIC_IRQChannelSubPriority = 1;
		nvicStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicStructure);
		break;
	case INT_DISABLE:
		// disable interrupt
		nvicStructure.NVIC_IRQChannelCmd = DISABLE;
		NVIC_Init(&nvicStructure);
		break;
	default:
		//Do nothing
		break;
	}
}
