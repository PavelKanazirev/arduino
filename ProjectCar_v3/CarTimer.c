// filename CarTimer.c
// project ProjectCar
// author Pavel

#include <stddef.h> // for using the NULL value

#include  "CarTimer.h"

static pTimer_callback_fxn_t _1ms_cb = NULL;

result_t cartimer_init(pTimer_callback_fxn_t _cb)
{
  result_t result = EOK;
  if (NULL != _cb)
  {
    _1ms_cb = _cb;
    
// TCCR1 is composed of two registers TCCR1A and TCCR1B 
    BIT_SET(TCCR1A,COM1A1); // Clear OC1A on compare match // TCCR1A controls the compare modes
    BIT_SET(TCCR1B,CS11); BIT_SET(TCCR1B,CS10); // set prescaler to 64
     // set the compare register to a value generating interrupts each 1 milli sec 1000*250*64 = 16 MHZ
    OCR1A = 250; // this value was set after some reverse engineering - appropriate should be 15625 for 1 second
    TCNT1H = 0; // TCNT1 consists of two registers
    TCNT1L = 0; // timer counter value cleared
    BIT_SET(TIMSK1,OCIE1A); // timer counter 1 output compare enable
  }
  else
  {
    result = ENOK;
  }
  
  return result;
}

result_t cartimer_waitIdle()
{
  result_t const result = EOK;
  
  _1ms_cb(result);
  
  return EOK;
}

ISR(TIMER1_COMPA_vect)
{ // preconfigured to occur on each 1 ms
    result_t const result = EOK;
  
    _1ms_cb(result);

    TCNT1H = 0; // timer counter value cleared
    TCNT1L = 0; // timer counter value cleared
}
