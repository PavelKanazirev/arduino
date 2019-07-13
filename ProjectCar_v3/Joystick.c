// filename Joystick.c
// project ProjectCar
// author Pavel
#include "Joystick.h"

//const int xPin = A0;
//const int yPin = A1;
const int buttonPin = 2;

static volatile unsigned int xPosition = 0;
static volatile unsigned int yPosition = 0;
static volatile bool buttonPressed = 0;

result_t joystick_init()
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
  // XXX: this will not work properly for other clock speeds, and
  // this code should use F_CPU to determine the prescale factor.
  BIT_SET(ADCSRA, ADPS2);
  BIT_SET(ADCSRA, ADPS1);
  BIT_SET(ADCSRA, ADPS0);

  return EOK;
}

result_t joystick_getXAxis(unsigned int * const _pX_value, unsigned int * const _low, unsigned int * const _high)
{
  result_t result = EOK;
  uint8_t low = 0U;
  uint8_t high = 0U;
  uint16_t sumed10bit = 0U;

  if ( NULL != _pX_value )
  {  
    BIT_SET(ADMUX,REFS0);  // when REFS0 = 1 
    BIT_CLEAR(ADMUX,REFS1); // REFS1 = 0 - AVcc with external capacitor 
    BIT_CLEAR(ADMUX,ADLAR); // set to 0 for right adjusted result
    BIT_CLEAR(ADMUX,MUX3); // b0000 for ADC0
    BIT_CLEAR(ADMUX,MUX2); // b0
    BIT_CLEAR(ADMUX,MUX1); // b0
    BIT_CLEAR(ADMUX,MUX0); // b0
    BIT_SET(ADCSRA,ADSC); // start the conversion
    while (BIT_IS_SET(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes
    // we have to read ADCL first; doing so locks both ADCL
    // and ADCH until ADCH is read.  reading ADCL second would
    // cause the results of each conversion to be discarded,
    // as ADCL and ADCH would be locked when it completed.
    low  = ADCL;
    high = ADCH;
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
  uint16_t sumed10bit = 0U;

  if ( NULL != _pY_value )
  {  
//    ADMUX = 0;
    BIT_SET(ADMUX,REFS0);  // when REFS0 = 1 
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
    low  = ADCL;
    high = ADCH;
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

result_t joystick_getHorn(bool * const _pButton_down)
{
  // *_pButton_down = (PINB & _BV(PB4)) >> PB4;
  *_pButton_down = !BIT_IS_SET(PINB, PB4);
  
 return EOK;
}
