// filename CarTimer.h
// project ProjectCar
// author Pavel

#ifndef CARTIMER_H
#define CARTIMER_H

#include "CommonTypes.h"

#define CAR_TIMER_TIME_SLICE_IN_10MS 5
#define CAR_TIMER_MS_IN_A_SECOND 1000
#define CAR_TIMER_MAX_MICROSECONDS_DELAY_VALUE 50000
#define CAR_TIMER_MIN_MICROSECONDS_DELAY_VALUE 20

// approximate 4 microseconds for one tick ( 1 millisecond / 250 ticks )
#define CAR_TIMER_COMPARE_VALUE 249UL

typedef void (*pTimer_callback_fxn_t)(result_t const _status);

result_t cartimer_init(pTimer_callback_fxn_t _cb);

result_t cartimer_delayMicroseconds(unsigned int const micros);

#endif // CARTIMER_H
