// filename RGBLed.h
// project ProjectCar
// author Pavel

#ifndef RGBLED_H
#define RGBLED_H

#include "CommonTypes.h"

typedef enum rgb_color_e {
  ECOLOR_NONE = 0,
  ECOLOR_RED = 1,
  ECOLOR_GREEN = 2
//  ECOLOR_BLUE = 3
} rgb_color_t;

result_t rgbled_init();
result_t rgbled_setcolor(rgb_color_t _rgbbcolor);
result_t rgbled_setLed(bool _enable);

#endif // RGBLED_H
