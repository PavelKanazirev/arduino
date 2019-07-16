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
  #include "Joystick.h"
  #include "Buzzer.h"
  #include "RGBLed.h"
  #include "DCMotors.h"
}

// globals

// global functions
static void Callback1ms(result_t const _status);
static void JoysticLeftCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticRightCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticUpCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticDownCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta);
static void JoysticButtonChangeDetected(boolean const button_down);
static bool currentPointInCenter(unsigned int const _xAxis, unsigned int const _yAxis);

static pTimer_callback_fxn_t cb = Callback1ms;
static bool state_idle = false;

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

unsigned int current_direction = EIDLE;
unsigned int current_left_wheel_speed = 0;
unsigned int current_right_wheel_speed = 0;

// init
void setup() {
  joystic_init_t joystick_callback_list;
  
  initialize_IOpins();
  joystick_callback_list.left_cmd_evt = &JoysticLeftCommandDetected;
  joystick_callback_list.right_cmd_evt = &JoysticRightCommandDetected;
  joystick_callback_list.down_cmd_evt = &JoysticDownCommandDetected;
  joystick_callback_list.up_cmd_evt = &JoysticUpCommandDetected;
  joystick_callback_list.btn_cmd_evt = &JoysticButtonChangeDetected;
  cartimer_init(cb);
  joystick_init(joystick_callback_list);
  buzzer_init();
  rgbled_init();
  dcmotors_init();

  Serial.begin(9600);
  TRACE_WARNING("Start");
}

void Task1ms()
{
  static unsigned int detect10milliseconds = 0U;
  if ( detect10milliseconds++ > 10000 )
  {
    TRACE_WARNING(" 10 sec ");
    detect10milliseconds = 0;
  }

  if ( 0 == ( detect10milliseconds % 10 ) )
  {
    joystick_Task10ms();
    dcmotors_Task10ms();
  }
}

// main loop
void loop() {
  if (false == state_idle)
  {
    Task1ms();
    state_idle = true;
  }
}

static void Callback1ms(result_t const _status)
{
  // this callback will be triggered by an ISR so it needs to end as quick as possible
  state_idle = false;
  if (EOK != _status)
  {
    TRACE_ERROR("Timer callback issue");
  }
}

static void JoysticLeftCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    current_left_wheel_speed = DC_MOTORS_STOP;
    current_right_wheel_speed = DC_MOTORS_STOP;
//    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction,ERIGHT_BIT))
  {
    if (BIT_IS_SET(current_direction, EUP_BIT))
    {
      if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT2)
      {
        current_left_wheel_speed = DC_MOTORS_MIN_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR3_SPEED;        
      }
      else
      {
        current_left_wheel_speed = DC_MOTORS_STOP;
        current_right_wheel_speed = DC_MOTORS_GEAR2_SPEED;
      }
    }
    else if (BIT_IS_SET(current_direction, EDOWN_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_STOP;
        current_right_wheel_speed = DC_MOTORS_GEAR1_SPEED;      
    }

    ENUM_BIT_SET(current_direction, ELEFT_BIT);
//    TRACE_INFO("left ");
//    TRACE_INFO(_xAxis);
//    TRACE_INFO("delta ");
//    TRACE_INFO(_delta);
//    TRACE_INFO("current_direction = ");
//    TRACE_INFO(current_direction);
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
//    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction,ELEFT_BIT))
  {
    if (BIT_IS_SET(current_direction, EUP_BIT))
    {
      if (_delta > DC_MOTORS_DELTA_TORQUE_LIMIT2)
      {
        current_left_wheel_speed = DC_MOTORS_GEAR3_SPEED;
        current_right_wheel_speed = DC_MOTORS_MIN_SPEED;
      }
      else
      {
        current_left_wheel_speed = DC_MOTORS_GEAR2_SPEED;
        current_right_wheel_speed = DC_MOTORS_STOP;        
      }
    }
    else if (BIT_IS_SET(current_direction, EDOWN_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_GEAR1_SPEED;
        current_right_wheel_speed = DC_MOTORS_STOP;      
    }

    ENUM_BIT_SET(current_direction, ERIGHT_BIT);
//    TRACE_INFO("right");
//    TRACE_INFO(_xAxis);
//    TRACE_INFO("delta ");
//    TRACE_INFO(_delta);
//    TRACE_INFO("current_direction = ");
//    TRACE_INFO(current_direction);
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
//    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction, EDOWN_BIT))
  { 
    if (BIT_IS_SET(current_direction, ELEFT_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_MIN_SPEED;
        current_right_wheel_speed = DC_MOTORS_GEAR3_SPEED;
    }
    else if (BIT_IS_SET(current_direction, ERIGHT_BIT))
    {
        current_left_wheel_speed = DC_MOTORS_GEAR3_SPEED;
        current_right_wheel_speed = DC_MOTORS_MIN_SPEED;     
    }
    else
    {
      ENUM_BIT_SET(current_direction, EUP_BIT);
  //    TRACE_INFO("up");
  //    TRACE_INFO(_yAxis);
  //    TRACE_INFO("delta ");
  //    TRACE_INFO(_delta);
  //    TRACE_INFO("current_direction = ");
  //    TRACE_INFO(current_direction);
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
//    TRACE_INFO("Center");
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
  //    TRACE_INFO("down");
  //    TRACE_INFO(_yAxis);
  //    TRACE_INFO("delta ");
  //    TRACE_INFO(_delta);
  //    TRACE_INFO("current_direction = ");
  //    TRACE_INFO(current_direction);
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

  dcmotors_setLeftDCMotorSpeed(current_left_wheel_speed);
  dcmotors_setRightDCMotorSpeed(current_right_wheel_speed);
}

static void JoysticButtonChangeDetected(
  boolean const button_down)
{
  buzzer_beep(button_down);
  
  if (button_down)
  {
    TRACE_DEBUG("button down");
  }
  else 
  {
    TRACE_DEBUG("button up");
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
