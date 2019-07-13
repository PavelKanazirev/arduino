// filename CarTimer.h
// project ProjectCar
// author Pavel

#ifndef CARTIMER_H
#define CARTIMER_H

#include "CommonTypes.h"

typedef void (*pTimer_callback_fxn_t)(result_t const _status);

result_t cartimer_init(pTimer_callback_fxn_t _cb);
result_t cartimer_waitIdle();

#endif // CARTIMER_H
