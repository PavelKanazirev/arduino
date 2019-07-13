// filename DCMotors.h
// project ProjectCar
// author Pavel

#ifndef DCMOTORS_H
#define DCMOTORS_H

#include "CommonTypes.h"

result_t dcmotors_init();
result_t dcmotors_setLeftDCMotorSpeed(unsigned int _speed);
result_t dcmotors_setRightDCMotorSpeed(unsigned int _speed);

result_t dcmotors_setDirectionForward(bool _direction);

#endif // DCMOTORS_H
