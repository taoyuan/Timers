/*
 * HardwareTimer.cpp
 *
 *  Created on: 2015年1月24日
 *      Author: taoyuan
 */

#include <HardwareTimer.h>

void HardwareTimer::init(long us) {
    _callbacks.init();
    setPeriod(us);
}


long HardwareTimer::setPeriod(long us) {
    long ret = _callbacks.period(us);
    stop();
    start();
    return ret;
}

void HardwareTimer::attachInterrupt(void (*isr)(), long us) {
    if (us > 0)
        setPeriod(us);
    _isr = isr;        // register the user's callback with the real ISR
    _callbacks.enable();    // sets the timer overflow interrupt enable bit
#ifdef __CC3200R1M1RGC__
#else
	sei();	
#endif				// ensures that interrupts are globally enabled
    start();
}

void HardwareTimer::detachInterrupt() {
    _callbacks.disable();    // clears the timer overflow interrupt enable bit
}

void HardwareTimer::start() {
    _callbacks.start();
}

void HardwareTimer::stop() {
    _callbacks.stop();          // clears all clock selects bits
}

void HardwareTimer::restart() {
    _callbacks.restart();
}


void HardwareTimer::isr() {
    if (_isr) {
        _isr();
    }
}
