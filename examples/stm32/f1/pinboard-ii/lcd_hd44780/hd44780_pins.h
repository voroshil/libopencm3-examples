#ifndef _H_HD44780_PINS_H_
#define _H_HD44780_PINS_H_

#define LCD_BUS_4BIT
//#define LCD_BUS_8BIT

#define RCC_LCD_CTRL  RCC_GPIOB
#define LCD_CTRL_BANK GPIOB
// All control pins must be on the same port !
#define LCD_E_PIN     GPIO7
#define LCD_RW_PIN    GPIO6
#define LCD_RS_PIN    GPIO5

#define RCC_LCD_DATA  RCC_GPIOB
#define LCD_DATA_BANK GPIOB
// All data pins must be on the same port !
#define LCD_D0_PIN    GPIO8
#define LCD_D1_PIN    GPIO9
#define LCD_D2_PIN    GPIO10
#define LCD_D3_PIN    GPIO11
#define LCD_D4_PIN    GPIO12
#define LCD_D5_PIN    GPIO13
#define LCD_D6_PIN    GPIO14
#define LCD_D7_PIN    GPIO15

#endif//_H_HD44780_PINS_H
