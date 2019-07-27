// filename UltrasonicSensor.h
// project ProjectCar
// author Pavel

#ifndef ULTRASONICSENSOR_H
#define ULTRASONICSENSOR_H

#include "CommonTypes.h"

#define ULTRASONICSENSOR_SAFE_STOP 125
#define ULTRASONICSENSOR_SAFE_DISTANCE_FAST_FWD_DRIVE 192
#define ULTRASONICSENSOR_SAFE_DISTANCE_SLOW_FWD_DRIVE 250

#define ULTRASONICSENSOR_COUNTER_STOPPED 0xFF
#define ULTRASONICSENSOR_COUNTER_START 0x06 // trigger is recommended to be sent once on each 60 ms
#define ULTRASONICSENSOR_COUNTER_READY 0x00

#define ULTRASONICSENSOR_MAX_UNSIGNED_LONG 1
#define ULTRASONICSENSOR_THRESHOLD 32
#define ULTRASONICSENSOR_MIN_PING_DISTANCE_MM 50
#define ULTRASONICSENSOR_MAX_VALUE 50000
#define ULTRASONICSENSOR_MAX_PING_DISTANCE_MM 40000
  // The approximate speed of sound in dry air is given by the formula:
  // c = 331.5 (m/s) + 0.6 * [air temperature in degrees Celsius]
  // Give the temperatue of 21 Celcius;
  // 331.5 + (0.6 * 21) = 344.1 meter/sec = 34410 cm/sec
  // 34410 cm/sec = (34410 / 1000) cm/msec = 0.034410 cm/microsec
#define ULTRASONICSENSOR_SPEED_SOUND_CM_PER_USEC 23

typedef void (*ultraSensor_echo_callback_fxn_t)(unsigned int const _distance);

result_t ultraSensor_init(ultraSensor_echo_callback_fxn_t _cb);
result_t ultraSensor_Task10ms();

#endif // ULTRASONICSENSOR_H
