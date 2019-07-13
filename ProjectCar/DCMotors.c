// filename DCMotors.c
// project ProjectCar
// author Pavel

#include "DCMotors.h"

static unsigned int currentSpeed = 0U;
static bool directionForward = true;

result_t dcmotors_init()
{
  return EOK;
}

result_t dcmotors_setLeftDCMotorSpeed(unsigned int _speed)
{  
  currentSpeed = _speed;

  return EOK;  
}

result_t dcmotors_setRightDCMotorSpeed(unsigned int _speed)
{
  currentSpeed = _speed;
  
  return EOK;  
}

result_t dcmotors_setDirectionForward(bool _direction)
{
  directionForward = _direction;
  
  return EOK;  
}
