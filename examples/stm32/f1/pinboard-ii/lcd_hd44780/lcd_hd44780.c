#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "hd44780.h"

static void clock_setup(void)
{
  rcc_clock_setup_in_hse_12mhz_out_72mhz();

  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_reset_pulse(RST_TIM2);

  timer_reset(TIM2);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM2, (rcc_ahb_frequency / 1000000)-1); // 1MHz
  timer_disable_preload(TIM2);
  timer_one_shot_mode(TIM2);
}

void _delay_us(uint16_t us){
  TIM_ARR(TIM2) = us; // Period = us
  TIM_CNT(TIM2) = 0;
  TIM_CR1(TIM2) |= TIM_CR1_CEN;
  while (TIM_CR1(TIM2) & TIM_CR1_CEN);
}

static void lcd_init(void){
  hd44780_init();
  hd44780_onoff(1,0,0); // Screen on, cursor off, no blinking

  hd44780_command(LCD_CMD_RESET);
  hd44780_command(LCD_CMD_AUTO_MOVE(1, 0));
}

int main(void){
  clock_setup();

  lcd_init();
  
  hd44780_str("Hitachi");
  hd44780_goto(1, 1);
  hd44780_str("micro");

  while(1){
  }
  return 0;
}