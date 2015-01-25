/*
 * HardwareTimer.h
 *
 *  Created on: 2015年1月24日
 *      Author: taoyuan
 */

#ifndef HARDWARETIMER_H_
#define HARDWARETIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define RESOLUTION_T8	256
#define RESOLUTION_T16	65536

#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif

struct hwt_callbacks {
	void (*init)();
	long (*period)(long us);
	void (*enable)();
	void (*disable)();
	void (*start)();
	void (*stop)();
	void (*restart)();
};

typedef void (*ISRCallback)();

class HardwareTimer {
public:
	HardwareTimer(hwt_callbacks &callbacks, long resolution) :
		_callbacks(callbacks), _resolution(resolution), _isr(0) {
	}
	void init(long us = 1000000);
	void start();
	void stop();
	void restart();
	void attachInterrupt(void (*isr)(), long us = -1);
	void detachInterrupt();
	long setPeriod(long us);
	void isr();
protected:
	hwt_callbacks &_callbacks;
	long _resolution;
	ISRCallback _isr;
};

#if defined(TCCR2A)
extern HardwareTimer Timer2;
#define HAVE_HWTIMER2
#endif

#if defined(TCCR1A)
extern HardwareTimer Timer1;
#define HAVE_HWTIMER1
#endif

#if defined(TCCR3A)
extern HardwareTimer Timer3;
#define HAVE_HWTIMER3
#endif

#endif /* HARDWARETIMER_H_ */
