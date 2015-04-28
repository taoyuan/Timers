#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER3

#include "Arduino.h"

byte _t3_csb;
long _t3_cyc;
int _t3_sca;
#ifdef __CC3200R1M1RGC__

void TimerHandler2(void) {
    MAP_TimerIntClear(TIMERA2_BASE, TIMER_B);
    Timer3.isr();
}

#else
// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER3_OVF_vect) {
	Timer3.isr();
}
#endif

void _t3_init() {
#ifdef __CC3200R1M1RGC__
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA2);
    MAP_TimerConfigure(TIMERA2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    MAP_TimerIntRegister(TIMERA2_BASE, TIMER_B, TimerHandler2);
    MAP_TimerIntEnable(TIMERA2_BASE, TIMER_TIMB_TIMEOUT);//3E8//TIMER_TIMB_TIMEOUT
#else
			TCCR3A = 0;                 // clear control register A
			TCCR3B = _BV(WGM33);        // set mode as phase and frequency correct pwm, stop the timer
	
#endif


}

long _t3_period(long us) {
#ifdef __CC3200R1M1RGC__
    if (us > 1000) {
        long cycles = us / 1000;
        if (cycles > 200) {
            return 0;
        }
        MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_B, 250);
        MAP_TimerLoadSet(TIMERA2_BASE, TIMER_B, 320 * cycles);

    }
    else if (us <= 1000) {

        long cycles = us * 10;
        MAP_TimerPrescaleSet(TIMERA2_BASE, TIMER_B, 7);//7
        MAP_TimerLoadSet(TIMERA2_BASE, TIMER_B, cycles);

    }

#else
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
#endif
}

void _t3_enable() {

#ifdef __CC3200R1M1RGC__
#else
			TIMSK3 = _BV(TOIE3);
#endif

}

void _t3_disable() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerDisable(TIMERA2_BASE, TIMER_B);

#else
		TIMSK3 &= ~_BV(TOIE3);
#endif

}

void _t3_start() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerEnable(TIMERA2_BASE, TIMER_B);
#else
		TCCR3B |= _t3_csb;
#endif


}

void _t3_stop() {
#ifdef __CC3200R1M1RGC__
#else
		TCCR3B &= ~(_BV(CS30) | _BV(CS31) | _BV(CS32));          // clears all clock selects bits
#endif

}

void _t3_restart() {
#ifdef __CC3200R1M1RGC__
#else
		TCNT3 = 0;
#endif

}

hwt_callbacks TIMER3_CALLBACKS = {_t3_init, _t3_period, _t3_enable, _t3_disable, _t3_start, _t3_stop, _t3_restart};

HardwareTimer Timer3(TIMER3_CALLBACKS, RESOLUTION_T16);


#endif
