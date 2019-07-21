// filename CarTimer.h
// project ProjectCar
// author Pavel

#ifndef CARTIMER_H
#define CARTIMER_H

#include "CommonTypes.h"

#define CAR_TIMER_TIME_SLICE_IN_10MS 5
#define CAR_TIMER_MS_IN_A_SECOND 1000

#define CAR_TIMER_COMPARE_VALUE 249UL

typedef void (*pTimer_callback_fxn_t)(result_t const _status);

result_t cartimer_init(pTimer_callback_fxn_t _cb);
// this function is used only for checking the alarm if set
result_t cartimer_idle();

result_t cartimer_setTimerToSchedule();

// gets the ticks 0 ... 249 and the compare reset count 0 ... 4294967295UL
// each reset is done once in 2 milliseconds so more than 8589934 seconds can be identified
result_t cartimer_getTickCounter(unsigned int * const ticks, unsigned long * const _p2msCycles);

// returns the amount of the milliseconds since the boot time
result_t cartimer_getMillisecondsSinceStart(unsigned long * const _pMilliseconds);

// provides callback triger for period no bigger than 10 seconds = 10 000 000 microseconds
// after execution the callback is automatically set to NULL - so it cannot be executed more than once
result_t cartimer_setAlarmAfterMicroSeconds(unsigned long const _microSeconds, pTimer_callback_fxn_t _cb);


#endif // CARTIMER_H
