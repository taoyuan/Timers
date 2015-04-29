#include "HardwareTimer.h"

#ifdef HAVE_HWTIMER3

#ifdef __CC3200R1M1RGC__

#include <driverlib/timer.h>
#include <inc/hw_ints.h>
#include <driverlib/prcm.h>

#endif

byte _t3_csb;
long _t3_cyc;
int _t3_sca;

#ifdef __CC3200R1M1RGC__

void __t3_timer_handler(void) {
    unsigned long ulInts;

    ulInts = MAP_TimerIntStatus(TIMERA3_BASE, 1);
    //
    // Clear the timer interrupt.
    //
    MAP_TimerIntClear(TIMERA3_BASE, ulInts);
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
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);
    PRCMCC3200MCUInit();
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralReset(PRCM_TIMERA3);
    MAP_TimerConfigure(TIMERA3_BASE, TIMER_CFG_PERIODIC);
    MAP_TimerPrescaleSet(TIMERA3_BASE, TIMER_BOTH, 0);

#else
			TCCR3A = 0;                 // clear control register A
			TCCR3B = _BV(WGM33);        // set mode as phase and frequency correct pwm, stop the timer
	
#endif


}

long _t3_period(long us) {
#ifdef __CC3200R1M1RGC__
    MAP_TimerLoadSet(TIMERA3_BASE, TIMER_BOTH, US_TO_TICKS(us));

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
	MAP_TimerIntRegister(TIMERA3_BASE, TIMER_BOTH, __t3_timer_handler);
	MAP_IntPrioritySet(INT_TIMERA0A, INT_PRIORITY_LVL_1);
	MAP_TimerIntEnable(TIMERA3_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);
#else
			TIMSK3 = _BV(TOIE3);
#endif

}

void _t3_disable() {
#ifdef __CC3200R1M1RGC__
	MAP_TimerIntDisable(TIMERA3_BASE, TIMER_BOTH);

#else
		TIMSK3 &= ~_BV(TOIE3);
#endif

}

void _t3_start() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerEnable(TIMERA3_BASE, TIMER_BOTH);
#else
		TCCR3B |= _t3_csb;
#endif


}

void _t3_stop() {
#ifdef __CC3200R1M1RGC__
    MAP_TimerDisable(TIMERA3_BASE, TIMER_BOTH);
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

HardwareTimer Timer3(TIMER3_CALLBACKS);


#endif
