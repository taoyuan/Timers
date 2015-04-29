#include <Timers.h>

HardwareTimer &Timer = Timer2; // Could be Timer1, Timer2 or Timer3

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

#if defined(RGB_BUILTIN_R)
const int led = RGB_BUILTIN_R;  // the pin with a LED
#elif defined(__CC3200R1M1RGC__)
const int led = RED_LED;
#else
const int led = LED_BUILTIN;
#endif

void setup(void) {
	pinMode(led, OUTPUT);
	Timer.init(100000);		// Timer2 only support below 16.384 ms interval
	Timer.attachInterrupt(blinkLED); // blinkLED to run every 0.15 seconds
	Serial.begin(115200);
}

// The interrupt will blink the LED, and keep track of how many times it has blinked.
int ledState = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables

void blinkLED(void) {
	ledState = !ledState;
	if (ledState) blinkCount = blinkCount + 1;  // increase when LED turns on
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
