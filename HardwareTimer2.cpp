#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER2

#include "Arduino.h"

byte _t2_csb;
long _t2_cyc;
int _t2_sca;
#ifdef __CC3200R1M1RGC__

void TimerHandler1(void) {
    MAP_TimerIntClear(TIMERA1_BASE, TIMER_B);
    Timer2.isr();
}

#else
// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER2_OVF_vect) {
	Timer2.isr();
}
#endif

void _t2_init() {
#ifdef __CC3200R1M1RGC__
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA1);
    MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    MAP_TimerIntRegister(TIMERA1_BASE, TIMER_B, TimerHandler1);
    MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMB_TIMEOUT);//3E8//TIMER_TIMB_TIMEOUT
#else
		TCCR2A = _BV(WGM21);

#endif

//	TCCR2B = 0;
}

long _t2_period(long us) {
#ifdef __CC3200R1M1RGC__
    if (us > 1000) {
        long cycles = us / 1000;
        if (cycles > 200) {
            return 0;
        }
        MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_B, 250);
        MAP_TimerLoadSet(TIMERA1_BASE, TIMER_B, 320 * cycles);

    }
    else if (us <= 1000) {

        long cycles = us * 10;
        MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_B, 7);//7
        MAP_TimerLoadSet(TIMERA1_BASE, TIMER_B, cycles);

    }

#else
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
#endif
}

void _t2_enable() {
#ifdef __CC3200R1M1RGC__
#else
			TIMSK2 = _BV(OCIE2A);
#endif

}

void _t2_disable() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerDisable(TIMERA1_BASE, TIMER_B);

#else
		TIMSK2 &= ~_BV(OCIE2A);
#endif


}

void _t2_start() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerEnable(TIMERA1_BASE, TIMER_B);
#else
		TCCR2B |= _t2_csb;
#endif

}

void _t2_stop() {
#ifdef __CC3200R1M1RGC__
#else
		TCCR2B &= ~(_BV(CS20) | _BV(CS21) | _BV(CS22));          // clears all clock selects bits
#endif

}

void _t2_restart() {
#ifdef __CC3200R1M1RGC__
#else
		TCNT2 = 0;
#endif

}

hwt_callbacks TIMER2_CALLBACKS = {_t2_init, _t2_period, _t2_enable, _t2_disable, _t2_start, _t2_stop, _t2_restart};

HardwareTimer Timer2(TIMER2_CALLBACKS, RESOLUTION_T8);

// interrupt service routine that wraps a user defined function supplied by attachInterrupt


#endif
