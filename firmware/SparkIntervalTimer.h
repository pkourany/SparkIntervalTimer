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


#ifndef __INTERVALTIMER_H__
#define __INTERVALTIMER_H__

#include "application.h"

#if defined(PLATFORM_ID)
#if (PLATFORM_ID == 0) // Core
  #define SYSCORECLOCK	72000000UL		// Core provides SYSCORECLOCK to all timers
#elif (PLATFORM_ID == 6) // Photon
  #define SYSCORECLOCK	60000000UL		// Photon provides SYSCORECLOCK/2 as root clock to timers (except TIMER1)
#else
  #error "*** PLATFORM_ID not supported by this library. PLATFORM should be Core or Photon ***"
#endif
#endif

enum {uSec, hmSec};			// microseconds or half-milliseconds
enum action {INT_DISABLE, INT_ENABLE};

#ifdef __cplusplus
extern "C" {
#endif


#if (PLATFORM_ID == 0)	//Core
extern void Wiring_TIM2_Interrupt_Handler_override(void);
extern void Wiring_TIM3_Interrupt_Handler_override(void);
extern void Wiring_TIM4_Interrupt_Handler_override(void);

enum TIMid {TIMER2, TIMER3, TIMER4, AUTO=255};
#elif (PLATFORM_ID == 6)	//Photon
extern void Wiring_TIM3_Interrupt_Handler_override(void);
extern void Wiring_TIM4_Interrupt_Handler_override(void);
extern void Wiring_TIM5_Interrupt_Handler_override(void);
extern void Wiring_TIM6_Interrupt_Handler_override(void);
extern void Wiring_TIM7_Interrupt_Handler_override(void);

enum TIMid {TIMER3, TIMER4, TIMER5, TIMER6, TIMER7, AUTO=255};
#endif

class IntervalTimer {
  private:
	typedef void (*ISRcallback)();
    enum {TIMER_OFF, TIMER_SIT};
#if (PLATFORM_ID == 0)	//Core
    static const uint8_t NUM_SIT = 3;
#elif (PLATFORM_ID == 6)	//Photon
    static const uint8_t NUM_SIT = 5;
#endif
	
	// Timer ClockDivision = DIV4
	const uint16_t SIT_PRESCALERu = (uint16_t)(SYSCORECLOCK / 1000000UL) - 1;	//To get TIM counter clock = 1MHz
	const uint16_t SIT_PRESCALERm = (uint16_t)(SYSCORECLOCK / 2000UL) - 1;		//To get TIM counter clock = 2KHz
    const uint16_t MAX_PERIOD = UINT16_MAX;		// 1-65535 us
	
    static bool SIT_used[NUM_SIT];
    bool allocate_SIT(uint16_t Period, bool scale, TIMid id);
    void start_SIT(uint16_t Period, bool scale);
    void stop_SIT();
    bool status;
    uint8_t SIT_id;
 	ISRcallback myISRcallback;
	
    bool beginCycles(void (*isrCallback)(), uint16_t Period, bool scale, TIMid id);
	
  public:
    IntervalTimer() {
	status = TIMER_OFF;
	
	for (int i=0; i < NUM_SIT; i++)		//Set all SIT slots to unused
		SIT_used[i] = false;
	
	// Attach timer interrupt handlers
#if defined (STM32F10X_MD)	//Core
	if (!attachSystemInterrupt(SysInterrupt_TIM2_Update, Wiring_TIM2_Interrupt_Handler_override)) ;	//error
	if (!attachSystemInterrupt(SysInterrupt_TIM3_Update, Wiring_TIM3_Interrupt_Handler_override)) ;	//error
	if (!attachSystemInterrupt(SysInterrupt_TIM4_Update, Wiring_TIM4_Interrupt_Handler_override)) ;	//error
#elif defined (STM32F2XX)	//Photon+
	if (!attachSystemInterrupt(SysInterrupt_TIM3_Update, Wiring_TIM3_Interrupt_Handler_override)) ;	//error
	if (!attachSystemInterrupt(SysInterrupt_TIM4_Update, Wiring_TIM4_Interrupt_Handler_override)) ;	//error
	if (!attachSystemInterrupt(SysInterrupt_TIM5_Update, Wiring_TIM5_Interrupt_Handler_override)) ;	//error
	if (!attachSystemInterrupt(SysInterrupt_TIM6_Update, Wiring_TIM6_Interrupt_Handler_override)) digitalWrite(D7,HIGH);	//error
	if (!attachSystemInterrupt(SysInterrupt_TIM7_Update, Wiring_TIM7_Interrupt_Handler_override)) digitalWrite(D5,HIGH);	//error
#endif

/*
    SysInterrupt_TIM1_BRK_TIM9_IRQ,
    SysInterrupt_TIM1_Break,
    SysInterrupt_TIM9_Compare1,
    SysInterrupt_TIM9_Compare2,
    SysInterrupt_TIM9_Update,
    SysInterrupt_TIM9_Trigger,
    SysInterrupt_TIM1_UP_TIM10_IRQ,
    SysInterrupt_TIM1_Update,
    SysInterrupt_TIM10_Compare,
    SysInterrupt_TIM10_Update,
    SysInterrupt_TIM8_BRK_TIM12_IRQ,
    SysInterrupt_TIM8_Break,
    SysInterrupt_TIM12_Compare1,
    SysInterrupt_TIM12_Compare2,
    SysInterrupt_TIM12_Update,
    SysInterrupt_TIM12_Trigger,
    SysInterrupt_TIM8_UP_TIM13_IRQ,
    SysInterrupt_TIM8_Update,
    SysInterrupt_TIM13_Compare,
    SysInterrupt_TIM13_Update,
    SysInterrupt_TIM8_TRG_COM_TIM14_IRQ,
    SysInterrupt_TIM8_Trigger,
    SysInterrupt_TIM14_COM,
    SysInterrupt_TIM14_Compare,
    SysInterrupt_TIM14_Update,
    SysInterrupt_TIM8_IRQ,
    SysInterrupt_TIM8_Compare1,
    SysInterrupt_TIM8_Compare2,
    SysInterrupt_TIM8_Compare3,
    SysInterrupt_TIM1_TRG_COM_TIM11_IRQ,
    SysInterrupt_TIM1_Trigger,
    SysInterrupt_TIM1_COM,
    SysInterrupt_TIM11_Compare,
    SysInterrupt_TIM11_Update,
*/

    }
	
    ~IntervalTimer() { end(); }
	
    bool begin(void (*isrCallback)(), uint16_t Period, bool scale) {
		if (Period == 0 || Period > MAX_PERIOD)
			return false;
		return beginCycles(isrCallback, Period, scale, AUTO);
    }
	
    bool begin(void (*isrCallback)(), uint16_t Period, bool scale, TIMid id) {
		if (Period == 0 || Period > MAX_PERIOD)
			return false;
		return beginCycles(isrCallback, Period, scale, id);
    }
	
    void end();
	void interrupt_SIT(action ACT);
	void resetPeriod_SIT(uint16_t newPeriod, bool scale);
	int8_t isAllocated_SIT(void);
	
    static ISRcallback SIT_CALLBACK[NUM_SIT];
};


#ifdef __cplusplus
}
#endif

#endif
