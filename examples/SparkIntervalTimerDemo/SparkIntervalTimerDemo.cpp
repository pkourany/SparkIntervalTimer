// Spark Interval Timer demo
//
// Please refer to the github README file for more details:
// https://github.com/pkourany/SparkIntervalTimer/blob/master/README.md
//
// This demo will create several Interval Timers (3 on Core, 5 on Photon) to blink 
// LEDs at different intervals.  The first timer will blink the onboard LED
// while extra LEDs (and small current limiting resistors) must be added
// by the user on pins D3 and D4 (Core) and additionally on D5 and D6 for Photon.
// After 100 blinks, Timer1 will reset to 1/4 of its interval and after 200 more
// blinks, Timer1 is shut down and will stop blinking.


#include "SparkIntervalTimer.h"

SYSTEM_MODE(MANUAL);		//For this demo, WiFi and Cloud connections are disabled

// Create IntervalTimer objects
IntervalTimer myTimer;		// 3 for the Core
IntervalTimer myTimer2;
IntervalTimer myTimer3;
#if (PLATFORM_ID == 6)		// 2 more for the Photon
IntervalTimer myTimer4;
IntervalTimer myTimer5;
#endif

// Pre-declare ISR callback functions
void blinkLED(void);
void blinkLED2(void);
void blinkLED3(void);
#if (PLATFORM_ID == 6)		// Extras for Photon
void blinkLED4(void);
void blinkLED5(void);
#endif

const uint8_t ledPin = D7;		// LED for first Interval Timer
const uint8_t ledPin2 = D3;		// LED for second Interval Timer
const uint8_t ledPin3 = D4;		// LED for third Interval Timer
#if (PLATFORM_ID == 6)			// Extras for Photon
const uint8_t ledPin4 = D5;
const uint8_t ledPin5 = D6;
#endif

void setup(void) {
  pinMode(ledPin, OUTPUT);		// Configure LED pins as OUTPUTs
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
 #if (PLATFORM_ID == 6)			//Extras for Photon
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
#endif
  
  // AUTO allocate blinkLED to run every 500ms using hmSec timescale (1000 * 0.5ms period)
  // On Core the first allocated timer is TIMER2.  On Photon, it is TIMER3.
  myTimer.begin(blinkLED, 1000, hmSec);

  // Manually allocate blinkLED2 to hardware timer TIMER4 to run every 250ms using hmSec timescale (500 * 0.5ms period)
  // TIMER4 is common to both Core and Photon.
  myTimer2.begin(blinkLED2, 500, hmSec, TIMER4);
  
  // Auto allocate blinkLED3 to run every 1000ms using hmSec timescale (2000 * 0.5ms period)
  // On Core allocated timer will be first free = TIMER3.  On Photon, it will be TIMER5.
  myTimer3.begin(blinkLED3, 2000, hmSec);
  
#if (PLATFORM_ID == 6)
  // Auto allocate blinkLED4 to run every 65ms using uSec timescale (65000 * 1us period)
  // On Photon (only) allocated timer will be next free = TIMER6.
  myTimer4.begin(blinkLED4, 65000, uSec);
  
  // Manually allocate blinkLED5 to hardware timer TIMER7 blinkLED to run every 5000ms (10000 * .5ms period)
  myTimer5.begin(blinkLED5, 10000, hmSec, TIMER7);
#endif
}

// The first TIMER (myTimer) interrupt will blink the LED, and keep
// track of how many times it has blinked.  After 200 on/off cycles
// the timer interval will be changed to 125ms (250 * 0.5ms).
// After a further 200 pulses, the timer will be shut down.
// All other timers only blink their associated LEDs

volatile unsigned long blinkCount = 0; // use volatile for variables shared with ISR

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.

// Callback for Timer 1
void blinkLED(void) {
	digitalWrite(ledPin,!digitalRead(ledPin));
    blinkCount++;		// increase when LED turns changes
}

// Callback for Timer 2
void blinkLED2(void) {
	digitalWrite(ledPin2,!digitalRead(ledPin2));
}

// Callback for Timer 3
void blinkLED3(void) {
	digitalWrite(ledPin3,!digitalRead(ledPin3));
}

 #if (PLATFORM_ID == 6)		// Extras for the Photon
// Callback for Timer 4
void blinkLED4(void) {
	digitalWrite(ledPin4,!digitalRead(ledPin4));
}

// Callback for Timer 5
void blinkLED5(void) {
	digitalWrite(ledPin5,!digitalRead(ledPin5));
}
#endif


void loop(void) {

  if (blinkCount == 200)	{			// After 200 on/off cycles, change first timer interval
  							// to 125ms (250 * 0.5ms) for next 200 cycles.
    blinkCount++;					// increment count so IF does not trigger again
	myTimer.resetPeriod_SIT(250, hmSec);
	}
  else if (blinkCount >= 400) {				// After 200 more on/off cycles, shut down first timer
  	blinkCount = 0;					// reset count so IF does not keep passing
	myTimer.end();					// Disable the first timer
	}
}
