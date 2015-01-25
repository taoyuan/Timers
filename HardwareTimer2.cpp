#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER2

#include "Arduino.h"

byte _t2_csb;
long _t2_cyc;
int _t2_sca;

void _t2_init() {
	TCCR2A = _BV(WGM21);
//	TCCR2B = 0;
}

long _t2_period(long us) {
	long cycles = us * (SYSCLOCK / 1000000.0);
	if (cycles < RESOLUTION_T8) {
		_t2_csb = _BV(CS20);              				// no prescale, full xtal
		_t2_sca = 1;
	} else if ((cycles >>= 3) < RESOLUTION_T8) {
		_t2_csb = _BV(CS21);              				// prescale by /8
		_t2_sca = 8;
	} else if ((cycles >>= 2) < RESOLUTION_T8) {
		_t2_csb = _BV(CS21) | _BV(CS20);  				// prescale by /32
		_t2_sca = 32;
	} else if ((cycles >>= 1) < RESOLUTION_T8) {
		_t2_csb = _BV(CS22);              				// prescale by /64
		_t2_sca = 64;
	} else if ((cycles >>= 1) < RESOLUTION_T8) {
		_t2_csb = _BV(CS22) | _BV(CS20);  				// prescale by /128
		_t2_sca = 128;
	} else if ((cycles >>= 1) < RESOLUTION_T8) {
		_t2_csb = _BV(CS22) | _BV(CS21);  				// prescale by /256
		_t2_sca = 256;
	} else if ((cycles >>= 2) < RESOLUTION_T8) {
		_t2_csb = _BV(CS22) | _BV(CS21) | _BV(CS20);  	// prescale by /1024
		_t2_sca = 1024;
	} else {
		_t2_csb = _BV(CS22) | _BV(CS21) | _BV(CS20);  // request was out of bounds, set as maximum
		_t2_sca = 1024;
		cycles = RESOLUTION_T8 - 1;
	}
	OCR2A = _t2_cyc = cycles;
	return (cycles * _t2_sca) * (1000000.0 / SYSCLOCK);
}

void _t2_enable() {
	TIMSK2 = _BV(OCIE2A);
}

void _t2_disable() {
	TIMSK2 &= ~_BV(OCIE2A);
}

void _t2_start() {
	TCCR2B |= _t2_csb;
}

void _t2_stop() {
	TCCR2B &= ~(_BV(CS20) | _BV(CS21) | _BV(CS22));          // clears all clock selects bits
}

void _t2_restart() {
	TCNT2 = 0;
}

hwt_callbacks TIMER2_CALLBACKS = { _t2_init, _t2_period, _t2_enable, _t2_disable, _t2_start, _t2_stop, _t2_restart };

HardwareTimer Timer2(RESOLUTION_T8, TIMER2_CALLBACKS);

// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER2_COMPA_vect) {
	Timer2.isrCallback();
}

#endif
