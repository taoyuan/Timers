#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER1

#include "Arduino.h"

byte _t1_csb;
long _t1_cyc;
int _t1_sca;

void _t1_init() {
	TCCR1A = 0;                 // clear control register A
	TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer
}

long _t1_period(long us) {
	long cycles = us * (SYSCLOCK / 2000000.0); // the counter runs backwards after TOP, interrupt is at BOTTOM so divide us by 2
	if (cycles < RESOLUTION_T16) {					// no prescale, full xtal
		_t1_sca = 1;
		_t1_csb = _BV(CS10);
	} else if ((cycles >>= 3) < RESOLUTION_T16) {	// prescale by /8
		_t1_sca = 8;
		_t1_csb = _BV(CS11);
	} else if ((cycles >>= 3) < RESOLUTION_T16) {	// prescale by /64
		_t1_sca = 64;
		_t1_csb = _BV(CS11) | _BV(CS10);
	} else if ((cycles >>= 2) < RESOLUTION_T16) {	// prescale by /256
		_t1_sca = 256;
		_t1_csb = _BV(CS12);
	} else if ((cycles >>= 2) < RESOLUTION_T16) {	// prescale by /1024
		_t1_sca = 1024;
		_t1_csb = _BV(CS12) | _BV(CS10);
	} else {
		_t1_sca = 1024;
		_t1_csb = _BV(CS12) | _BV(CS10);  // request was out of bounds, set as maximum
		cycles = RESOLUTION_T16 - 1;
	}

	ICR1 = _t1_cyc = cycles;
	return (cycles * _t1_sca) * (2000000.0 / SYSCLOCK);
}

void _t1_enable() {
	TIMSK1 = _BV(TOIE1);
}

void _t1_disable() {
	TIMSK1 &= ~_BV(TOIE1);
}

void _t1_start() {
	TCCR1B |= _t1_csb;
}

void _t1_stop() {
	TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));          // clears all clock selects bits
}

void _t1_restart() {
	TCNT1 = 0;
}

hwt_callbacks TIMER1_CALLBACKS = { _t1_init, _t1_period, _t1_enable, _t1_disable, _t1_start, _t1_stop, _t1_restart };

HardwareTimer Timer1(TIMER1_CALLBACKS, RESOLUTION_T16);

// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER1_OVF_vect) {
	Timer1.isr();
}

#endif
