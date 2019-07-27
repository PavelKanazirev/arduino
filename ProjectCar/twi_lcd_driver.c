// filename twi_lcd_driver.c
// project ProjectCar
// author Pavel

#include "Arduino.h"
#include <avr/io.h>
#include <compat/twi.h>
#include <avr/delay.h>

#include "CarTimer.h"
#include "twi_lcd_driver.h"

// Specification https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

#define F_CPU 16000000L

#define I2C_read 1
#define I2C_write 0
// ATMega328 datasheet: After a START condition has been transmitted, the TWINT Flag is set by hardware
#define wait while(!(TWCR & (1 << TWINT)))
#define ADRS 0x27

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

uint8_t _Addr=ADRS;
uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _numlines;
uint8_t _cols;
uint8_t _rows;
uint8_t _backlightval=LCD_NOBACKLIGHT;

/*********** high level commands, for screen manipulation */
void clear();
void home();
void setCursor(uint8_t col, uint8_t row);

// Turn the display on/off (quickly)
void noDisplay();
void display();

// Turns the underline cursor on/off
void noCursor();
void cursor();

// Turn on and off the blinking cursor
void noBlink();
void blink();

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void);
void scrollDisplayRight(void);

// This is for text that flows Left to Right
void leftToRight(void);
// This is for text that flows Right to Left
void rightToLeft(void);
// This will 'right justify' text from the cursor
void autoscroll(void);
// This will 'left justify' text from the cursor
void noAutoscroll(void);
// Turn the (optional) backlight off/on
void noBacklight(void);
/*********** mid level commands, for sending data/cmds */
inline void command(uint8_t value);

/************ low level data pushing commands **********/
// write either command or data
void send(uint8_t value, uint8_t mode);
void write4bits(uint8_t value);
void expanderWrite(uint8_t _data);
void pulseEnable(uint8_t _data);

// TWI protocol related routines
void TWI_init();
void TWI_start();
void TWI_write_adrs(uint8_t adrs);
void TWI_write_data(uint8_t data);
void TWI_stop();

void TWI_init() {
  
  // (Fc = 16 000 000 )/[16+2 * (TWBR = 72)*(PrescalerValue = 1)] = 100 000
  // initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1
  TWSR = 0x00; // no prescaler
  TWBR = TWI_100KHZ_PRECALCULATED_BIT_RATE_VALUE;   // set SCL to 100 KHz as the Standard is
}

//  ATMega328 datasheet: page 226
void TWI_start() {
  // ATMega328 datasheet: The first step in a TWI transmission is to transmit a START condition. This is done by
  // writing a specific value into TWCR, instructing the TWI hardware to transmit a START condition.
  // it is important that the TWINT bit is set in the value written
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
  // Wait for TWINT Flag set. This indicates that the START condition has been transmitted
  wait;

  volatile uint8_t status= TWSR & 0xF8;
  // value specified at https://www.microchip.com/webdoc/AVRLibcReferenceManual/ch20s33s01.html
  // Check value of TWI Status Register. Mask prescaler bits. If status different from START go to ERROR
  while (status != TW_START)
  {
      TRACE_ERROR("Stuck in START loop");
  }
  
  TRACE_DEBUG("START Acknowledged");
  cartimer_delayMicroseconds(10);
}

// HD44780 datasheet: Bus Timing Characteristics used for the delays page 49
// HD44780 datasheet: page 25
// RS R/W DB7   DB6 DB5 DB4 DB3 DB2 DB1 DB0
// 1  0   Write data
// ATMega328 datasheet: Figure 19-10. Interfacing the Application to the TWI in a Typical Transmission
void TWI_write_adrs(uint8_t adrs) {
  wait;
  // ATMega328 datasheet: page 226 Load SLA_W into TWDR Register. Clear TWINT bit in TWCR to start transmission of address
  TWDR = adrs<<1 + I2C_write;
  TWCR = (1 << TWINT) | (1 << TWEN);
  // Wait for TWINT Flag set. This indicates that the SLA+W has been transmitted, and ACK/NACK has been received.
  wait;
  
  //uint8_t status = TWSR & 0b1111100;
  // Check value of TWI Status Register. Mask prescaler bits. If status different from MT_SLA_ACK go to ERROR
  // ATMega328 datasheet: Table 19-2. Status codes for Master Transmitter Mode
  volatile uint8_t status= TWSR & 0xF8;
  while (status != TW_MT_SLA_ACK) 
  {
      TRACE_ERROR("Stuck in SLAV+R/W loop");
  }

  TRACE_DEBUG("SLAVE Acknowledged");
  cartimer_delayMicroseconds(10);
}


// HD44780 datasheet: Bus Timing Characteristics used for the delays page 49
// HD44780 datasheet: page 25
// RS R/W DB7   DB6 DB5 DB4 DB3 DB2 DB1 DB0
// 1  0   Write data
// ATMega328 datasheet: Figure 19-10. Interfacing the Application to the TWI in a Typical Transmission
void TWI_write_data(uint8_t data) {
  wait;
  // ATMega328 datasheet: page 226 Load DATA into TWDR Register. 
  TWDR = data;
  // Clear TWINT bit in TWCR to start transmission of data
  TWCR = (1 << TWINT) | (1 << TWEN);
  // Wait for TWINT Flag set. This indicates that the DATA has been transmitted, and ACK/NACK has been received.
  wait;

  // ATMega328 datasheet: page 226 Check value of TWI Status Register. Mask prescaler bits. If status different from MT_DATA_ACK go to ERROR
  // ATMega328 datasheet: Table 19-2. Status codes for Master Transmitter Mode
  // https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__util__twi_1ga4c28186053b5298305b131ad3e1111f7.html
  // #define TW_MT_DATA_ACK 0x28
  volatile uint8_t status = TWSR & 0xF8; // 0x28 ?
  while(status != TW_MT_DATA_ACK)
  {
      //  TRACE_DEBUG("Stuck in WRITE DATA loop");
  }
  
  // TRACE_DEBUG("DATA SENT");
  cartimer_delayMicroseconds(10); // commands need > 37us to settle
}


void TWI_stop() {
  // ATMega328 datasheet: page 226  Transmit STOP condition
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  while(!(TWCR & (1<<TWSTO)));  // Wait till stop condition is transmitted

//  TRACE_DEBUG("STOP");
  cartimer_delayMicroseconds(10);
}

//----------------end I2C communication-----------------------------------//


/********** high level commands, for the user! */
void clear(){
  command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
  cartimer_delayMicroseconds(2000);  // this command takes a long time!
}

void home(){
  command(LCD_RETURNHOME);  // set cursor position to zero
  cartimer_delayMicroseconds(2000);  // this command takes a long time!
}

void setCursor(uint8_t col, uint8_t row){
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Turn the (optional) backlight off/on
void noBacklight(void) {
  _backlightval=LCD_NOBACKLIGHT;
  expanderWrite(0);
}

void backlight(void) {
  _backlightval=LCD_BACKLIGHT;
  expanderWrite(0);
}

/*********** mid level commands, for sending data/cmds */

inline void command(uint8_t value) {
  send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
void send(uint8_t value, uint8_t mode) {
  uint8_t highnib=value&0xf0;
  uint8_t lownib=(value<<4)&0xf0;
  write4bits((highnib)|mode);
  write4bits((lownib)|mode); 
}

void write4bits(uint8_t value) {
  expanderWrite(value);
  pulseEnable(value);
}

void expanderWrite(uint8_t _data){
  TWI_start();
  TWI_write_adrs(_Addr);
  TWI_write_data((int)(_data) | _backlightval);
  TWI_stop();
}

void pulseEnable(uint8_t _data){
  expanderWrite(_data | En);  // En high
  cartimer_delayMicroseconds(1);   // enable pulse must be >450ns
  
  expanderWrite(_data & ~En); // En low
  cartimer_delayMicroseconds(50);    // commands need > 37us to settle
}

void initLCD() {
  TWI_init();
  
  _displayfunction=LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x10DOTS;
  _displaycontrol=LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  _displaymode=LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  _numlines=2;
  _cols=16;
  _rows=2;
  
  // HD44780U datasheet: PAGE 45/46 Figures 23
  // Wait for more than 40 ms after VCC rises to 2.7 V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 41
  cartimer_delayMicroseconds(41000);
  
  // Now we pull both RS and R/W low to begin commands
  expanderWrite(_backlightval); // reset expander and turn backlight off (Bit 8 =1)
  for ( unsigned i = 0; i < 20; i++ )
  {
      cartimer_delayMicroseconds(50000);
  }

    //put the LCD into 4 bit mode
  // this is according to the hitachi HD44780 datasheet
  // figure 24, pg 46
  
    // we start in 8bit mode, try to set 4 bit mode
   write4bits(0x03 << 4);
   cartimer_delayMicroseconds(4100); // wait min 4.1ms
   
   // second try
   write4bits(0x03 << 4);
   cartimer_delayMicroseconds(4100); // wait min 4.1ms
   
   // third go!
   write4bits(0x03 << 4); 
   cartimer_delayMicroseconds(150);
   
   // finally, set to 4-bit interface
   write4bits(0x02 << 4); 


  // set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  
  
  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();
  
  // clear it off
  clear();
  
  // Initialize to default text direction (for roman languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
  
  home();

  backlight();
  display_StaticData();
}

result_t display_StaticData()
{
    result_t result = EOK;
    
    setCursor(0,0);
    char c1[] = "State";
    for(int i = 0; c1[i] != '\0'; ++i)
    {
        send(c1[i], Rs);
    }
    
    setCursor(6,0);          
    char c3[] = "LW G:";
    for(int i = 0; c3[i] != '\0'; ++i)
    {
        send(c3[i], Rs);
    }

    setCursor(13,0);          
    char c4[] = "D:";
    for(int i = 0; c4[i] != '\0'; ++i)
    {
        send(c4[i], Rs);
    }

    setCursor(6,1);          
    char c5[] = "RW G:";
    for(int i = 0; c5[i] != '\0'; ++i)
    {
        send(c5[i], Rs);
    }

    setCursor(13,1);
    char c6[] = "D:";
    for(int i = 0; c6[i] != '\0'; ++i)
    {
        send(c6[i], Rs);
    }
}

result_t display_Task10ms( unsigned char _current_risk, 
            unsigned char _current_direction, 
            unsigned char _current_left_wheel_gear, 
            unsigned char _current_right_wheel_gear )
{
    result_t result = EOK;

    setCursor(11,0);
    send(_current_left_wheel_gear, Rs);
    setCursor(15,0);
    send(_current_direction, Rs);

    setCursor(0,1);
    send(_current_risk, Rs);

    setCursor(11, 1);
    send(_current_right_wheel_gear, Rs);
    setCursor(15,1);
    send(_current_direction, Rs);

    return result;
}
