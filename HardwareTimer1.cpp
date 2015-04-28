#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER1

#ifdef __CC3200R1M1RGC__

#include "wiring_private.h"
#include <driverlib/prcm.h>
#include <driverlib/timer.h>
#include "driverlib/utils.h"
#include "driverlib/Wdt.h"

#else
#include "Arduino.h"
#endif


byte _t1_csb;
long _t1_cyc;
int _t1_sca;

#ifdef __CC3200R1M1RGC__

void TimerHandler(void) {
    MAP_TimerIntClear(TIMERA0_BASE, TIMER_B);
    Timer1.isr();
}

#else
// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER1_OVF_vect) {
	Timer1.isr();
}
#endif

void _t1_init() {
#ifdef __CC3200R1M1RGC__
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA0);
    MAP_TimerConfigure(TIMERA0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    MAP_TimerIntRegister(TIMERA0_BASE, TIMER_B, TimerHandler);
    MAP_TimerIntEnable(TIMERA0_BASE, TIMER_TIMB_TIMEOUT);//3E8//TIMER_TIMB_TIMEOUT
#else
	TCCR1A = 0;                 // clear control register A
	TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer

#endif

}

long _t1_period(long us) {
#ifdef __CC3200R1M1RGC__
    if (us > 1000) {
        long cycles = us / 1000;
        if (cycles > 200) {
            return 0;
        }
        MAP_TimerPrescaleSet(TIMERA0_BASE, TIMER_B, 250);
        MAP_TimerLoadSet(TIMERA0_BASE, TIMER_B, 320 * cycles);

    }
    else if (us <= 1000) {

        long cycles = us * 10;
        MAP_TimerPrescaleSet(TIMERA0_BASE, TIMER_B, 7);//7
        MAP_TimerLoadSet(TIMERA0_BASE, TIMER_B, cycles);

    }

#else
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
#endif
}

void _t1_enable() {
#ifdef __CC3200R1M1RGC__
#else
		TIMSK1 = _BV(TOIE1);
#endif

}

void _t1_disable() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerDisable(TIMERA0_BASE, TIMER_B);
#else
		TIMSK1 &= ~_BV(TOIE1);
#endif

}

void _t1_start() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerEnable(TIMERA0_BASE, TIMER_B);
#else
		TCCR1B |= _t1_csb;
#endif


}

void _t1_stop() {
#ifdef __CC3200R1M1RGC__
#else
		TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
#endif
    //          // clears all clock selects bits
}

void _t1_restart() {
#ifdef __CC3200R1M1RGC__
#else
		TCNT1 = 0;
#endif

}

hwt_callbacks TIMER1_CALLBACKS = {_t1_init, _t1_period, _t1_enable, _t1_disable, _t1_start, _t1_stop, _t1_restart};

HardwareTimer Timer1(TIMER1_CALLBACKS, RESOLUTION_T16);

#endif
