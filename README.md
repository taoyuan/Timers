# Timers
> A hardware timer library for Arduino.

### Basic Usage
The timer is configured to repetitively measure a period of time, in microseconds. At the end of each period, an interrupt function can be run. 

### Configuration

`TimerX.initialize(microseconds)`

Begin using the timer. This function must be called first. "microseconds" is the period of time the timer takes.

`TimerX.setPeriod(microseconds)`

Set a new period after the library is already initialized.

### Run Control

`TimerX.start()`

Start the timer, beginning a new period.

`TimerX.stop()`

Stop the timer.

`TimerX.restart();`

Restart the timer, from the beginning of a new period.

`TimerX.resume()`

Resume running a stopped timer. A new period is not begun.

### Interrupt Function

`TimerX.attachInterrupt(function)`

Run a function each time the timer period finishes. The function is run as an interrupt, so special care is needed to share any variables beteen the interrupt function and your main program.

`TimerX.detachInterrupt();`

Disable the interrupt, so the function no longer runs.

### Example 

```cpp
#include <Timers.h>

HardwareTimer &Timer = Timer1; // Could be Timer1, Timer2 or Timer3

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

const int led = RGB_BUILTIN_R;  // the pin with a LED

void setup(void) {
	pinMode(led, OUTPUT);
	Timer.init(150000);		// Timer2 only support below 16.384 ms interval
	Timer.attachInterrupt(blinkLED); // blinkLED to run every 0.15 seconds
	Serial.begin(115200);
}

// The interrupt will blink the LED, and keep track of how many times it has blinked.
int ledState = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables

void blinkLED(void) {
	if (ledState == LOW) {
		ledState = HIGH;
		blinkCount = blinkCount + 1;  // increase when LED turns on
	} else {
		ledState = LOW;
	}
	digitalWrite(led, ledState);
}

// The main program will print the blink count to the Arduino Serial Monitor
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

	Serial.print("blinkCount = ");
	Serial.println(blinkCopy);
	delay(1000);
}
```

### References
[TimerOne & TimerThree Libraries](https://www.pjrc.com/teensy/td_libs_TimerOne.html)