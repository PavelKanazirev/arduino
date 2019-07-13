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

static pTimer_callback_fxn_t cb = Callback1ms;
volatile static bool state_idle = false;
static unsigned int old_xAxis = 0U;
static unsigned int old_yAxis = 0U;

// init
void setup() {
  initialize_IOpins();

  cartimer_init(cb);
  joystick_init();
  buzzer_init();
  rgbled_init();
  dcmotors_init();

  Serial.begin(9600);
  TRACE_INFO("Start");
}

void Task1ms()
{
  boolean button_down = false;
  unsigned int xAxis = 0;
  unsigned int yAxis = 0;
  unsigned int xRefMax, xRefMin, yRefMax, yRefMin;
  unsigned int low, high = 0;
  static unsigned int milliseconds = 0U;
  milliseconds++;

  joystick_getHorn(&button_down);
  buzzer_beep(button_down);
  joystick_getXAxis(&xAxis, &low, &high);
  if (old_xAxis >= JOYSTICK_MAX - JOYSTICK_THRESHOLD) {
    xRefMax = JOYSTICK_MAX;
    xRefMin = old_xAxis;
  }
  else if (old_xAxis <= JOYSTICK_MIN + JOYSTICK_THRESHOLD) {
    xRefMin = JOYSTICK_MIN;
    xRefMax = old_xAxis;
  }
  else
  {
    xRefMax = old_xAxis+ JOYSTICK_THRESHOLD;
    xRefMin = old_xAxis- JOYSTICK_THRESHOLD;
  }

  if ( (xAxis > xRefMax) | (xAxis < xRefMin ) )
  {
    TRACE_INFO(" x = ");
    TRACE_INFO(xAxis);
    TRACE_INFO(" low = ");
    TRACE_INFO(low);
    TRACE_INFO(" high = ");
    TRACE_INFO(high);
    old_xAxis = xAxis; 
  }
  else
  {
//    old_yAxis = yAxis;
  }

  joystick_getYAxis(&yAxis, &low, &high);
  if (old_yAxis >= JOYSTICK_MAX - JOYSTICK_THRESHOLD) {
    yRefMax = JOYSTICK_MAX;
    yRefMin = old_yAxis;
  }
  else if (old_yAxis <= JOYSTICK_MIN + JOYSTICK_THRESHOLD) {
    yRefMin = JOYSTICK_MIN;
    yRefMax = old_yAxis;
  }
  else
  {
    yRefMax = old_yAxis + JOYSTICK_THRESHOLD;
    yRefMin = old_yAxis - JOYSTICK_THRESHOLD;
  }

  if ( (yAxis > yRefMax ) | (yAxis < yRefMin) )
  {
    TRACE_INFO(" y = ");
    TRACE_INFO(yAxis);
    TRACE_INFO(" low = ");
    TRACE_INFO(low);
    TRACE_INFO(" high = ");
    TRACE_INFO(high);
    old_yAxis = yAxis; 
  }
  else
  {
//    old_yAxis = yAxis;
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

static void Callback1ms(__attribute__((unused)) 
                result_t const _status)
{
  // this callback will be triggered by an ISR so it needs to end as quick as possible
  state_idle = false;
}
