/* Copyright (c) 2014 Paul Kourany, based on work by Dianel Gilbert

 - UPDATED Jan 29, 2016 - update to Libraries 2.0
 - UPDATED Jan 5, 2016 - Udpated/fixed example
 - UPDATED Sept 3, 2015 - Added support for Particle Photon
 - BUG FIX Oct 5, 2015 - Fix for manual/auto SIT allocation (thanks @lige!)`

Copyright (c) 2013 Daniel Gilbert, loglow@gmail.com
Copyright (c) 2014 Paul Kourany, pkourany@gmail.com

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

SparkIntervalTimer 
==================

IntervalTimer uses interrupts to call a function at a precise timing 
interval. Advanced programming is required to properly use 
IntervalTimer, because your function runs as an interrupt. See below for 
details. 

IntervalTimer is generally recommended for use only in libraries and 
advanced applications which require highly precise timing. Usually 
elapsedMillis variables are easier to use, because they avoid the 
pitfalls of interrupt programming. 

1. Using Hardware Timers 
------------------------

Up to 5 (3 on the Core) IntervalTimer objects may be active simultaneously. The
Photon hardware timers TMR3, TMR4, TMR5, TMR6 and TMR7 will allocated while the Core 
hardware timers TMR2, TMR3 and TMR4 will be allocated as required (in those orders).

Hardware timers are used for providing PWM output via the analogWrite() function.
Allocating a hardware timer will disable PWM capabilities to certain pins based
on the timer allocated as follows:

```
CORE:
PIN		TMR2	TMR3	TMR4
----------------------------
D0						 x
D1						 x
A0		 x
A1		 x
A4				 x
A5				 x
A6				 x
A7 				 x

PHOTON:
PIN		TMR3	TMR4	TMR5	TMR6	TMR7
--------------------------------------------
D0			  	 x
D1			  	 x
D2		 x
D3		 x
A4		 x
A5		 x
WKP					     x

```
Note that digital I/O (read/write) will still functions on the affected pins.  Also not that on the Photon, TMR6 and TMR7 are not mapped to any I/O pins.

2. IntervalTimer Usage 
----------------------

```
IntervalTimer myTimer;
```
Create an IntervalTimer object. You may create as many IntervalTimers as 
needed, but only a limited number (3 on Core, 5 on Photon) may be active
simultaneously. Normally IntervalTimer objects should be created as global
variables. 


```
myTimer.begin(function, time, timebase);  		//AUTO allocate timer
```
Auto-allocate a timer from the pool and start it. The interval is specified in 
microseconds or (half) milliseconds based on the selected timebase: uSec for 
microseconds and hmSec for half-milliseconds. The time specified as a uint16_t.
The function returns true if successful. False is returned if all hardware
timer resources are allocated and used by other IntervalTimer objects.

The functions specified to be called by IntervalTimer should be short, run as
quickly as possible, and should avoid calling other functions if possible.


```
myTimer.begin(function, time, timebase, id);  //MANUALLY allocate timer
```
Manually allocate a timer from the pool by specifying its id and start it.
The specified id corresponds to a hardware timer - Core = TIMER2, TIMER3
or TIMER4 and Photon = TIMER3, TIMER4, TIMER5, TIMER6, TIMER7.The remaining
parameters are the same as above.


```
myTimer.resetPeriod_SIT(time, timebase);
```
Reset the timer's time and timebase.  Timer will be stopped and restarted with
new settings.  See above for parameter details.


```
myTimer.interrupt_SIT(action);
```
Enables (action = INT_ENABLE) or disables (action = INT_DISABLE) an active
IntervalTimer's interrupts without deleting the object


```
myTimer.isAllocated_SIT();
```
Returns -1 if timer is not allocated or allocated timer id (Core = TIMER2,
TIMER3, TIMER4 and Photon = TIMER3, TIMER4, TIMER5, TIMER6, TIMER7).


```
myTimer.end();
```
Stop the timer (and interrupts) and deallocate it from the timer pool. The hardware
resource becomes available for use by other IntervalTimer objects. 


3. Example Program 
------------------

The included demo program will create 3 or 5 Interval Timers (maximum 
allowed on Core or Photon) to blink three LEDs at different intervals.
The first timer will blink the onboard LED while 2 (Core) or 4 (Photon)
extra LEDs (and small current limiting resistors) must be added by the
user on pins D3 and D4 on Core and D3, D4, D5 and D6 on Photon. After
100 blinks, Timer1 will reset to 1/4 of its interval (250ms) and
after 200 more blinks, Timer1 is shut down and will stop blinking.


4. Interrupt Context Issues 
---------------------------

IntervalTimer will call your function from interrupt context. Because it 
can interrupt your program at any moment, special design is necessary to 
share data with the rest of your program. Many ordinary functions are 
not designed to work properly from interrupt context. String objects 
should be avoided. A general guideline is to keep your function short 
and avoid calling other functions if possible. 

Variables usually need to be "volatile" types. Volatile tells the 
compiler to avoid optimizations that assume variable can not 
spontaneously change. Because your function may change variables while 
your program is using them, the compiler needs this hint. But volatile 
alone is often not enough. 

When accessing shared variables, usually interrupts must be disabled. 
Even with volatile, if the interrupt changes a multi-byte variable 
between a sequence of instructions, it can be read incorrectly. If your 
data is multiple variables, such as an array and a count, usually 
interrupts need to be disabled for the entire sequence of your code 
which accesses the data. 
