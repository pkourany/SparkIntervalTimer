// Spark Interval Timer demo
//
// This demo will create three Interval Timers (maximum allowed) to blink three
// LEDs at different intervals.  The first timer will blink the onboard LED
// while 2 extra LEDs (and small current limiting resistors) must be added
// by the user on pins D3 and D4.  A counter showing the number of Timer 1 blinks
// is output to the serial console.  After 100 blinks, Timer 1 is shut down and
// will stop blinking (and the blink counter will no longer change).


#include "SparkIntervalTimer.h"

// Create 3 IntervalTimer objects
IntervalTimer myTimer;
IntervalTimer myTimer2;
IntervalTimer myTimer3;

// Pre-define ISR callback functions
void blinkLED(void);
void blinkLED2(void);
void blinkLED3(void);


const uint8_t ledPin = D7;		// LED for first Interval Timer
const uint8_t ledPin2 = D3;		// LED for second Interval Timer
const uint8_t ledPin3 = D4;		// LED for third Interval Timer


void setup(void) {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  
  myTimer.begin(blinkLED, 1000, hmSec);  // blinkLED to run every 500ms (1000 * .5ms period)
  myTimer2.begin(blinkLED2, 500, hmSec);  // blinkLED to run every 250ms (500 * .5ms period)
  myTimer3.begin(blinkLED3, 2000, hmSec);  // blinkLED to run every 1000ms (2000 * .5ms period)

}

// The first TIMER interrupt will blink the LED, and keep
// track of how many times it has blinked.  The other 
// two timers only blink their LEDs
int ledState = LOW;
int ledState2 = LOW;
int ledState3 = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.

// Callback for Timer 1
void blinkLED(void) {
  if (ledState == LOW) {
    ledState = HIGH;
    blinkCount = blinkCount + 1;  // increase when LED turns on
	PIN_MAP[ledPin].gpio_peripheral->BSRR = PIN_MAP[ledPin].gpio_pin; // LED High
  }
  else {
    ledState = LOW;
    PIN_MAP[ledPin].gpio_peripheral->BRR = PIN_MAP[ledPin].gpio_pin; // LED low
  }
}

// Callback for Timer 2
void blinkLED2(void) {
  if (ledState2 == LOW) {
    ledState2 = HIGH;
	PIN_MAP[ledPin2].gpio_peripheral->BSRR = PIN_MAP[ledPin2].gpio_pin; // LED High
  }
  else {
    ledState2 = LOW;
    PIN_MAP[ledPin2].gpio_peripheral->BRR = PIN_MAP[ledPin2].gpio_pin; // LED low
  }
}

// Callback for Timer 3
void blinkLED3(void) {
  if (ledState3 == LOW) {
    ledState3 = HIGH;
	PIN_MAP[ledPin3].gpio_peripheral->BSRR = PIN_MAP[ledPin3].gpio_pin; // LED High
  }
  else {
    ledState3 = LOW;
    PIN_MAP[ledPin3].gpio_peripheral->BRR = PIN_MAP[ledPin3].gpio_pin; // LED low
  }
}

// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void) {
  unsigned long blinkCopy;  // holds a copy of the blinkCount

  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.
  noInterrupts();
  blinkCopy = blinkCount;
  interrupts();

//  if (blinkCopy >= 100)				// After 100 blinks, shut down timer 1
//	myTimer.end();

/*
  Serial.print("LED=");
  Serial.print(ledState);
  Serial.print(" LED2=");
  Serial.print(ledState2);
  Serial.print(" LED3=");
  Serial.print(ledState3);

  Serial.print("blinkCount = ");
  Serial.println(blinkCopy);
*/
  delay(200);
}
