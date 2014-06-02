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

1. Compatibility 
----------------

Up to 3 IntervalTimer objects may be active simultaneously. The Core 
timers TMR2, TMR3 and TMR4 will be allocated as required.


2. IntervalTimer Usage 
----------------------

```IntervalTimer myTimer;```

Create an IntervalTimer object. You may create as many IntervalTimers as 
needed, but only a limited number (3) may be active simultaneously. 
Normally IntervalTimer objects should be created as global variables. 


```myTimer.begin(function, time, timebase);```

Begin calling the specified function. The interval is specified in 
microseconds or milliseconds based on the selected timebase: uSec for 
microseconds and hmSec for half-milliseconds. The time may be an 
unsigned integer, integer or long. The function returns true if 
successful. False is returned if all hardware resources are busy, used 
by other IntervalTimer objects.

Functions called by IntervalTimer shouldbe short, run as quickly as
possible, and should avoid calling other functions if possible.


myTimer.end();

Stop the timer function. The hardware resource becomes available for use 
by other IntervalTimer objects. 


```myTImer.interrupt_SIT(action);```

Enables or disables an active IntervalTimer's interrupts without 
deleting the object. 


3. Example Program 
------------------

The included demo program will create three Interval Timers (maximum 
allowed) to blink three LEDs at different intervals. The first timer 
will blink the onboard LED while 2 extra LEDs (and small current 
limiting resistors) must be added by the user on pins D3 and D4. A 
counter showing the number of Timer 1 blinks is output to the serial 
console. After 100 blinks, Timer 1 is shut down and will stop blinking 
(and the blink counter will no longer change). 


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

