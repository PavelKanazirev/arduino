#include "Initialize.h"

result_t initialize_IOpins()
{
  result_t result = EOK;

  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 0;
  TIMSK1 = 0;
  
  DDRB = 0;
  PORTB = 0;
  DDRD = 0;
  PORTD = 0;
  DDRC = 0;
  PORTC = 0;

  ADMUX = 0;

  return result;
}
