#ifndef _H_HD44780_H_
#define _H_HD44780_H_

#define LCD_CMD_CLEAR_SCREEN           (0x01)
#define LCD_CMD_RESET                  (0x02)
#define LCD_CMD_AUTO_MOVE(inc, scr)    (0x04 | ((inc)?2:0) | ((scr)?1:0))
#define LCD_CMD_ON(on, dash, blink)    (0x08 | ((on)?4:0) | ((dash)?2:0) | ((blink)?:10))
#define LCD_CMD_MOVE(scr, right)       (0x10 | ((scr)?8:0) | ((right)?:4:0))
#define LCD_CMD_MODE(bit8, line2, h10) (0x20 | ((bit8)?0x10:0) | ((line2)?8:0) | ((h10)?4:0))
#define LCD_CMD_USE_SGRAM(addr)        (0x40 | ((addr) & 0x3f))
#define LCD_CMD_USE_DDRAM(addr)        (0x80 | ((addr) & 0x7f))

// Must be implemented somewhere
void _delay_us(uint16_t us);

void hd44780_init(void);
void hd44780_command(uint8_t data);
void hd44780_char(uint8_t data);
void hd44780_onoff(uint8_t screen_on, uint8_t cursor_dash, uint8_t cursor_blink);
void hd44780_goto(uint8_t x, uint8_t y);
void hd44780_clear(void);
void hd44780_wait(void);
#endif//_H_HD44780_H
