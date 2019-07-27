// filename DCMotors.h
// project ProjectCar
// author Pavel

#ifndef DCMOTORS_H
#define DCMOTORS_H

#include "CommonTypes.h"

#define DC_MOTORS_STOP 0U
#define DC_MOTORS_SPEED_THRESHOLD 2U
#define DC_MOTORS_MIN_SPEED 64
#define DC_MOTORS_MAX_SPEED 255
#define DC_MOTORS_GEAR1_SPEED 92
#define DC_MOTORS_GEAR2_SPEED 144
#define DC_MOTORS_GEAR3_SPEED 192
#define DC_MOTORS_DELTA_TORQUE_LIMIT1 5
#define DC_MOTORS_DELTA_TORQUE_LIMIT2 20
#define DC_MOTORS_DELTA_TORQUE_LIMIT3 35


result_t dcmotors_init();

result_t dcmotors_setLeftDCMotorSpeed(unsigned int _speed);
result_t dcmotors_setRightDCMotorSpeed(unsigned int _speed);

result_t dcmotors_setDirectionForward(bool const _direction);

result_t dcmotors_Task10ms();

#endif // DCMOTORS_H
