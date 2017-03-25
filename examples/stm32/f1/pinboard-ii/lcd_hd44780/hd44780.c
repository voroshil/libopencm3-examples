#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <inttypes.h>

#include "hd44780.h"
#include "hd44780_pins.h"

static uint16_t _hd44780_read(void);
static void _hd44780_write(uint16_t gpios);

#if defined(LCD_BUS_4BIT) && defined(LCD_BUS_8BIT)
#error Only one of LCD_BUS_4BIT or LCD_BUS_8BIT should be defined

#elif defined(LCD_BUS_8BIT)
#define LCD_ALL_DATA_PINS  (LCD_D0_PIN | LCD_D1_PIN | LCD_D2_PIN | LCD_D3_PIN | LCD_D4_PIN | LCD_D5_PIN | LCD_D6_PIN | LCD_D7_PIN)
#define LCD_CMD_INIT(lines,height)   LCD_CMD_MODE(1, lines, height)

static void hd44780_write(uint8_t data){
  uint16_t gpios = (data & 1 ? LCD_D0_PIN:0) | 
  (data & 2 ? LCD_D1_PIN:0)|
  (data & 4 ? LCD_D2_PIN:0)|
  (data & 8 ? LCD_D3_PIN:0)|
  (data & 16 ? LCD_D4_PIN:0)|
  (data & 32 ? LCD_D4_PIN:0)|
  (data & 64 ? LCD_D4_PIN:0)|
  (data & 128 ? LCD_D5_PIN:0);

  _hd44780_write(gpios);
}

static uint8_t hd44780_read(void){
  uint16_t gpios = _hd44780_read();

  return (gpios & LCD_D0_PIN ? 1 : 0) |
         (gpios & LCD_D1_PIN ? 2 : 0) |
         (gpios & LCD_D2_PIN ? 4 : 0) |
         (gpios & LCD_D3_PIN ? 8 : 0) |
         (gpios & LCD_D4_PIN ? 16 : 0)|
         (gpios & LCD_D5_PIN ? 32 : 0)|
         (gpios & LCD_D6_PIN ? 64 : 0)|
         (gpios & LCD_D7_PIN ? 128 : 0);
}

#elif defined(LCD_BUS_4BIT)
#define LCD_ALL_DATA_PINS  (LCD_D0_PIN | LCD_D1_PIN | LCD_D2_PIN | LCD_D3_PIN)
#define LCD_CMD_INIT(lines,height)   LCD_CMD_MODE(0, lines, height)

static void hd44780_write(uint8_t data){
  uint16_t gpios = (data & 1 ? LCD_D0_PIN) | 
  (data & 2 ? LCD_D1_PIN)|
  (data & 4 ? LCD_D2_PIN)|
  (data & 8 ? LCD_D3_PIN)|
  _hd44780_write(gpios);
  gpios = (data & 16 ? LCD_D0_PIN) | 
  (data & 32 ? LCD_D1_PIN)|
  (data & 64 ? LCD_D2_PIN)|
  (data & 128 ? LCD_D3_PIN)|
  _hd44780_write(gpios);
}

static uint8_t hd44780_read(void){
  uint16_t gpios = _hd44780_read();
  uint8_t res = 0;

  res |= 
  (gpios & LCD_D0_PIN ? 1 : 0) |
         (gpios & LCD_D1_PIN ? 2 : 0) |
         (gpios & LCD_D2_PIN ? 4 : 0) |
         (gpios & LCD_D3_PIN ? 8 : 0);
  gpios = _hd44780_read();
  res |=
         (gpios & LCD_D0_PIN ? 16 : 0)|
         (gpios & LCD_D1_PIN ? 32 : 0)|
         (gpios & LCD_D2_PIN ? 64 : 0)|
         (gpios & LCD_D3_PIN ? 128 : 0);
  return res;
}

#else
#error Either LCD_BUS_4BIT or LCD_BUS_8BIT should be defined
#endif

void hd44780_wait(void){
  uint8_t data;

  gpio_clear(LCD_CTRL_BANK, LCD_RS_PIN);
  do{
    data = hd44780_read();
  }while(data & 0x80);  
}
static void _hd44780_write(uint16_t gpios){

  gpio_set_mode(LCD_DATA_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_ALL_DATA_PINS);


  gpio_set(LCD_DATA_BANK, gpios & LCD_ALL_DATA_PINS);
  gpio_clear(LCD_DATA_BANK, (~gpios) & LCD_ALL_DATA_PINS);

  gpio_set(LCD_CTRL_BANK, LCD_E_PIN);
  _delay_us(2);
  gpio_clear(LCD_CTRL_BANK, LCD_E_PIN);

  gpio_set_mode(LCD_DATA_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, LCD_ALL_DATA_PINS);
  gpio_set(LCD_DATA_BANK, LCD_ALL_DATA_PINS);
}

static uint16_t _hd44780_read(void){
  uint16_t gpios = 0;

  gpio_set_mode(LCD_DATA_BANK, GPIO_CNF_INPUT_PULL_UPDOWN,
		      GPIO_MODE_INPUT, LCD_ALL_DATA_PINS);

  gpio_set(LCD_CTRL_BANK, LCD_RW_PIN);

  gpio_set(LCD_CTRL_BANK, LCD_E_PIN);
  _delay_us(2);
  gpios = gpio_get(LCD_DATA_BANK, LCD_ALL_DATA_PINS);  
  gpio_clear(LCD_CTRL_BANK, LCD_E_PIN);

  gpio_clear(LCD_CTRL_BANK, LCD_RW_PIN);

  gpio_set_mode(LCD_DATA_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, LCD_ALL_DATA_PINS);
  gpio_set(LCD_DATA_BANK, LCD_ALL_DATA_PINS);

  return gpios;
}

void hd44780_command(uint8_t data){
  hd44780_wait();
  gpio_clear(LCD_CTRL_BANK, LCD_RS_PIN);
  hd44780_write(data);
}
void hd44780_char(uint8_t data){
  hd44780_wait();
  gpio_set(LCD_CTRL_BANK, LCD_RS_PIN);
  hd44780_write(data);
}

void hd44780_init(){
  rcc_periph_clock_enable(RCC_LCD_CTRL);
  rcc_periph_clock_enable(RCC_LCD_DATA);

  gpio_set_mode(LCD_CTRL_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_E_PIN);
  gpio_clear(LCD_CTRL_BANK, LCD_E_PIN);

  gpio_set_mode(LCD_CTRL_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_RW_PIN);
  gpio_clear(LCD_CTRL_BANK, LCD_RW_PIN);

  gpio_set_mode(LCD_CTRL_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_RS_PIN);
  gpio_set(LCD_CTRL_BANK, LCD_RS_PIN);

  gpio_set_mode(LCD_DATA_BANK, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, LCD_ALL_DATA_PINS);
  gpio_set(LCD_DATA_BANK, LCD_ALL_DATA_PINS);

  hd44780_command(LCD_CMD_INIT(1, 0)); // 2-lines, 5x8
  _delay_us(40);
  hd44780_clear();
  hd44780_command(LCD_CMD_AUTO_MOVE(1, 0));
}

void hd44780_onoff(uint8_t screen_on, uint8_t cursor_dash, uint8_t cursor_blink){
  hd44780_command(LCD_CMD_ON(screen_on, cursor_dash, cursor_blink));
}
void hd44780_goto(uint8_t x, uint8_t y){
  hd44780_command(LCD_CMD_USE_DDRAM((((y)& 1)*0x40)+((x) & 15)));
}
void hd44780_clear(){
  hd44780_command(LCD_CMD_CLEAR_SCREEN);
}
