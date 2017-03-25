#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

void gpio_setup(void){
}
static void clock_setup(void)
{
  rcc_clock_setup_in_hse_12mhz_out_72mhz();
}

void lcd_init(void){
  hd44780_init();
  hd44780_onoff(1,0,0); // Screen on, cursor off, no blinking
}

void main(void){
  clock_setup();
  gpio_setup();

  lcd_init();
  
  while(1){
  }
}