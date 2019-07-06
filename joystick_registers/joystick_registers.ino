
const int xPin = A0;
const int yPin = A1;
const int buttonPin = 2;
//const int buzzerPin = 8;
//const int redColPin = 9;
//const int greenColPin = 10;
//const int blueColPin = 11;

static volatile int xPosition = 0;
static volatile int yPosition = 0;
static volatile int buttonState = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
//  pinMode(buzzerPin, OUTPUT); // PD
  DDRB |= (1<<DDB0); // set the PB1 port direction to output
  PORTB &= ~(1<<PB0); // set the default value of the PB0 output to 0
//  pinMode(redColPin, OUTPUT);
  DDRB |= (1<<DDB1); // set the PB1 port direction to output
  PORTB &= ~(1<<PB1); // set the default value of the PB1 output to 0
//  pinMode(greenColPin, OUTPUT);
  DDRB |= (1<<DDB2); // set the PB2 port direction to output
  PORTB &= ~(1<<PB2); // set the default value of the PB2 output to 0
//  pinMode(blueColPin, OUTPUT); // Blue wire is connected to pin11 - connected to PB3
  DDRB |= (1<<DDB3); // set the PB3 port direction to output
  PORTB &= ~(1<<PB3); // set the default value of the PB3 output to 0
  Serial.print("PORTB = ");
  Serial.println(PORTB);

  //activate pull-up resistor on the push-button pin
//  pinMode(buttonPin, INPUT_PULLUP); 
  DDRD &= (1<<DDD2); // set the PD2 port direction to input
  PORTD |= (1<<PD2); // pull up resistor is activated for the button
}

void loop() {
  xPosition = analogRead(xPin);
  yPosition = analogRead(yPin);

  buttonState = (PIND &  _BV(PD2)) >> PD2;
  
//  Serial.print("X: ");
//  Serial.print(xPosition);
//  Serial.print(" | Y: ");
//  Serial.print(yPosition);
//  Serial.print(" | Button: ");
//  Serial.println(buttonState);

  if ( 0 == buttonState )
  {
    // beep
    PORTB =  _BV(PB0) | PORTB; // set pin to 1
  }
  else
  {
    PORTB = ~_BV(PB0) & PORTB; // set pin to 0
  }

  if ( 0 == xPosition )
  {
    // beep
    PORTB = _BV(PB2) | PORTB; // set pin to 1
    Serial.print("0 == xPosition PORTB = ");
    Serial.println(PORTB);
  }
  else if ( 0 == yPosition )
  {
    PORTB = _BV(PB3) | PORTB; // set pin to 1
    Serial.print("0 == yPosition PORTB = ");
    Serial.println(PORTB);
  } 
  else if (1023 <= yPosition)
  {
    PORTB = _BV(PB1) | PORTB; // set pin to 1
    Serial.print("xPosition 1023 PORTB = ");
    Serial.println(PORTB);
  }
  else if (1023 <= xPosition)
  {
    PORTB = _BV(PB1) | PORTB; // set pin to 1
    Serial.print("xPosition 1023 PORTB = ");
    Serial.println(PORTB);
  }
  else
  {
    PORTB = ~_BV(PB1) & PORTB; // set pin to 0
    PORTB = ~_BV(PB2) & PORTB; // set pin to 0
    PORTB = ~_BV(PB3) & PORTB; // set pin to 0 
    Serial.print("PORTB = ");
    Serial.println(PORTB); 
  }
  
  delay(500); // add some delay between reads
}
