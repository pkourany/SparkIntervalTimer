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


enum {uSec, hmSec};			// microseconds or half-milliseconds

#ifdef __cplusplus
extern "C" {
#endif

extern void (*Wiring_TIM2_Interrupt_Handler)(void);
extern void (*Wiring_TIM3_Interrupt_Handler)(void);
extern void (*Wiring_TIM4_Interrupt_Handler)(void);

extern void Wiring_TIM2_Interrupt_Handler_override(void);
extern void Wiring_TIM3_Interrupt_Handler_override(void);
extern void Wiring_TIM4_Interrupt_Handler_override(void);

enum action {INT_DISABLE, INT_ENABLE};
enum TIMid {TIMER2, TIMER3, TIMER4, AUTO=255};

class IntervalTimer {
  private:
	typedef void (*ISRcallback)();
    enum {TIMER_OFF, TIMER_SIT};
    static const uint8_t NUM_SIT = 3;
	
	// Timer ClockDivision = DIV4
	const uint16_t SIT_PRESCALERu = (uint16_t)(SystemCoreClock / 1000000) - 1;	//To get TIM counter clock = 1MHz
	const uint16_t SIT_PRESCALERm = (uint16_t)(SystemCoreClock / 2000) - 1;		//To get TIM counter clock = 2KHz
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
        Wiring_TIM2_Interrupt_Handler = Wiring_TIM2_Interrupt_Handler_override;
        Wiring_TIM3_Interrupt_Handler = Wiring_TIM3_Interrupt_Handler_override;
        Wiring_TIM4_Interrupt_Handler = Wiring_TIM4_Interrupt_Handler_override;
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