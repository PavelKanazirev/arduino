// filename DCMotors.c
// project ProjectCar
// author Pavel

#include "CommonTypes.h"

#include "DCMotors.h"

static unsigned int current_left_wheel_speed = 0U;
static unsigned int current_right_wheel_speed = 0U;
static bool directionForward = true;

/*
Driver Right Motor En - pin 11 ( PWM ) - out - PB3
Driver Right Motor In1 - 6 - out - PD6
Driver Right Motor In2 - 7 - out - PD7
Driver Left Motor En - pin 10 ( PWM ) - out - PB2
Driver Left Motor In3 - 8 - out - PB0
Driver Left Motor In4 - 9 - out - PB1
*/

result_t dcmotors_init()
{
//  OUTPUT // PD6 , pin 6 = L298N in1 - right motor direction
  BIT_SET(DDRD,DDD6); // set the PD6 port direction to output
  BIT_CLEAR(PORTD,PORTD6); // set the default value of the PD6 output to 0
//  OUTPUT // PD7 , pin 7 = L298N in2 - right motor direction
  BIT_SET(DDRD,DDD7); // set the PD7 port direction to output
  BIT_CLEAR(PORTD,PORTD7); // set the default value of the PD7 output to 0
//  OUTPUT // PB0 , pin 8 = L298N in3 - left motor direction
  BIT_SET(DDRB,DDB0); // set the PB0 port direction to output
  BIT_CLEAR(PORTB,PORTB0); // set the default value of the PB0 output to 0
//  OUTPUT // PB1 , pin 9 = L298N in4 - left motor direction
  BIT_SET(DDRB,DDB1); // set the PB1 port direction to output
  BIT_CLEAR(PORTB,PORTB1); // set the default value of the PB1 output to 0  
  
//  OUTPUT // PB3 , pin 11, PWM  = L298N en - right motor torque
  BIT_SET(DDRB,DDB3); // set the PB3 port direction to output
  BIT_CLEAR(PORTB,PORTB3); // set the default value of the PB3 output to 0
//  OUTPUT // PB2 , pin 10, PWM  = L298N en - left motor torque
  BIT_SET(DDRB,DDB2); // set the PB2 port direction to output
  BIT_CLEAR(PORTB,PORTB2); // set the default value of the PB2 output to 0
  
  return EOK;
}

result_t dcmotors_setLeftDCMotorSpeed(unsigned int _speed)
{  
  current_left_wheel_speed = _speed;

  // if currentSpeed < preCalcMinSpeed currentSpeed = 0;
//  TRACE_INFO("left wheel speed = ");
//  TRACE_INFO(current_left_wheel_speed);

  return EOK;  
}

result_t dcmotors_setRightDCMotorSpeed(unsigned int _speed)
{
  current_right_wheel_speed = _speed;

  // if currentSpeed < preCalcMinSpeed currentSpeed = 0;
//  TRACE_INFO("right wheel speed = ");
//  TRACE_INFO(current_right_wheel_speed);
  
  return EOK;  
}

result_t dcmotors_setDirectionForward(bool const _direction)
{
  directionForward = _direction;
  if (directionForward)
  {// set in1 HIGH, in2 LOW, in3 HIGH, in4 LOW
//    TRACE_INFO("direction forward");
    BIT_SET(PORTD,PD6); // PD6 , pin 6 = L298N in1 - right motor direction
    BIT_CLEAR(PORTD,PD7); // PD7 , pin 7 = L298N in2 - right motor direction
    BIT_SET(PORTB,PB0); // PB0 , pin 8 = L298N in3 - left motor direction
    BIT_CLEAR(PORTB,PB1); // PB1 , pin 9 = L298N in4 - left motor direction
  }
  else
  {// set in1 LOW, in2 HIGH, in3 LOW, in4 HIGH
//    TRACE_INFO("direction back");
    BIT_CLEAR(PORTD,PD6); // PD6 , pin 6 = L298N in1 - right motor direction
    BIT_SET(PORTD,PD7); // PD7 , pin 7 = L298N in2 - right motor direction
    BIT_CLEAR(PORTB,PB0); // PB0 , pin 8 = L298N in3 - left motor direction
    BIT_SET(PORTB,PB1); // PB1 , pin 9 = L298N in4 - left motor direction    
  }
  
  return EOK;  
}

result_t dcmotors_Task1ms()
{
  // signal the motors based on current_left_wheel_speed, current_right_wheel_speed and directionForward
  // PB3 , pin 11, PWM  = L298N en - right motor torque
  // PB2 , pin 10, PWM  = L298N en - left motor torque
  
  return EOK;
}
