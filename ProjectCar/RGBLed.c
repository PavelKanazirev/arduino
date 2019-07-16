// filename RGBLed.c
// project ProjectCar
// author Pavel

#include "RGBLed.h"

static rgb_color_t rgbColor = ECOLOR_NONE;
static bool ledEnabled = false;

result_t rgbled_init()
{
  //  red // PD5
  BIT_SET(DDRD,DDD5); // set the PD5 port direction to output
  BIT_CLEAR(PORTD,PD5); // set the default value of the PD5 output to 0
  //  green // PD4 // currently not used - in the end product can be redirected to pins 0 and 1 when Serial is not used
//  BIT_SET(DDRD,DDD4); // set the PD4 port direction to output
//  BIT_CLEAR(PORTD,PD4); // set the default value of the PD4 output to 0
  
  return EOK;
}

result_t rgbled_setcolor(rgb_color_t _rgbbcolor)
{
  rgbColor = _rgbbcolor;
  
  return EOK;
}

result_t rgbled_setLed(bool _enable)
{
  ledEnabled = _enable;

  if (_enable)
  {
    if (ECOLOR_NONE == rgbColor)
    {
      BIT_CLEAR(PORTD,PD5); // set pin to 0
      // currently not used - in the end product can be redirected to pins 0 and 1 when Serial is not used
//      BIT_CLEAR(PORTD,PD4); // set pin to 0
    }
    else if (ECOLOR_RED == rgbColor)
    {
      BIT_SET(PORTD,PD5); // set pin to 1 - led color to Red
    }
    else if (ECOLOR_GREEN == rgbColor)
    {
      // currently not used - in the end product can be redirected to pins 0 and 1 when Serial is not used
//      BIT_SET(PORTD,PD4); // set pin to 1- led color to Green
    }
  }
  else
  {
    BIT_CLEAR(PORTD,PD5); // set pin to 0
    // currently not used - in the end product can be redirected to pins 0 and 1 when Serial is not used
//    BIT_CLEAR(PORTD,PD4); // set pin to 0    
  }
  
  return EOK;
}
