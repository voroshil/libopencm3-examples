#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "lcd_ledkey.h"

const uint8_t[11] str = {'0','1','2','3','4','5','6','7','8','9',' '};


static uint32_t lk_receive_cb(void){
  uint32_t temp = 0;

  // Pull-up on
  gpio_set_mode(GPIO_BANK_SPI2_MOSI, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_SPI2_MOSI);
  gpio_set(GPIO_BANK_SPI2_MOSI, GPIO_SPI2_MOSI);

  for (int i = 0; i < 32; i++) {
    temp >>= 1;

    gpio_clear(GPIO_BANK_SPI2_SCK, GPIO_SPI2_SCK);

    if (gpio_get(GPIO_BANK_SPI2_MOSI, GPIO_SPI2_MOSI)){
      temp |= 0x80000000;
    }

    gpio_set(GPIO_BANK_SPI2_SCK, GPIO_SPI2_SCK);
  }

  // Pull-up off
  gpio_set_mode(GPIO_BANK_SPI2_MOSI, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_MOSI);
  gpio_clear(GPIO_BANK_SPI2_MOSI, GPIO_SPI2_MOSI);
  return temp;
}
static void lk_send_cb(uint8_t data){
  uint8_t i;
  for(i=0; i<8; i++){
    gpio_clear(GPIO_BANK_SPI2_SCK, GPIO_SPI2_SCK);
    if (data & 1){
      gpio_set(GPIO_BANK_SPI2_MOSI, GPIO_SPI2_MOSI);
    }else{
      gpio_clear(GPIO_BANK_SPI2_MOSI, GPIO_SPI2_MOSI);
    }
    gpio_set(GPIO_BANK_SPI2_SCK, GPIO_SPI2_SCK);
    data >>= 1;
  }
}
static void lk_ss_cb(uint8_t ss){
  if (ss){
    gpio_set(GPIO_BANK_SPI2_NSS, GPIO_SPI2_NSS);
  }else{
    gpio_clear(GPIO_BANK_SPI2_NSS, GPIO_SPI2_NSS);
  }
}
void lk_delay_cb(void){
}


void update_lcd_state(){
  for(i=0; i<8; i++){
    uint8_t pos = (i + state) % 11;
    lk_set_char(i,str[pos]);
    if (i>=1 && i <=4){
      lk_led_on(pos);
    }else{
      lk_led_off(pos);
    }
  }
  lk_update();
}
int main(){
  uint8_t i;

  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  rcc_set_usbpre(RCC_CFGR_USBPRE_PLL_CLK_DIV1_5);

  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

  gpio_set_mode(GPIO_BANK_SPI2_NSS, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_NSS);
  gpio_set(GPIO_BANK_SPI2_NSS, GPIO_SPI2_NSS);

  gpio_set_mode(GPIO_BANK_SPI2_SCK, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_SCK);
  gpio_set_mode(GPIO_BANK_SPI2_MOSI, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO_SPI2_MOSI);
  
  gpio_set(GPIO_BANK_SPI2_SCK, GPIO_SPI2_SCK);
  gpio_set(GPIO_BANK_SPI2_MOSI, GPIO_SPI2_MOSI);

  lk_init(lk_receive_cb, lk_send_cb, lk_ss_cb);

  while(1){
    update_lcd_state();
    for (i = 0; i < 8000000; i++)	/* Wait a bit. */
      __asm__("nop");
  }
  return 0;
}