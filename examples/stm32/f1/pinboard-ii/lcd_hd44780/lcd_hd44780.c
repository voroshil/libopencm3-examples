#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "hd44780.h"

static void clock_setup(void)
{
  rcc_clock_setup_in_hse_12mhz_out_72mhz();

  rcc_periph_clock_enable(RCC_TIM2);
  rcc_periph_reset_pulse(RST_TIM2);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM2, 71); // 1MHz
  timer_one_shot_mode(TIM2);
}

void _delay_us(uint16_t us){
  TIM_ARR(TIM2) = us; // Period = us
  TIM_CNT(TIM2) = 0;
  TIM_CR1(TIM2) |= TIM_CR1_CEN;
  while (TIM_SR(TIM2) & TIM_CR1_CEN);
}

static void lcd_init(void){
  hd44780_init();
  hd44780_onoff(1,0,0); // Screen on, cursor off, no blinking
}

int main(void){
  clock_setup();

  lcd_init();
  
  while(1){
  }
  return 0;
}