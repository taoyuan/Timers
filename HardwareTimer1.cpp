#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER1

#ifdef __CC3200R1M1RGC__

#include <driverlib/timer.h>
#include <inc/hw_ints.h>
#include <driverlib/prcm.h>

#endif

byte _t1_csb;
long _t1_cyc;
int _t1_sca;

#ifdef __CC3200R1M1RGC__

void __t1_timer_handler(void) {
    unsigned long ulInts;
    ulInts = MAP_TimerIntStatus(TIMERA1_BASE, 1);
    //
    // Clear the timer interrupt.
    //
    MAP_TimerIntClear(TIMERA1_BASE, ulInts);
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
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);
    PRCMCC3200MCUInit();
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA1, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA1);
    MAP_TimerConfigure(TIMERA1_BASE, TIMER_CFG_PERIODIC);
    MAP_TimerPrescaleSet(TIMERA1_BASE, TIMER_BOTH, 0);
#else
	TCCR1A = 0;                 // clear control register A
	TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer

#endif

}

long _t1_period(long us) {
#ifdef __CC3200R1M1RGC__
    MAP_TimerLoadSet(TIMERA1_BASE, TIMER_BOTH, US_TO_TICKS(us));
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
	MAP_TimerIntRegister(TIMERA1_BASE, TIMER_BOTH, __t1_timer_handler);
	MAP_IntPrioritySet(INT_TIMERA1A, INT_PRIORITY_LVL_1);
	MAP_TimerIntEnable(TIMERA1_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);

#else
		TIMSK1 = _BV(TOIE1);
#endif

}

void _t1_disable() {
#ifdef __CC3200R1M1RGC__
	MAP_TimerIntDisable(TIMERA1_BASE, TIMER_BOTH);
#else
		TIMSK1 &= ~_BV(TOIE1);
#endif

}

void _t1_start() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerEnable(TIMERA1_BASE, TIMER_BOTH);
#else
		TCCR1B |= _t1_csb;
#endif


}

void _t1_stop() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerDisable(TIMERA1_BASE, TIMER_BOTH);

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

HardwareTimer Timer1(TIMER1_CALLBACKS);

#endif
