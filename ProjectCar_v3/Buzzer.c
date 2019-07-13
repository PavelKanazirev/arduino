// filename Buzzer.c
// project ProjectCar
// author Pavel

#include "Buzzer.h"

result_t buzzer_init()
{
//  pinMode(buzzerPin, OUTPUT); // PD2 , pin 13
  BIT_SET(DDRD,DDD2); // set the PD2 port direction to output
  BIT_CLEAR(PORTD,PORTD2); // set the default value of the PD2 output to 0
  
  return EOK;
}

result_t buzzer_beep(bool const _enable)
{
  result_t result = EOK;
  if ( _enable )
  {
    // beep
    BIT_SET(PORTD,PD2); // set pin to 1
  }
  else
  {
    // silence
    BIT_CLEAR(PORTD,PD2); // set pin to 0
  }
  
  return result;
}
