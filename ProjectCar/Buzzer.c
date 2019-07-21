// filename Buzzer.c
// project ProjectCar
// author Pavel

#include "Buzzer.h"

result_t buzzer_init()
{
//  OUTPUT // PB2 , pin 10
  BIT_SET(DDRB,DDB2); // set the PB2 port direction to output
  BIT_CLEAR(PORTB,PORTB2); // set the default value of the PB2 output to 0
  
  return EOK;
}

result_t buzzer_beep(bool const _enable)
{
  result_t result = EOK;
  if ( _enable )
  {
    // beep
    BIT_SET(PORTB,PB2); // set pin to 1
  }
  else
  {
    // silence
    BIT_CLEAR(PORTB,PB2); // set pin to 0
  }
  
  return result;
}
