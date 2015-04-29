/*
 * HardwareTimer.h
 *
 *  Created on: 2015年1月24日
 *      Author: taoyuan
 */
#ifndef HARDWARETIMER_H_
#define HARDWARETIMER_H_

#define __CC3200R1M1RGC__

#ifdef ARDUINO

#include <Arduino.h>

#endif

#ifdef __CC3200R1M1RGC__

#else
#include <avr/io.h>
#include <avr/interrupt.h>

#endif


#define RESOLUTION_T8     256
#define RESOLUTION_T16    65536

#ifdef F_CPU
#define SYSCLOCK F_CPU     // main Arduino clock
#else
#define SYSCLOCK 16000000  // main Arduino clock
#endif

#define MS_TO_TICKS(ms)         ((SYSCLOCK/1000) * (ms))
#define US_TO_TICKS(us)         ((SYSCLOCK/1000000UL) * (us))

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
    HardwareTimer(hwt_callbacks &callbacks) : _callbacks(callbacks), _isr(0) {
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
    ISRCallback _isr;
};


#if defined(TCCR1A) || defined(TIMERA1A)
extern HardwareTimer Timer1;
#define HAVE_HWTIMER1
#endif

#if defined(TCCR2A) || defined(TIMERA2A)
extern HardwareTimer Timer2;
#define HAVE_HWTIMER2
#endif

#if defined(TCCR3A) || defined(TIMERA3A)
extern HardwareTimer Timer3;
#define HAVE_HWTIMER3
#endif

#endif /* HARDWARETIMER_H_ */
