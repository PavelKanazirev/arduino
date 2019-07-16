// filename Joystick.h
// project ProjectCar
// author Pavel

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "CommonTypes.h"

#define JOYSTICK_MAX 255
#define JOYSTICK_MIN 0
#define JOYSTICK_THRESHOLD 2
#define JOYSTIC_CENTER_X_MIN 124
#define JOYSTIC_CENTER_X_MAX 132
#define JOYSTIC_CENTER_Y_MIN 124
#define JOYSTIC_CENTER_Y_MAX 132

typedef void (*command_callback_fxn_t)(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
typedef void (*btn_callback_fxn_t)(boolean const button_down);
typedef struct joystic_init_s {
  command_callback_fxn_t left_cmd_evt;
  command_callback_fxn_t right_cmd_evt;
  command_callback_fxn_t up_cmd_evt;
  command_callback_fxn_t down_cmd_evt;
  btn_callback_fxn_t btn_cmd_evt;
} joystic_init_t;

result_t joystick_init(joystic_init_t const _cbck_list);
result_t joystick_Task10ms();

#endif // JOYSTICK_H
