// filename DCMotors.c
// project ProjectCar
// author Pavel

#include "CommonTypes.h"
#include "limits.h"
#include "UltrasonicSensor.h"
#include "CarTimer.h"

typedef enum UltraSensor_states_e {
  EUNAVAILABLE = 0,
  ESTATE_TRIGGER_SENDING, // 1
  ESTATE_TRIGGER_SENT, // 2
  ESTATE_ECHO_RECEIVING, // 3
  ESTATE_ECHO_RECEIVED, // 4
  ESTATE_ECHO_IDLE, // 5
  ESTATE_MAX
} UltraSensor_states_t;

static UltraSensor_states_t current_state = EUNAVAILABLE;
static ultraSensor_echo_callback_fxn_t echo_callback;
static volatile boolean alarm_trigger_pending = false;

static unsigned int last_distance_measured = 0;

static volatile int echoPulse = 0;
static volatile int echoPinState = 0; // indicates if the Echo Pin goes from low to high

static result_t ultraSensor_sendTrigger();
static result_t ultraSensor_stopSendingTrigger();
static void ultraSensor_alarmCallback(result_t const _status);
static result_t ultraSensor_receiveEcho(unsigned int * const _pCurrent_distance_measured);
static boolean is_distance_change_real(unsigned int current_distance_measured );

result_t ultraSensor_init(ultraSensor_echo_callback_fxn_t _cb)
{
  result_t result = EOK;

  if (NULL != _cb)
  {
    echo_callback = _cb;
  }
  else
  {
    result = ENOK;
  }

  // pin 13 : PB5 Out - sends trigger signal
  BIT_SET(DDRB,PB5); // set the PB5 port direction to output
  BIT_CLEAR(PORTB,PB5); // set the default value of the PB5 output to 0
  // pin 2 : PD2 In - receive feedback about the distance of an object detected 2 to 400 sm 
  BIT_CLEAR(DDRD,DDD2); // set the PD2 port direction to input
//  BIT_SET(PORTD,PD2); // pull up resistor is activated for the the echo pin
  BIT_CLEAR(PORTD,PD2); // the rising edge output of ECHO port // obsolette call

  // The second instruction defines that any logical change at the INT0/PIND2 Pin will cause the microcontroller.
  BIT_SET(EICRA,ISC00); // Any logical change on INT0 generates an interrupt request
  // This Instruction is used to configure the PIN D2 as an interrupt PIN as the ECHO pin of the sensor is connected here.
  BIT_SET(EIMSK,INT0); // When INT0 bit is set and the I-bit in the Status Register (SREG) is set (one), the external pin interrupt is enabled.
//  BIT_SET(SREG,SREGI); // Global Interrupt Enable bit must be set for the interrupts to be enabled.
  sei();

  return result;
}

/*
 * The general sensor constraints are implemented using this state machine
 * In order to generate the ultrasound you need to set the Trig on a High State for 10 µs.
 * if the object is 10 cm away from the sensor, and the speed of the sound is 340 m/s or 0.034 cm/µs the sound wave will need to travel about 294 u seconds
 * may set an alarm - but need to inform the main loop 
 * designed like this for optimization purposes - when alarm is not set - timer task will not need to be running
 */
result_t ultraSensor_Task10ms(boolean * const _pAlarm_enabled)
{
  result_t result = EOK;
  static unsigned int pause_btw_triggers = ULTRASONICSENSOR_COUNTER_STOPPED;
  unsigned int current_distance_measured = 0;

  if ( NULL != _pAlarm_enabled )
  {
      if ( ULTRASONICSENSOR_COUNTER_STOPPED != pause_btw_triggers )
      {
        if ( ULTRASONICSENSOR_COUNTER_READY == pause_btw_triggers-- )
        {
            pause_btw_triggers = ULTRASONICSENSOR_COUNTER_STOPPED;
    //        *_pAlarm_enabled = false;
        }
      }
      
      if ( ( EUNAVAILABLE == current_state ) || ( ESTATE_ECHO_IDLE == current_state ) )
      {
          if ( ULTRASONICSENSOR_COUNTER_STOPPED == pause_btw_triggers )
          {
            pause_btw_triggers = ULTRASONICSENSOR_COUNTER_START;
            current_state = ESTATE_TRIGGER_SENDING;
            *_pAlarm_enabled = true;  // alarm is activated so that in the next call ( the receiving ) - callbacks to be working
          }
          
          result = EOK;
      }
      else // current_state not IDLE or UNAVL
      { // All these states are described after the alarm has been activated for the callbacks
          if ( ESTATE_TRIGGER_SENDING == current_state )
          {
              ultraSensor_sendTrigger();
      
              current_state = ESTATE_TRIGGER_SENT;
              result = EOK;
          }
    
          if ( ESTATE_TRIGGER_SENT == current_state )
          {
              ultraSensor_stopSendingTrigger();
        
              current_state = ESTATE_ECHO_RECEIVING;
              result = EOK;
          }
    
          if ( ESTATE_ECHO_RECEIVING == current_state )
          {
              result_t echoReadresult = ENOK;
              echoReadresult = ultraSensor_receiveEcho(&current_distance_measured);
              if ( EOK == echoReadresult )
              {
                  result = EOK;
                  current_state = ESTATE_ECHO_RECEIVED;
              }
              else if ( EOVERFLOW == echoReadresult )
              {   // wait for the timer value to not be in the overflow range
                  result = EOVERFLOW;
              }
              else if ( EINPROGRESS == echoReadresult )
              {   // ping is in progress - ISR for the timerinterupt will serve us to finish it
                  result = EOK;
              }
              else if ( ENULLPOINTER == echoReadresult )
              {   // another ping is in progress
                  result = ENULLPOINTER;
              }
          }
    
          if ( ESTATE_ECHO_RECEIVED == current_state )
          {
              if ( is_distance_change_real(current_distance_measured) )
              {
                echo_callback(current_distance_measured);
              }
        
              current_state = ESTATE_ECHO_IDLE;
              result = EOK;
          }
      } // current_state not IDLE or UNAVL
  }
  else
  {
    result = ENULLPOINTER;
  }

 
  return result;
}

static result_t ultraSensor_sendTrigger()
{
  result_t result = EOK;

  BIT_SET(PORTB,PB5); // set pin 13 : PB5 Out - sends signal  to 1
  delayMicroseconds(10);

  return result;
}

static result_t ultraSensor_stopSendingTrigger()
{
  result_t result = EOK;

  BIT_CLEAR(PORTB,PB5); // set pin 13 : PB5 Out - sends signal  to 1

  return result;
}

//static void ultraSensor_alarmCallback(result_t const _status)
//{
//  unsigned long microSeconds = 1000UL;
//  
//  alarm_trigger_pending = false;
//}

static result_t ultraSensor_receiveEcho(unsigned int * const _pCurrent_distance_measured)
{
   // store the final output value after processing. 
    static volatile unsigned int mm_distance = 0; // when divided by 58 gives the distance measure in centimeters.
    result_t result = EOK;

    if ( NULL != _pCurrent_distance_measured )
    {
      if ( 0 != echoPulse )
      {
        mm_distance = echoPulse / (ULTRASONICSENSOR_SPEED_SOUND_CM_PER_USEC * 2);
        if ( ULTRASONICSENSOR_MAX_PING_DISTANCE_MM < mm_distance )
        {
          mm_distance = ULTRASONICSENSOR_MAX_PING_DISTANCE_MM;
        }
        else if ( ULTRASONICSENSOR_MIN_PING_DISTANCE_MM > mm_distance )
        {
          mm_distance = ULTRASONICSENSOR_MIN_PING_DISTANCE_MM;
        }

        echoPulse = 0;
        result = EOK;
      }
      else
      {
        result = EINPROGRESS;
      }
  
      *_pCurrent_distance_measured = mm_distance;
    }
    else
    {
      result = ENULLPOINTER;
    }

    return result;
}

static boolean is_distance_change_real(unsigned int current_distance_measured )
{
  boolean result = false;

  if ((ULTRASONICSENSOR_THRESHOLD < current_distance_measured) && (ULTRASONICSENSOR_MAX_VALUE > current_distance_measured))
  {
    if ( last_distance_measured > current_distance_measured )
    {
      // object is comming closer than it used to be before
      if ( (last_distance_measured - current_distance_measured) >= ULTRASONICSENSOR_THRESHOLD )
      {
        result = true;
      }
    }
    else
    {
      // object is going away than it used to be before
      if ( (current_distance_measured - last_distance_measured) >= ULTRASONICSENSOR_THRESHOLD )
      {
        result = true;
      }
    }
  }
  else
  { // echo response bellow threshold
      result = false;
  }
  
  last_distance_measured = current_distance_measured;

  return result;
}

/*
 * The echo pin is the one that becomes high after it receives reflected waves. 
 * The time for which the echo pin is high is directly proportional to the distance of the obstacle from sensor.
 */
ISR(INT0_vect)
{
  if ( ( ESTATE_TRIGGER_SENT == current_state ) || (ESTATE_ECHO_RECEIVING == current_state) )
  {
    if ( 1 == echoPinState )
    { // change signal is detected and the echo pin was up
      TCCR1B = 0;
      echoPulse = TCNT1; // store the The count value
      TCNT1 = 0;
      echoPinState = 0;
    }
    
    if( 0 == echoPinState )
    { // change signal is detected and the echo pin was down
      // Setting TCCR's CS10 bit to 1 starts the timer with a prescaling of 1
      TCCR1B |= 1<<CS10; // clock prescaler 1 - no prescaling
      echoPinState = 1;
    }
  }
}
