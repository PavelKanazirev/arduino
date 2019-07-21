// filename CarTimer.c
// project ProjectCar
// author Pavel

// depends on Timer2 that will be initialized in the DCMotors init

#include <stddef.h> // for using the NULL value

#include "CommonTypes.h"
#include  "CarTimer.h"

static pTimer_callback_fxn_t _1ms_cb = NULL;
static volatile unsigned long timer_cmpr_counter = 0UL;

static unsigned long alarm_cmpr_counter = 0UL;
static unsigned long alarm_ticks = 0UL;
static pTimer_callback_fxn_t alarm_cb = NULL;

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

// this function is used only for checking the alarm if set
result_t cartimer_idle()
{
    result_t result = EOK;
    unsigned long milliseconds = 0LU;
    static unsigned int detect10milliseconds = 0U;

    if ( detect10milliseconds++ > CAR_TIMER_MS_IN_A_SECOND )
    { // aprox. 10 seconds ( 10sec 240 milliseconds
        detect10milliseconds = 0;
    }

    if ( NULL != alarm_cb )
    {
        if (timer_cmpr_counter == alarm_cmpr_counter) 
        {
            if (TCNT2 >= alarm_ticks)
            {
                alarm_cb(result);
                alarm_cb = NULL;
            }
        }
        else if (timer_cmpr_counter > alarm_cmpr_counter)
        {
             alarm_cb(result);
             alarm_cb = NULL;
        }
    }
    
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

// gets the ticks 0 ... 249 and the compare reset count 0 ... 4294967295UL
// each reset is done once in 2 milliseconds so more than 8589934 seconds can be identified
result_t cartimer_getTickCounter(unsigned int * const _pTicks, unsigned long * const _p2msCycles)
{
  result_t result = EOK;
  
  if (NULL != _pTicks)
  {
    *_pTicks = TCNT2;
    if ( NULL != _p2msCycles)
    {
        *_p2msCycles = timer_cmpr_counter;
    }
    else
    {
      result = ENULLPOINTER;
    }
  }
  else
  {
    result = ENULLPOINTER;
  }

  return result;
}

result_t cartimer_getMillisecondsSinceStart(unsigned long * const _pMilliseconds)
{
  result_t result = EOK;
  
  if (NULL != _pMilliseconds)
  {
      *_pMilliseconds = timer_cmpr_counter * 2;
      if (TCNT2 > (CAR_TIMER_COMPARE_VALUE / 2) )
      {
          (*_pMilliseconds)++;
      }
  }  
  else
  {
    result = ENULLPOINTER;
  }

  return result;
}

// provides callback triger for period no bigger than 10 seconds = 10 000 000 microseconds
result_t cartimer_setAlarmAfterMicroSeconds(unsigned long const _microSeconds, pTimer_callback_fxn_t _cb)
{
    result_t result = EOK;
    unsigned long total_alarm_ticks = (_microSeconds / 4) / 2;  // 1 millisecond / CAR_TIMER_COMPARE_VALUE = 0.25 microseconds; 500 Hzs
    unsigned long last_cmpr_counter = timer_cmpr_counter; // get one fixed value since it is changed by the timer

    if ( NULL != _cb )
    {
      alarm_cmpr_counter = last_cmpr_counter + total_alarm_ticks / (CAR_TIMER_COMPARE_VALUE + 1);     // calculate how many timer_cmpr_counter cycles will be expected
      alarm_ticks = last_cmpr_counter % (CAR_TIMER_COMPARE_VALUE + 1);            // calculate microseconds to tick number
  
      if ((alarm_ticks + TCNT2) > CAR_TIMER_COMPARE_VALUE)
      { // adding one more cycle if the current TCNT2 overflows after summing
          alarm_cmpr_counter++;
          alarm_ticks = alarm_ticks + TCNT2 - CAR_TIMER_COMPARE_VALUE;
      }
      else
      {
        alarm_ticks = alarm_ticks + TCNT2;
      }

       alarm_cb = _cb;     
    }
    else
    {
        result = ENULLPOINTER;
    }

    return result;
}

ISR(TIMER2_COMPA_vect) // Interrupt service run when Timer/Counter1 reaches OCR1A
{   
    result_t const result = EOK;
      
    _1ms_cb(result);
    timer_cmpr_counter++;
}
