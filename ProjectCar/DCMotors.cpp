// filename DCMotors.c
// project ProjectCar
// author Pavel

#include "CommonTypes.h"

extern "C" {
#include "DCMotors.h"
#include "CarTimer.h"
}

int pin3 = 3;

static unsigned int current_left_wheel_speed = 0U;
static unsigned int current_right_wheel_speed = 0U;
static bool directionForward = true;
static unsigned int milliseconds = 0U;

/*
Driver Right Motor En - pin 11 ( PWM ) - out - PB3
Driver Right Motor In1 - 6 - out - PD6
Driver Right Motor In2 - 7 - out - PD7
Driver Left Motor En - pin 3 ( PWM ) - out - PD3
Driver Left Motor In3 - 8 - out - PB0
Driver Left Motor In4 - 9 - out - PB1 substituted with 13 - PB5
*/

result_t dcmotors_init()
{
//  OUTPUT // PD6 , pin 6 = L298N in1 - right motor direction
  BIT_SET(DDRD,DDD6); // set the PD6 port direction to output
  BIT_CLEAR(PORTD,PD6); // set the default value of the PD6 output to 0
//  OUTPUT // PD7 , pin 7 = L298N in2 - right motor direction
  BIT_SET(DDRD,DDD7); // set the PD7 port direction to output
  BIT_CLEAR(PORTD,PD7); // set the default value of the PD7 output to 0
//  OUTPUT // PB0 , pin 8 = L298N in3 - left motor direction
  BIT_SET(DDRB,DDB0); // set the PB0 port direction to output
  BIT_CLEAR(PORTB,PB0); // set the default value of the PB0 output to 0
//  OUTPUT // PB1 , pin 9 = L298N in4 - left motor direction
  BIT_SET(DDRB,DDB1); // set the PB1 port direction to output
  BIT_CLEAR(PORTB,PB1); // set the default value of the PB1 output to 0
  
//  OUTPUT // PB3 , pin 11, PWM  = L298N en - right motor torque
  BIT_SET(DDRB,DDB3); // set the PB3 port direction to output
  BIT_CLEAR(PORTB,PB3); // set the default value of the PB3 output to 0
//  OUTPUT // PD3 , pin 3, PWM  = L298N en - left motor torque
  BIT_SET(DDRD,DDD3); // set the PD3 port direction to output
  BIT_CLEAR(PORTD,PD3); // set the default value of the PD3 output to 0
//  pinMode(pin3, OUTPUT);
//  digitalWrite(pin3, LOW);

  // PWM frequency and duty cycle set
  //  The COM2A1 and COM2A0 control the PWM of the digital pin 11. 
  // connect pwm to pins on timer 
  BIT_CLEAR(TCCR2A, COM2A0); // clear OC2A on compare match
  BIT_CLEAR(TCCR2A, COM2A1); // clear the bit that will be used for enabling
  //  The COM2B1 and COM2B0 control the PWM of the digital pin 3.
  BIT_CLEAR(TCCR2A, COM2B0); // clear OC2B on compare match
  BIT_CLEAR(TCCR2A, COM2B1); // clear the bit that will be used for enabling
  // configure timer 2 for phase correct pwm (8-bit)
  // counter is incremented from 0 to the value stored in OCR2A
// The type of timer/counter 2 is mode 1, the Phase Correct PWM
  BIT_SET(TCCR2A, WGM20); // WGM22 - WGM20 b001 - PWM, Phase Correct chosen
  BIT_CLEAR(TCCR2A, WGM21); // WGM22 - WGM20 b001 - PWM, Phase Correct chosen
  BIT_CLEAR(TCCR2B, WGM22); // WGM22 - WGM20 b001 - PWM, Phase Correct chosen
  // set timer 2 prescale factor to 64
  BIT_SET(TCCR2B, CS22); // CS20 - CS22 b100 prescaler set to 64
  BIT_CLEAR(TCCR2B, CS21); // CS20 - CS22 b100 prescaler set to 64
  BIT_CLEAR(TCCR2B, CS20); // CS20 - CS22 b100 prescaler set to 64
  // 16000000 / (64*510) = 490 Hz
  return EOK;
}

result_t dcmotors_setTimerToPwmMode()
{
// // PWM frequency and duty cycle set
  //  The COM2A1 and COM2A0 control the PWM of the digital pin 11. 
  // connect pwm to pins on timer 
  BIT_CLEAR(TCCR2A, COM2A0); // clear OC2A on compare match
  BIT_CLEAR(TCCR2A, COM2A1); // clear the bit that will be used for enabling
  //  The COM2B1 and COM2B0 control the PWM of the digital pin 3.
  BIT_CLEAR(TCCR2A, COM2B0); // clear OC2B on compare match
  BIT_CLEAR(TCCR2A, COM2B1); // clear the bit that will be used for enabling

// The type of timer/counter 2 is mode 1, the Phase Correct PWM
  BIT_SET(TCCR2A, WGM20); // WGM22 - WGM20 b001 - PWM, Phase Correct chosen
  BIT_CLEAR(TCCR2A, WGM21); // WGM22 - WGM20 b001 - PWM, Phase Correct chosen
  BIT_CLEAR(TCCR2B, WGM22); // WGM22 - WGM20 b001 - PWM, Phase Correct chosen

  return EOK;
}

result_t dcmotors_setLeftDCMotorSpeed(unsigned int _speed)
{
  if (DC_MOTORS_MIN_SPEED >= _speed)
  {
    current_left_wheel_speed = 0;
  }
  else if (DC_MOTORS_MAX_SPEED < _speed)
  {
    current_left_wheel_speed = DC_MOTORS_MAX_SPEED;
  }
  else
  {
    current_left_wheel_speed = _speed;
  }

  // set the duty cycle value for pin 3 - left PWM
  if (0 < current_left_wheel_speed)
  { 
//    dcmotors_setTimerToPwmMode();
    BIT_SET(TCCR2A, COM2B1);
    OCR2B = current_left_wheel_speed; // set pwm duty for pin 3
  }
  else
  {
    BIT_SET(TCCR2A, COM2B1);
    OCR2B = current_left_wheel_speed; // set pwm duty for pin 3
//    cartimer_setTimerToSchedule();
  }

  return EOK;  
}

result_t dcmotors_setRightDCMotorSpeed(unsigned int _speed)
{
  if (DC_MOTORS_MIN_SPEED >= _speed)
  {
    current_right_wheel_speed = 0;
  }
  else if (DC_MOTORS_MAX_SPEED < _speed)
  {
    current_right_wheel_speed = DC_MOTORS_MAX_SPEED;
  }
  else
  {
    current_right_wheel_speed = _speed;
  }

  // set the duty cycle value for pin 11 - right PWM
  if (0 < current_right_wheel_speed)
  {
//    dcmotors_setTimerToPwmMode();
    BIT_SET(TCCR2A, COM2A1);
    OCR2A = current_right_wheel_speed; // set pwm duty
  }
  else
  {
    BIT_SET(TCCR2A, COM2A1);
    OCR2A = current_right_wheel_speed; // set pwm duty
//    cartimer_setTimerToSchedule();
  }
  
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

result_t dcmotors_Task10ms()
{
  // signal the motors based on current_left_wheel_speed, current_right_wheel_speed and directionForward
  // PB3 , pin 11, PWM  = L298N en - right motor torque
  // PD3 , pin 3, PWM  = L298N en - left motor torque
  milliseconds++;
  dcmotors_setDirectionForward(directionForward);
  dcmotors_setLeftDCMotorSpeed(current_left_wheel_speed);
  dcmotors_setRightDCMotorSpeed(current_right_wheel_speed);

  if ((0U < current_left_wheel_speed) && (true != directionForward))
  {
    TRACE_WARNING("Gotcha : ");
//    TRACE_WARNING(milliseconds);

    TRACE_WARNING("left speed");
    TRACE_WARNING(current_left_wheel_speed);    
  }

  if (milliseconds > 500)
  {    
    milliseconds = 0;
  }
  
  return EOK;
}
