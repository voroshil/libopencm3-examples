#ifndef _H_LIBMCU_LCD_LK_H_
#define _H_LIBMCU_LCD_LK_H_

#include <stdint.h>

#define DISC0 0
#define DISC1 1
#define DISC2 2
#define DISC3 3
#define DISC4 4
#define DISC5 5
#define DVD   6
#define DDD   7
#define MP3   8
#define MP4   9
#define VCD   10
#define PLAY  11
#define PAUSE 12
#define DOTS1 13
#define DOTS2 14
#define DTS   15
#define B1    16
#define C1    17

typedef uint32_t (*lk_receive_t)(void);
typedef void (*lk_send_t)(uint8_t data);
typedef void (*lk_ss_t)(uint8_t ss);
typedef void (*lk_delay_t)(void);

void lk_init(lk_receive_t receive_cb, lk_send_t send_cb, lk_ss_t ss_cb);

void lk_test(lk_delay_t delay_cb);
void lk_test2(lk_delay_t delay_cb);
void lk_update(void);
void lk_led_on(uint8_t led);
void lk_led_off(uint8_t led);
void lk_led_all(uint8_t on);
void lk_set_char(uint8_t addr, char chr);
void lk_set_seg(uint8_t addr, uint8_t num);
void lk_apply_font(uint8_t glyph, uint8_t addr);
void lk_clear(void);
uint8_t lk_get_buttons(void);

#endif // _H_LIBMCU_LCD_LK_H_