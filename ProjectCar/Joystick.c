// filename Joystick.c
// project ProjectCar
// author Pavel
#include "Joystick.h"

static unsigned int xPosition = 0;
static unsigned int yPosition = 0;
static bool buttonPressed = 0;
static joystic_init_t callback_list;

static result_t joystick_getXAxis(unsigned int * const _pX_value, unsigned int * const _low, unsigned int * const _high);
static result_t joystick_getYAxis(unsigned int * const _pX_value, unsigned int * const _low, unsigned int * const _high);
static result_t joystick_getHorn();

result_t joystick_init(joystic_init_t const _cbck_list)
{
  // joystick x ADC0 (PC0), pin A0 DDRC0, 
  BIT_CLEAR(DDRC,DDC0); // If the bit of corresponding DDR is 0, the pin is configured as an input
  BIT_CLEAR(PORTC,PORTC0); // set default value to 0
  // joystick y ADC1 (PC1), pin A1 DDRC1
  BIT_CLEAR(DDRC,DDC1); // If the bit of corresponding DDR is 0, the pin is configured as an input
  BIT_CLEAR(PORTC,PORTC1); // set default value to 0

  // joystick button PB4, pin 12
  //activate pull-up resistor on the push-button pin
  //  pinMode(buttonPin, INPUT_PULLUP);
  BIT_CLEAR(DDRB,DDB4); // set the PB4 port direction to input
  BIT_SET(PORTB,PB4); // pull up resistor is activated for the button

  BIT_SET(ADCSRA,ADEN); // enables ADC
  // set a2d prescale factor to 128
  // 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
  BIT_SET(ADCSRA, ADPS2);
  BIT_SET(ADCSRA, ADPS1);
  BIT_SET(ADCSRA, ADPS0);

  callback_list.left_cmd_evt = _cbck_list.left_cmd_evt;
  callback_list.right_cmd_evt = _cbck_list.right_cmd_evt;
  callback_list.up_cmd_evt = _cbck_list.up_cmd_evt;
  callback_list.down_cmd_evt = _cbck_list.down_cmd_evt;
  callback_list.btn_cmd_evt = _cbck_list.btn_cmd_evt;

  return EOK;
}

result_t joystick_getXAxis(unsigned int * const _pX_value, unsigned int * const _low, unsigned int * const _high)
{
  result_t result = EOK;
  uint8_t low = 0U;
  uint8_t high = 0U;

  if ( NULL != _pX_value )
  {
    // ATMEGA datasheet - b01 ( REFS1, REFS0) = AVCC with external capacitor at AREF pin
    BIT_SET(ADMUX,REFS0);  // when REFS0 = 1 
    BIT_CLEAR(ADMUX,REFS1); // REFS1 = 0 - AVcc with external capacitor 
    BIT_CLEAR(ADMUX,ADLAR); // set to 0 for right adjusted result
    BIT_CLEAR(ADMUX,MUX3); // b0000 for ADC0
    BIT_CLEAR(ADMUX,MUX2); // b0
    BIT_CLEAR(ADMUX,MUX1); // b0
    BIT_CLEAR(ADMUX,MUX0); // b0
    BIT_SET(ADCSRA,ADSC); // start the conversion
    // ATMEGA328 datasheet: ADSC will read as one as long as a conversion is in progress. When the conversion is complete, it returns to zero.
    while (BIT_IS_SET(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes
    // we have to read ADCL first; doing so locks both ADCL
    // and ADCH until ADCH is read.  reading ADCL second would
    // cause the results of each conversion to be discarded,
    // as ADCL and ADCH would be locked when it completed.
    //  ATMEGA328 datasheet: When an ADC conversion is complete, the result is found in these two registers.
    low  = ADCL; // carries ADC0 ( LSB ) and ADC7 when ADLAR = 0 // right adjusted result
    high = ADCH; // carries ADC9 ( MSB ) and ADC8 when ADLAR = 0 // right adjusted result
    *_pX_value = (low>>2) + (high<<6);
    *_low = low;
    *_high = high;
  }
  else
  {
    result = ENOK;
  }

  return result;
}

result_t joystick_getYAxis(unsigned int * const _pY_value, unsigned int * const _low, unsigned int * const _high)
{
  result_t result = EOK;
  uint8_t low = 0U;
  uint8_t high = 0U;

  if ( NULL != _pY_value )
  {
    // ATMEGA datasheet - b01 ( REFS1, REFS0) = AVCC with external capacitor at AREF pin
    BIT_SET(ADMUX,REFS0);   // REFS0 = 1 
    BIT_CLEAR(ADMUX,REFS1); // REFS1 = 0 - AVcc with external capacitor 
    BIT_CLEAR(ADMUX,ADLAR); // set to 0 for right adjusted result
    BIT_CLEAR(ADMUX,MUX3);   // b0001 for ADC1
    BIT_CLEAR(ADMUX,MUX2);  // b0001 for ADC1
    BIT_CLEAR(ADMUX,MUX1);  // b0001 for ADC1
    BIT_SET(ADMUX,MUX0);     // b0001 for ADC1
    BIT_SET(ADCSRA,ADSC);  // start the conversion
    while (BIT_IS_SET(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes
    // we have to read ADCL first; doing so locks both ADCL
    // and ADCH until ADCH is read.  reading ADCL second would
    // cause the results of each conversion to be discarded,
    // as ADCL and ADCH would be locked when it completed.
    //  ATMEGA328 datasheet: When an ADC conversion is complete, the result is found in these two registers.
    low  = ADCL; // carries ADC0 ( LSB ) and ADC7 when ADLAR = 0 // right adjusted result
    high = ADCH; // carries ADC9 ( MSB ) and ADC8 when ADLAR = 0 // right adjusted result
    *_pY_value = (low >> 2) + (high << 6);
    *_low = low;
    *_high = high;
  }
  else
  {
    result = ENOK;
  }

  return result;
}

result_t joystick_getHorn()
{
  static bool button_down_before = false;
  bool isButtonDown = false;
  isButtonDown = !BIT_IS_SET(PINB, PB4);
  if (button_down_before != isButtonDown)
  {
    // btn command sent
    button_down_before = isButtonDown;
    callback_list.btn_cmd_evt(isButtonDown);
  }
  
 return EOK;
}

result_t joystick_Task10ms()
{
  result_t result = EOK;
  unsigned int low, high = 0;
  unsigned int xRefMax = 0U;
  unsigned int xRefMin = 0U;
  unsigned int yRefMax = 0U;
  unsigned int yRefMin = 0U;
  static unsigned int old_xAxis = 0U;
  static unsigned int old_yAxis = 0U;
  
  result = joystick_getXAxis(&xPosition, &low, &high);
  if (EOK == result)
  {
    result = joystick_getYAxis(&yPosition, &low, &high);
  }

  // check if this is not just a flickering by using a threshold
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
    xRefMax = old_xAxis + JOYSTICK_THRESHOLD;
    xRefMin = old_xAxis - JOYSTICK_THRESHOLD;
  }

  if (xPosition < xRefMin)
  {
    // left move command detected
    callback_list.left_cmd_evt(xPosition, yPosition, old_xAxis - xPosition);

    old_xAxis = xPosition;
  }
  else if ( xPosition > xRefMax )
  {
    // right move command detected
    callback_list.right_cmd_evt(xPosition, yPosition, xPosition - old_xAxis);
    old_xAxis = xPosition;
  }
  else if (old_xAxis != xPosition)
  {
    old_xAxis = xPosition; // small fluctuation - might not be a trigger
  }

  // check if this is not just a flickering by using a threshold
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

  if (yPosition > yRefMax )
  {
    // down move command detected
    callback_list.down_cmd_evt(xPosition, yPosition, yPosition - old_yAxis);

    old_yAxis = yPosition; 
  }
  else if (yPosition < yRefMin)
  {
    // up move command detected
    callback_list.up_cmd_evt(xPosition, yPosition, old_yAxis - yPosition);

    old_yAxis = yPosition; 
  }
  else if (old_xAxis != xPosition)
  {
    old_xAxis = xPosition; // small fluctuation - might not be a trigger
  }

  if (EOK == result)
  {
    result = joystick_getHorn();
  }

  return result;
}

void dump(void)
{
  xPosition = 0;
  yPosition = 0;
  buttonPressed = 0;
}
