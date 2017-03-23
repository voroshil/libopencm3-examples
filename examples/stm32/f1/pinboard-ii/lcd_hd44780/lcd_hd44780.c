#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#define LCD_E_PIN  GPIO7
#define LCD_RW_PIN  GPIO6
#define LCD_RS_PIN  GPIO5

static const uint16_t data_pins[8] = {
  GPIO8,GPIO9,GPIO10,GPIO11,GPIO12,GPIO13,GPIO14,GPIO15
}
void gpio_setup(void){
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_E_PIN);
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_RW_PIN);
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, LCD_RS_PIN);
  for (int i=0; i<8; i++){
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, data_pins[i]);
  }
}
static void clock_setup(void)
{
  rcc_clock_setup_in_hse_12mhz_out_72mhz();

  rcc_periph_clock_enable(RCC_GPIOB);
}
void lcd_init(void){

  gpio_set(GPIOB, LCD_RS_PIN);
  gpio_clear(GPIOB, LCD_E_PIN);
  gpio_clear(GPIOB, LCD_RW_PIN);

  for(int i=0; i<8; i++){
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, data_pins[i]);
    gpio_set(GPIOB data_pins[i]);
  }
}

void lcd_write_byte(uint8_t data){
  gpio_set(GPIOB, LCD_RS_PIN);

  for(int i=0; i<8; i++){
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, data_pins[i]);
    if (data & (1<<i)){
      gpio_set(GPIOB, data_pins[i]);
    }else{
      gpio_clear(GPIOB, data_pins[i]);
    }
  }
  gpio_set(GPIOB, LCD_E_PIN);
  gpio_clear(GPIOB, LCD_E_PIN);
  for(int i=0; i<8; i++){
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, data_pins[i]);
    gpio_set(GPIOB data_pins[i]);
  }
}

uint8_t lcd_read_byte(void){
  uint8_t res = 0;

  gpio_set(LCD_RS_BANK, LCD_RS_PIN);
  gpio_set(LCD_RW_BANK, LCD_RW_PIN);
  gpio_set(LCD_E_BANK, LCD_E_PIN);

  for(int i=0; i<8; i++){
    if (gpio_get(data_ports[i], data_pins[i])){
      res |= (1<<i);
    }
  }

  gpio_clear(LCD_E_BANK, LCD_E_PIN);
  gpio_clear(LCD_RW_BANK, LCD_RW_PIN);
}

void main(void){
  clock_setup();
  gpio_setup();

  while(1){
  }
}