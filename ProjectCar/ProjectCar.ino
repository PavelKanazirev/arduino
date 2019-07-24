// filename ProjectCar
// project ProjectCar
// author Pavel

// system defines

// include system files

// include project files
#include "CommonTypes.h"

extern "C" {
  #include "Initialize.h"
  #include "CarTimer.h"
  #include "UltrasonicSensor.h"
  #include "Joystick.h"
  #include "Buzzer.h"
  #include "RGBLed.h"
  #include "DCMotors.h"
  #include "twi_lcd_driver.h"
}

// global types
typedef enum direction_e {
  EIDLE,
  ELEFT = 1,
  ERIGHT = 2,
  EUP = 4,
  EDOWN = 8
} direction_t;

typedef enum direction_bit_e {
  ELEFT_BIT = 0,
  ERIGHT_BIT = 1,
  EUP_BIT = 2,
  EDOWN_BIT = 3
} direction_bit_t;

typedef enum risk_level_e {
  ERISK_UNDEFINED = 0,
  ERISK_SELF_DIAG,  // check sensor feedbacks
  ERISK_NO,   // green light = OK
  ERISK_SMALL,  // light a telltale only = WRN
  ERISK_BIG,  // trigger the buzzer = ATT
  ERISK_CRITICAL, // stop the engines - discard the joystick forward command inputs = EEL
  ERISK_MAX 
} risk_level_t;

// global functions
static void CallbackTimerCmpr(result_t const _status);
static void UltraSensorDistanceChange(unsigned int const _distance);
static void JoysticLeftCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticRightCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticUpCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticDownCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticButtonChangeDetected(boolean const button_down);

static bool currentPointInCenter(unsigned int const _xAxis, unsigned int const _yAxis);
static unsigned char risk_to_ch(unsigned int _risk );
static unsigned char direction_to_ch( unsigned int _direction );
static unsigned char speed_to_gear_ch( unsigned int _speed );

/*
 * this funxtion will use for read-only purposes the data from the global variables
 * unsigned int current_distance, unsigned int current_direction, current_left_wheel_speed, current_right_wheel_speed
 */
static result_t projectCar_ADAS_Task10ms(risk_level_t * const _pRisk);

// global variables
static pTimer_callback_fxn_t cb = CallbackTimerCmpr;
static bool state_idle = false;
static bool alarm_set = false;
static bool btn_pressed = false;

static unsigned int current_distance = 0;
static unsigned int current_direction = EIDLE;
static unsigned int current_left_wheel_speed = 0;
static unsigned int current_right_wheel_speed = 0;

risk_level_t current_risk = ERISK_UNDEFINED;

// init
void setup() {
  joystic_init_t joystick_callback_list;

  initialize_IOpins();
  initLCD();  

  joystick_callback_list.left_cmd_evt = &JoysticLeftCommandDetected;
  joystick_callback_list.right_cmd_evt = &JoysticRightCommandDetected;
  joystick_callback_list.down_cmd_evt = &JoysticDownCommandDetected;
  joystick_callback_list.up_cmd_evt = &JoysticUpCommandDetected;
  joystick_callback_list.btn_cmd_evt = &JoysticButtonChangeDetected;
  cartimer_init(cb);
  ultraSensor_init(UltraSensorDistanceChange);
  joystick_init(joystick_callback_list);
  buzzer_init();
  rgbled_init();
  dcmotors_init();
#if ( DEBUG_LEVEL_OFF != CURRENT_DEBUG_LEVEL )
  Serial.begin(9600);
#endif
  TRACE_WARNING("Start");
}

void Task2ms()
{
    static unsigned int detect10milliseconds = 0U;
    unsigned long milliseconds = 0LU;
    unsigned int ticks = 0U;
    unsigned long cycles = 0LU;
    rgb_color_t rgbbcolor = ECOLOR_NONE;
    boolean enableLed = false;
    boolean enableBeep = false;
    
    if ( detect10milliseconds++ > ( CAR_TIMER_TIME_SLICE_IN_10MS * CAR_TIMER_MS_IN_A_SECOND ) )
    { // aprox. 10 seconds ( 10sec 240 milliseconds
        detect10milliseconds = 0;
        cartimer_getMillisecondsSinceStart(&milliseconds);
        cartimer_getTickCounter(&ticks, &cycles);
    }
  
    if ( 0 == ( detect10milliseconds % CAR_TIMER_TIME_SLICE_IN_10MS ) )
    {
        joystick_Task10ms();
        dcmotors_Task10ms();
        if ( EOK != ultraSensor_Task10ms(&alarm_set) )
        {
            TRACE_ERROR(alarm_set);
        }
        
        if ( EOK != projectCar_ADAS_Task10ms(&current_risk) )
        {
            TRACE_ERROR(current_risk);
        }
        
        if ( ERISK_NO == current_risk )
        {// green light 
            rgbbcolor = ECOLOR_GREEN;
            enableLed = true;
            enableBeep = false;          
        }
        else if ( ERISK_SMALL == current_risk )
        {// light a red telltale only 
            rgbbcolor = ECOLOR_RED;
            enableLed = true;
            enableBeep = false;
        }
        else if ( ERISK_BIG == current_risk )
        {// trigger the buzzer, red light on
            rgbbcolor = ECOLOR_RED;
            enableLed = true;
            enableBeep = true;
            // emergency case - alarm is on - joystick button handler needs to sync its actions
            buzzer_beep(enableBeep); // no matter if the button is pressed or not - ring the bell
        }
        else if ( ERISK_CRITICAL == current_risk )
        {
            rgbbcolor = ECOLOR_RED;
            enableLed = true;
            enableBeep = true;
            // emergency case - alarm is on - joystick button handler and forward command needs to sync its actions
            buzzer_beep(enableBeep); // no matter if the button is pressed or not - ring the bell
        }
        else
        {
            rgbbcolor = ECOLOR_NONE;
            enableLed = false;
            enableBeep = false;
        }
    
        rgbled_setcolor(rgbbcolor);
        rgbled_setLed(enableLed);
        if (! btn_pressed) // synchronizing the state and the handler commands
        { // in the other use case the handler controls the buzzer state
            buzzer_beep(enableBeep);
        }

        if ( display_Task10ms( risk_to_ch(current_risk),
            direction_to_ch(current_direction), 
            speed_to_gear_ch(current_left_wheel_speed), 
            speed_to_gear_ch(current_right_wheel_speed) ) )
        {
            TRACE_ERROR(current_direction);
        }
    }
}

// main loop
void loop() {
  if (false == state_idle)
  {
      Task2ms();
      state_idle = true;
  }
  
  if (alarm_set)
  {
      cartimer_idle();
  }
}

static void CallbackTimerCmpr(result_t const _status)
{
  // this callback will be triggered by an ISR so it needs to end as quick as possible
  // current configuration relies on this callback to be triggered once on each 2 milliseconds
  state_idle = false;
  if (EOK != _status)
  {
    TRACE_ERROR("Timer callback issue");
  }
}

static void UltraSensorDistanceChange(unsigned int const _distance)
{
  if (current_distance != _distance)
  {
      TRACE_INFO("Old distance:");
      TRACE_INFO(current_distance);
      TRACE_INFO("New distance:");
      TRACE_INFO(_distance);
      current_distance = _distance;
  }
}

static void JoysticLeftCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    current_left_wheel_speed = DC_MOTORS_STOP;
    current_right_wheel_speed = DC_MOTORS_STOP;
  }
  else if (!BIT_IS_SET(current_direction,ERIGHT_BIT))
  {
      if (BIT_IS_SET(current_direction, EUP_BIT))
      {
          if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT2)
          {
              current_left_wheel_speed = DC_MOTORS_GEAR1_SPEED;
              current_right_wheel_speed = DC_MOTORS_GEAR3_SPEED;
          }
          else
          {
              current_left_wheel_speed = DC_MOTORS_MIN_SPEED;
              current_right_wheel_speed = DC_MOTORS_GEAR2_SPEED;
          }
      }
      else if (BIT_IS_SET(current_direction, EDOWN_BIT))
      {
          current_left_wheel_speed = DC_MOTORS_STOP;
          current_right_wheel_speed = DC_MOTORS_GEAR1_SPEED;      
      }
  
      ENUM_BIT_SET(current_direction, ELEFT_BIT);
  }
  else
  {
      ENUM_BIT_CLEAR(current_direction,ERIGHT_BIT);
  }

  dcmotors_setLeftDCMotorSpeed(current_left_wheel_speed);   // left motor slower
  dcmotors_setRightDCMotorSpeed(current_right_wheel_speed); // right motor faster
}

static void JoysticRightCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    current_left_wheel_speed = DC_MOTORS_STOP;
    current_right_wheel_speed = DC_MOTORS_STOP;
  }
  else if (!BIT_IS_SET(current_direction,ELEFT_BIT))
  {
    if (BIT_IS_SET(current_direction, EUP_BIT))
    {
      if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT2)
      {
        current_left_wheel_speed = DC_MOTORS_GEAR3_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR1_SPEED;
      }
      else
      {
        current_left_wheel_speed = DC_MOTORS_GEAR2_SPEED;
        current_right_wheel_speed = DC_MOTORS_MIN_SPEED;        
      }
    }
    else if (BIT_IS_SET(current_direction, EDOWN_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_GEAR1_SPEED;
        current_right_wheel_speed = DC_MOTORS_STOP;      
    }

    ENUM_BIT_SET(current_direction, ERIGHT_BIT);
  }
  else
  {
      ENUM_BIT_CLEAR(current_direction,ELEFT_BIT);
  }

  dcmotors_setLeftDCMotorSpeed(current_left_wheel_speed);   // left motor faster
  dcmotors_setRightDCMotorSpeed(current_right_wheel_speed); // right motor slower
}

static void JoysticUpCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    current_left_wheel_speed = DC_MOTORS_STOP;
    current_right_wheel_speed = DC_MOTORS_STOP;
  }
  else if (!BIT_IS_SET(current_direction, EDOWN_BIT))
  {
    if (BIT_IS_SET(current_direction, ELEFT_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_GEAR1_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR3_SPEED;
    }
    else if (BIT_IS_SET(current_direction, ERIGHT_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_GEAR3_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR1_SPEED;     
    }
    else
    {
      ENUM_BIT_SET(current_direction, EUP_BIT);
      dcmotors_setDirectionForward(BIT_IS_SET(current_direction,EUP_BIT));
      // calculate speed based on _yAxis and _delta      
      if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT3)
      {
        current_left_wheel_speed = DC_MOTORS_MAX_SPEED;
        current_right_wheel_speed = DC_MOTORS_MAX_SPEED;
      }
      else if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT2)
      {
        current_left_wheel_speed = DC_MOTORS_GEAR3_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR3_SPEED;
      }
      else if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT1)
      {
        current_left_wheel_speed = DC_MOTORS_GEAR2_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR2_SPEED;        
      }
    }
  }
  else
  {
      ENUM_BIT_CLEAR(current_direction, EDOWN_BIT);
  }

  dcmotors_setLeftDCMotorSpeed(current_left_wheel_speed);
  dcmotors_setRightDCMotorSpeed(current_right_wheel_speed);
}

static void JoysticDownCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    current_left_wheel_speed = DC_MOTORS_STOP;
    current_right_wheel_speed = DC_MOTORS_STOP;
  }
  else if (!BIT_IS_SET(current_direction,EUP_BIT))
  {
    if (BIT_IS_SET(current_direction, ELEFT_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_STOP;
        current_right_wheel_speed = DC_MOTORS_GEAR1_SPEED;
    }
    else if (BIT_IS_SET(current_direction, ERIGHT_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_GEAR1_SPEED;
        current_right_wheel_speed = DC_MOTORS_STOP;      
    }
    else
    {
        ENUM_BIT_SET(current_direction, EDOWN_BIT);
        dcmotors_setDirectionForward(BIT_IS_SET(current_direction,EUP_BIT));
        // calculate speed based on _yAxis and _delta
        if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT2)
        {
            current_left_wheel_speed = DC_MOTORS_GEAR2_SPEED;
            current_right_wheel_speed = DC_MOTORS_GEAR2_SPEED;
        }
        else
        {
            current_left_wheel_speed = DC_MOTORS_GEAR1_SPEED;
            current_right_wheel_speed = DC_MOTORS_GEAR1_SPEED;        
        }
    }
  }
  else
  {
      ENUM_BIT_CLEAR(current_direction, EUP_BIT);
  }

  dcmotors_setLeftDCMotorSpeed(current_left_wheel_speed);
  dcmotors_setRightDCMotorSpeed(current_right_wheel_speed);
}

static void JoysticButtonChangeDetected(
  boolean const button_down)
{
    btn_pressed = button_down;
    if ( (ERISK_BIG != current_risk) && ( ERISK_CRITICAL != current_risk ) )
    {
        buzzer_beep(button_down);
    }
    else
    {
        buzzer_beep(button_down);
    }
}

static bool currentPointInCenter(unsigned int const _xAxis, unsigned int const _yAxis)
{
  bool result = false;
  if ( (_xAxis >= JOYSTIC_CENTER_X_MIN) && (_xAxis <= JOYSTIC_CENTER_X_MAX) )
  {
    if ((_yAxis >= JOYSTIC_CENTER_Y_MIN) && (_yAxis <= JOYSTIC_CENTER_Y_MAX))
    {
      result = true; 
    }
  }

  return result;
}

/*
 * this function will use for read-only purposes the data from the global variables
 * unsigned int current_distance, unsigned int current_direction, current_left_wheel_speed, current_right_wheel_speed
 */
static result_t projectCar_ADAS_Task10ms(risk_level_t * const _pRisk)
{
    result_t result = EOK;

    if ( NULL != _pRisk )
    {
        *_pRisk = ERISK_NO;   // green light
        if ( (DC_MOTORS_GEAR3_SPEED <= current_left_wheel_speed) &&
            (DC_MOTORS_GEAR3_SPEED <= current_right_wheel_speed) &&
            BIT_IS_SET(current_direction, EUP_BIT) && 
            ( current_distance <= ULTRASONICSENSOR_SAFE_STOP ) )
        { // risk is big - turn on the buzzer and stop the engines
            *_pRisk = ERISK_CRITICAL;
             TRACE_WARNING("current_distance");
             TRACE_WARNING(current_distance);
             TRACE_WARNING("current_direction");
             TRACE_WARNING(current_direction);
             TRACE_WARNING("current_left_wheel_speed");
             TRACE_WARNING(current_left_wheel_speed);
             TRACE_WARNING("current_right_wheel_speed");
             TRACE_WARNING(current_right_wheel_speed);            
        }
        else if ( (DC_MOTORS_GEAR3_SPEED <= current_left_wheel_speed) &&
            (DC_MOTORS_GEAR3_SPEED <= current_right_wheel_speed) &&
            BIT_IS_SET(current_direction, EUP_BIT) && 
            ( current_distance <= ULTRASONICSENSOR_SAFE_DISTANCE_FAST_FWD_DRIVE ) )
        { // risk is big - turn on the buzzer
            *_pRisk = ERISK_BIG;
             TRACE_WARNING("current_distance");
             TRACE_WARNING(current_distance);
             TRACE_WARNING("current_direction");
             TRACE_WARNING(current_direction);
             TRACE_WARNING("current_left_wheel_speed");
             TRACE_WARNING(current_left_wheel_speed);
             TRACE_WARNING("current_right_wheel_speed");
             TRACE_WARNING(current_right_wheel_speed);            
        }
        else if ( (DC_MOTORS_GEAR1_SPEED <= current_left_wheel_speed) &&
            (DC_MOTORS_GEAR1_SPEED <= current_right_wheel_speed) &&
            BIT_IS_SET(current_direction, EUP_BIT) && 
            ( current_distance <= ULTRASONICSENSOR_SAFE_STOP ) )
        { // risk is big - turn on the buzzer
            *_pRisk = ERISK_BIG;
             TRACE_WARNING("current_distance");
             TRACE_WARNING(current_distance);
             TRACE_WARNING("current_direction");
             TRACE_WARNING(current_direction);
             TRACE_WARNING("current_left_wheel_speed");
             TRACE_WARNING(current_left_wheel_speed);
             TRACE_WARNING("current_right_wheel_speed");
             TRACE_WARNING(current_right_wheel_speed);            
        }
        else if ( ( DC_MOTORS_GEAR1_SPEED <= current_left_wheel_speed ) &&
            ( DC_MOTORS_GEAR1_SPEED <= current_right_wheel_speed ) &&
            BIT_IS_SET( current_direction, EUP_BIT ) && 
            (current_distance <= ULTRASONICSENSOR_SAFE_DISTANCE_SLOW_FWD_DRIVE ) )
        { // telltale will be turned on only
            *_pRisk = ERISK_SMALL;
             TRACE_WARNING("current_distance");
             TRACE_WARNING(current_distance);
             TRACE_WARNING("current_direction");
             TRACE_WARNING(current_direction);
             TRACE_WARNING("current_left_wheel_speed");
             TRACE_WARNING(current_left_wheel_speed);
             TRACE_WARNING("current_right_wheel_speed");
             TRACE_WARNING(current_right_wheel_speed);            
        }
    }
    else
    {
        result = ENULLPOINTER;
    }

    return result;
}

unsigned char risk_to_ch(unsigned int _risk )
{
    unsigned char ch_result = TWI_LCD_DIR_N;

    if ( ERISK_SELF_DIAG == _risk )
    {
        ch_result = TWI_LCD_RISK_SELF_DIAG;
    }
    else if ( TWI_LCD_RISK_NO == _risk )
    {
        ch_result = TWI_LCD_RISK_NO;
    }
    else if ( TWI_LCD_RISK_SMALL == _risk )
    {
        ch_result = TWI_LCD_RISK_SMALL;
    }
    else if ( TWI_LCD_RISK_BIG == _risk )
    {
        ch_result = TWI_LCD_RISK_BIG;
    }
    else if (TWI_LCD_RISK_CRITICAL == _risk )
    {
        ch_result = TWI_LCD_RISK_CRITICAL;
    }
  
    return ch_result;
}

unsigned char direction_to_ch(unsigned int _direction )
{
    unsigned char ch_result = TWI_LCD_DIR_N;

    if ( ENUM_BIT_IS_SET(_direction, EUP_BIT) )
    {
        ch_result = TWI_LCD_DIR_FW;
    }
    else if ( ENUM_BIT_IS_SET( _direction, EDOWN_BIT ) )
    {
        ch_result = TWI_LCD_DIR_BW;
    }
  
    return ch_result;
}

unsigned char speed_to_gear_ch(unsigned int _speed)
{
    unsigned char ch_result = TWI_LCD_GEAR0_SPEED;

    TRACE_INFO("Speed");
    if ((_speed >= DC_MOTORS_MIN_SPEED) && (_speed < DC_MOTORS_GEAR1_SPEED))
    {
        ch_result = TWI_LCD_GEAR0_SPEED;
        TRACE_INFO(ch_result);
    }
    else if ((_speed >= DC_MOTORS_GEAR1_SPEED) && (_speed < DC_MOTORS_GEAR2_SPEED))
    {
        ch_result = TWI_LCD_GEAR1_SPEED;
        TRACE_INFO(ch_result);      
    }
    else if ((_speed >= DC_MOTORS_GEAR2_SPEED) && (_speed < DC_MOTORS_GEAR3_SPEED))
    {
        ch_result = TWI_LCD_GEAR2_SPEED;
        TRACE_INFO(ch_result);
    }
    else if ( (_speed >= DC_MOTORS_GEAR3_SPEED) && (_speed <= DC_MOTORS_MAX_SPEED) )
    {
        ch_result = TWI_LCD_GEAR3_SPEED;
        TRACE_INFO(ch_result);
    }

    TRACE_INFO(_speed);
    TRACE_INFO(ch_result);

    return ch_result;
}
