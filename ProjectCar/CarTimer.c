// filename CarTimer.c
// project ProjectCar
// author Pavel

// depends on Timer2 that will be initialized in the DCMotors init

#include <stddef.h> // for using the NULL value

#include "CommonTypes.h"
#include  "CarTimer.h"

static pTimer_callback_fxn_t _1ms_cb = NULL;

// approximate 4 microseconds for one tick ( 1 millisecond / 250 ticks )
static volatile unsigned int current_tick_cycles = 0UL; //

result_t cartimer_init(pTimer_callback_fxn_t _cb)
{
  result_t result = EOK;
  if (NULL != _cb)
  {
    _1ms_cb = _cb;

  }
  else
  {
    result = ENULLPOINTER;
  }

  cli(); // disable interrupt
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 1khz increments - 1 tick equals 1 millisecond / CAR_TIMER_COMPARE_VALUE
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

  // timer0
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0; // initialize the counter from 0
  OCR0A = 2; // sets the counter compare value to each 1 microsecond
  TCCR0A |= (1<<WGM01); // enable the CTC mode
  // Set CS01 and CS00 bits for 8 prescaler - one tick equals 1 * 2 / ( (16*10^6) / 8 ) = 1 microseconds
  BIT_SET(TCCR0B,CS01);
  BIT_CLEAR(TCCR0B,CS00);
  BIT_SET(TIMSK0,OCIE0A); //enable the interrupt
  sei(); //allows interrupts
 
  return result;
}

// designed to work with no more than 4500 microseconds - approx 4.5 millisecond
result_t cartimer_delayMicroseconds( unsigned int const micros )
{
    result_t result = EOK;

    if ( (micros <= CAR_TIMER_MAX_MICROSECONDS_DELAY_VALUE) && ( 0U <= micros ) )
    {
        current_tick_cycles = 0U; // reset the timer in order to escape number overflow

        // start the delay
        while ( current_tick_cycles <= micros )
        {
            // do nothing
        }

        // delay over - exit
    }
    else
    {
        result = EOVERFLOW;
    }

    return result;
}

ISR(TIMER2_COMPA_vect) // Interrupt service run when Timer/Counter1 reaches OCR1A
{   
    result_t const result = EOK;
      
    _1ms_cb(result);
}

ISR(TIMER0_COMPA_vect) 
{
    // occurs each microsecond  
    current_tick_cycles++;
}
