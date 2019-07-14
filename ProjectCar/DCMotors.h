// filename DCMotors.h
// project ProjectCar
// author Pavel

#ifndef DCMOTORS_H
#define DCMOTORS_H

#include "CommonTypes.h"

#define DC_MOTORS_STOP 0U
#define DC_MOTORS_SPEED_THRESHOLD 2U
#define DC_MOTORS_MIN_SPEED 127U
#define DC_MOTORS_MAX_SPEED 255U
#define DC_MOTORS_GEAR1_SPEED 160U
#define DC_MOTORS_GEAR2_SPEED 192U
#define DC_MOTORS_GEAR3_SPEED 224U

result_t dcmotors_init();
result_t dcmotors_setLeftDCMotorSpeed(unsigned int _speed);
result_t dcmotors_setRightDCMotorSpeed(unsigned int _speed);

result_t dcmotors_setDirectionForward(bool const _direction);

result_t dcmotors_Task1ms();

#endif // DCMOTORS_H
