// filename twi_lcd_driver.h
// project ProjectCar
// author Pavel

#ifndef TWI_LCD_DRIVER_H
#define TWI_LCD_DRIVER_H

#include "CommonTypes.h"

#include "twi_lcd_driver.h"

#define TWI_LCD_RISK_SELF_DIAG 'D'
#define TWI_LCD_RISK_NO 'N'
#define TWI_LCD_RISK_SMALL 'W'
#define TWI_LCD_RISK_BIG 'A'
#define TWI_LCD_RISK_CRITICAL 'E'

#define TWI_LCD_DIR_FW  'F'
#define TWI_LCD_DIR_BW  'B'
#define TWI_LCD_DIR_N   'N'

#define TWI_LCD_GEAR0_SPEED 'N'
#define TWI_LCD_GEAR1_SPEED '1'
#define TWI_LCD_GEAR2_SPEED '2'
#define TWI_LCD_GEAR3_SPEED '3'

void initLCD();

result_t display_StaticData();
result_t display_Task10ms( unsigned char _current_risk, 
            unsigned char _current_direction, 
            unsigned char _current_left_wheel_gear, 
            unsigned char _current_right_wheel_gear );

#endif // TWI_LCD_DRIVER_H
