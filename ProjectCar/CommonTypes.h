// filename CommonTypes.h
// project ProjectCar
// author Pavel

#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include "Arduino.h"

//#define TEST_TIMER 1
//#define TEST_JOYSTICK 1
//#define TEST_BUZZER 1
//#define TEST_RGBLED 1

#define DEBUG_LEVEL_OFF 0
#define DEBUG_LEVEL_RESTRICTED 1
#define DEBUG_LEVEL_OPTIMIZED 2
#define DEBUG_LEVEL_VERBOSE 3

// set the current debug level from this line, no need to perform other changes
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_OPTIMIZED

#define TRACE_ERROR(param) if (CURRENT_DEBUG_LEVEL > DEBUG_LEVEL_OFF) { \
  Serial.print("Error at "); \
  Serial.print("File : "); \
  Serial.print(__FILE__); \
  Serial.print(" ; Line : "); \
  Serial.print(__LINE__); \
  Serial.print(" ; "); \
  Serial.println(param); \
}

#define TRACE_WARNING(param) if (CURRENT_DEBUG_LEVEL > DEBUG_LEVEL_RESTRICTED) { \
  Serial.print("Warning at "); \
  Serial.print("File : "); \
  Serial.print(__FILE__); \
  Serial.print(" ; Line : "); \
  Serial.print(__LINE__); \
  Serial.print(" ; "); \
  Serial.println(param); \
}

#define TRACE_INFO(param) if (CURRENT_DEBUG_LEVEL > DEBUG_LEVEL_OPTIMIZED) { \
  Serial.print("Info "); \
  Serial.print("File : "); \
  Serial.print(__FILE__); \
  Serial.print(" ; Line : "); \
  Serial.print(__LINE__); \
  Serial.print(" ; "); \
  Serial.println(param); \
}

#define TRACE_DEBUG(param) if (CURRENT_DEBUG_LEVEL > DEBUG_LEVEL_VERBOSE) { \
  Serial.print("Debug "); \
  Serial.print("File : "); \
  Serial.print(__FILE__); \
  Serial.print(" ; Line : "); \
  Serial.print(__LINE__); \
  Serial.print(" ; "); \
  Serial.println(param); \
}

#define BIT_IS_SET(byte,bit) (byte & (1<<bit))
#define BIT_SET(byte,bit) ((byte) |= (1<<(bit)))
#define BIT_CLEAR(byte,bit) ((byte) &= ~(1<<(bit)))
#define BIT_FLIP(byte,bit) ((byte) ^= (1<<(bit)))

#define ENUM_BIT_SET(byte,bit) BIT_SET(byte,bit)
#define ENUM_BIT_CLEAR(byte,bit) BIT_CLEAR(byte,bit)

typedef enum result_e {
  EOK = 0,
  ENOK = 1
} result_t;

#endif // COMMONTYPES_H
