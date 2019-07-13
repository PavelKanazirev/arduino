// filename Joystick.h
// project ProjectCar
// author Pavel

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "CommonTypes.h"

#define JOYSTICK_MAX 255
#define JOYSTICK_MIN 0
#define JOYSTICK_THRESHOLD 2

result_t joystick_init();
result_t joystick_getXAxis(unsigned int * const _pX_value, unsigned int * const _low, unsigned int * const _high);
result_t joystick_getYAxis(unsigned int * const _pX_value, unsigned int * const _low, unsigned int * const _high);
result_t joystick_getHorn(bool * const _pButton_down);

#endif // JOYSTICK_H
