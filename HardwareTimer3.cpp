#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER3

#include "Arduino.h"

byte _t3_csb;
long _t3_cyc;
int _t3_sca;

void _t3_init() {
	TCCR3A = 0;                 // clear control register A
	TCCR3B = _BV(WGM33);        // set mode as phase and frequency correct pwm, stop the timer
}

long _t3_period(long us) {
	long cycles = us * (SYSCLOCK / 2000000.0); // the counter runs backwards after TOP, interrupt is at BOTTOM so divide us by 2
	if (cycles < RESOLUTION_T16) {					// no prescale, full xtal
		_t3_sca = 1;
		_t3_csb = _BV(CS10);
	} else if ((cycles >>= 3) < RESOLUTION_T16) {	// prescale by /8
		_t3_sca = 8;
		_t3_csb = _BV(CS11);
	} else if ((cycles >>= 3) < RESOLUTION_T16) {	// prescale by /64
		_t3_sca = 64;
		_t3_csb = _BV(CS11) | _BV(CS10);
	} else if ((cycles >>= 2) < RESOLUTION_T16) {	// prescale by /256
		_t3_sca = 256;
		_t3_csb = _BV(CS12);
	} else if ((cycles >>= 2) < RESOLUTION_T16) {	// prescale by /1024
		_t3_sca = 1024;
		_t3_csb = _BV(CS12) | _BV(CS10);
	} else {
		_t3_sca = 1024;
		_t3_csb = _BV(CS12) | _BV(CS10);  // request was out of bounds, set as maximum
		cycles = RESOLUTION_T16 - 1;
	}

	ICR3 = _t3_cyc = cycles;
	return (cycles * _t3_sca) * (2000000.0 / SYSCLOCK);
}

void _t3_enable() {
	TIMSK3 = _BV(TOIE3);
}

void _t3_disable() {
	TIMSK3 &= ~_BV(TOIE3);
}

void _t3_start() {
	TCCR3B |= _t3_csb;
}

void _t3_stop() {
	TCCR3B &= ~(_BV(CS30) | _BV(CS31) | _BV(CS32));          // clears all clock selects bits
}

void _t3_restart() {
	TCNT3 = 0;
}

hwt_callbacks TIMER3_CALLBACKS = { _t3_init, _t3_period, _t3_enable, _t3_disable, _t3_start, _t3_stop, _t3_restart };

HardwareTimer Timer3(TIMER3_CALLBACKS, RESOLUTION_T16);

// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER3_OVF_vect) {
	Timer3.isr();
}

#endif
