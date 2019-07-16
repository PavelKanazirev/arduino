// filename CarTimer.c
// project ProjectCar
// author Pavel

// depends on Timer2 that will be initialized in the DCMotors init

#include <stddef.h> // for using the NULL value

#include  "CarTimer.h"

static pTimer_callback_fxn_t _1ms_cb = NULL;

result_t cartimer_init(pTimer_callback_fxn_t _cb)
{
  result_t result = EOK;
  if (NULL != _cb)
  {
    _1ms_cb = _cb;

  }
  else
  {
    result = ENOK;
  }

//  TCCR2A = 0;// set entire TCCR2A register to 0
//  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1khz increments
  OCR2A = CAR_TIMER_COMPARE_VALUE;// = (16*10^6) / (8000*64) - 1 (must be <256)
  // turn on CTC mode
  BIT_SET(TCCR2A, WGM20); // mode set to Phase Correct PWM
  BIT_CLEAR(TCCR2A, WGM21);
  BIT_CLEAR(TCCR2A, WGM22);
  // Set CS21 bit for 64 prescaler
  BIT_SET(TCCR2B, CS22); // CS20 - CS22 b100 prescaler set to 64
  BIT_CLEAR(TCCR2B, CS21); // CS20 - CS22 b100 prescaler set to 64
  BIT_CLEAR(TCCR2B, CS20); // CS20 - CS22 b100 prescaler set to 64 
  // enable timer compare interrupt
  BIT_CLEAR(TCCR2A, COM2A0);
  BIT_CLEAR(TCCR2A, COM2B0);
  
  TIMSK2 |= (1 << OCIE2A);
 
  return result;
}

result_t cartimer_setTimerToSchedule()
{
  result_t result = EOK;

  BIT_CLEAR(TCCR2A, WGM20);
  BIT_SET(TCCR2A, WGM21);
  BIT_CLEAR(TCCR2A, WGM22);
  //Setup Timer2 to fire every 1ms
  OCR2A = CAR_TIMER_COMPARE_VALUE;// = (16*10^6) / (8000*8) - 1 (must be <256)
  BIT_CLEAR(TCCR2A, COM2A1);
  BIT_CLEAR(TCCR2A, COM2B1);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  return result;  
}

result_t cartimer_waitIdle()
{
  result_t const result = EOK;
  
  _1ms_cb(result);
  
  return EOK;
}

ISR(TIMER2_COMPA_vect) // Interrupt service run when Timer/Counter1 reaches OCR1A (your own top value)
{   
  result_t const result = EOK;
  
  _1ms_cb(result);   
}

//ISR(TIMER2_OVF_vect)
//{ // preconfigured to occur on each 1 ms
//  result_t const result = EOK;
//  
//  _1ms_cb(result);
//
//  TCNT2 = 130;           //Reset Timer to 130 out of 255
//  TIFR2 = 0x00;          //Timer2 INT Flag Reg: Clear Timer Overflow Flag
//}
