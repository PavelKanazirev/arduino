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
  TRACE_INFO("Start");
}

void Task1ms()
{
  static unsigned int milliseconds = 0U;
  milliseconds++;

  joystick_Task1ms();
}

// main loop
void loop() {
  if (false == state_idle)
  {
    Task1ms();
    state_idle = true;
  }
}

static void Callback1ms(__attribute__((unused)) 
                result_t const _status)
{
  // this callback will be triggered by an ISR so it needs to end as quick as possible
  state_idle = false;
}

static void JoysticLeftCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction,ERIGHT_BIT))
  {  
    current_direction = ELEFT;
    TRACE_INFO("left ");
    TRACE_INFO(_xAxis);
    TRACE_INFO("delta ");
    TRACE_INFO(_delta);
    TRACE_INFO("current_direction = ");
    TRACE_INFO(current_direction);
  }
}

static void JoysticRightCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction,ELEFT_BIT))
  { 
    current_direction = ERIGHT;
    TRACE_INFO("right");
    TRACE_INFO(_xAxis);
    TRACE_INFO("delta ");
    TRACE_INFO(_delta);
    TRACE_INFO("current_direction = ");
    TRACE_INFO(current_direction);
  }
}

static void JoysticUpCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction,EDOWN_BIT))
  { 
    current_direction = EUP;
    TRACE_INFO("up");
    TRACE_INFO(_yAxis);
    TRACE_INFO("delta ");
    TRACE_INFO(_delta);
    TRACE_INFO("current_direction = ");
    TRACE_INFO(current_direction);
  }
}

static void JoysticDownCommandDetected(unsigned int const _xAxis, unsigned int const _yAxis, int const _delta)
{
  if ( currentPointInCenter(_xAxis, _yAxis) )
  {
    current_direction = EIDLE;
    TRACE_INFO("Center");
  }
  else if (!BIT_IS_SET(current_direction,EUP_BIT))
  { 
    current_direction = EDOWN;
    TRACE_INFO("down");
    TRACE_INFO(_yAxis);
    TRACE_INFO("delta ");
    TRACE_INFO(_delta);
    TRACE_INFO("current_direction = ");
    TRACE_INFO(current_direction);
  }
}

static void JoysticButtonChangeDetected(
  boolean const button_down)
{
  buzzer_beep(button_down);
  
  if (button_down)
  {
    TRACE_INFO("button down");
  }
  else 
  {
    TRACE_INFO("button up");
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
