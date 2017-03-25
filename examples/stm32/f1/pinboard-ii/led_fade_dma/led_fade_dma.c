#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>

static const uint16_t pwm_values[512] = {
// From 0x00 to 0x7E and backward
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
    0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
    0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E,
    0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E,
    0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E,
    0x7E, 0x7C, 0x7A, 0x78, 0x76, 0x74, 0x72, 0x70,
    0x6E, 0x6C, 0x6A, 0x68, 0x66, 0x64, 0x62, 0x60,
    0x5E, 0x5C, 0x5A, 0x58, 0x56, 0x54, 0x52, 0x50,
    0x4E, 0x4C, 0x4A, 0x48, 0x46, 0x44, 0x42, 0x40,
    0x3E, 0x3C, 0x3A, 0x38, 0x36, 0x34, 0x32, 0x30,
    0x2E, 0x2C, 0x2A, 0x28, 0x26, 0x24, 0x22, 0x20,
    0x1E, 0x1C, 0x1A, 0x18, 0x16, 0x14, 0x12, 0x10,
    0x0E, 0x0C, 0x0A, 0x08, 0x06, 0x04, 0x02, 0x00,
// Same as previous part
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
    0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
    0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E,
    0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E,
    0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E,
    0x7E, 0x7C, 0x7A, 0x78, 0x76, 0x74, 0x72, 0x70,
    0x6E, 0x6C, 0x6A, 0x68, 0x66, 0x64, 0x62, 0x60,
    0x5E, 0x5C, 0x5A, 0x58, 0x56, 0x54, 0x52, 0x50,
    0x4E, 0x4C, 0x4A, 0x48, 0x46, 0x44, 0x42, 0x40,
    0x3E, 0x3C, 0x3A, 0x38, 0x36, 0x34, 0x32, 0x30,
    0x2E, 0x2C, 0x2A, 0x28, 0x26, 0x24, 0x22, 0x20,
    0x1E, 0x1C, 0x1A, 0x18, 0x16, 0x14, 0x12, 0x10,
    0x0E, 0x0C, 0x0A, 0x08, 0x06, 0x04, 0x02, 0x00,
};

static void clock_setup(void){
  /* Set STM32 to 72 MHz. PinBoard II board has 12MHz quartz. */
  rcc_clock_setup_in_hse_12mhz_out_72mhz();

  /* Enable GPIOA clock. */
  rcc_periph_clock_enable(RCC_GPIOA);

  /* Enable TIM2 clock for PWM output. */
  rcc_periph_clock_enable(RCC_TIM2);

  /* Enable DMA1 clock */
  rcc_periph_clock_enable(RCC_DMA1);
}

static void gpio_setup(void){
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO1);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO2);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO3);
}

static void pwm_timer_setup(void){
  timer_reset(TIM2);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  timer_set_period(TIM2, 128); // ~ 250kHz PWM output in center-aligned mode
  timer_enable_break_main_output(TIM2);

  timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM2, TIM_OC2);
  timer_set_oc_value(TIM2, TIM_OC2, 0);

  timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM2, TIM_OC3);
  timer_set_oc_value(TIM2, TIM_OC3, 0);

  timer_set_oc_mode(TIM2, TIM_OC4, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM2, TIM_OC4);
  timer_set_oc_value(TIM2, TIM_OC4, 0);

  timer_enable_counter(TIM2);
}

static void dma_setup(void){
  dma_channel_reset(DMA1, DMA_CHANNEL2);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL2, (uint32_t)&(TIM2_CCR2));
  dma_set_memory_address(DMA1, DMA_CHANNEL2, (uint32_t)pwm_values); // LED Off
  dma_set_read_from_memory(DMA1, DMA_CHANNEL2);
  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL2);
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL2);
  dma_set_number_of_data(DMA1, DMA_CHANNEL2, 256);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL2, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL2, DMA_CCR_MSIZE_16BIT);
  dma_enable_circular_mode(DMA1, DMA_CHANNEL2);
  dma_enable_channel(DMA1, DMA_CHANNEL2);

  dma_channel_reset(DMA1, DMA_CHANNEL3);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL3, (uint32_t)&(TIM2_CCR3));
  dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)(pwm_values+64)); // LED half-power
  dma_set_read_from_memory(DMA1, DMA_CHANNEL3);
  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL3);
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL3);
  dma_set_number_of_data(DMA1, DMA_CHANNEL3, 256);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL3, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL3, DMA_CCR_MSIZE_16BIT);
  dma_enable_circular_mode(DMA1, DMA_CHANNEL3);
  dma_enable_channel(DMA1, DMA_CHANNEL3);

  dma_channel_reset(DMA1, DMA_CHANNEL6);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL6, (uint32_t)&(TIM2_CCR4));
  dma_set_memory_address(DMA1, DMA_CHANNEL6, (uint32_t)(pwm_values+128)); // LED on
  dma_set_read_from_memory(DMA1, DMA_CHANNEL6);
  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL6);
  dma_enable_memory_increment_mode(DMA1, DMA_CHANNEL6);
  dma_set_number_of_data(DMA1, DMA_CHANNEL6, 256);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL6, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL6, DMA_CCR_MSIZE_16BIT);
  dma_enable_circular_mode(DMA1, DMA_CHANNEL6);
  dma_enable_channel(DMA1, DMA_CHANNEL6);
}

static void fade_timer_setup(void){
  timer_reset(TIM3);
  timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM2, 624); // 115.2kHz
  timer_set_period(TIM3, 450); //  256Hz in edge-aligned mode

  timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1); // DMA channel 6
  timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1); // DMA channel 2
  timer_set_oc_mode(TIM3, TIM_OC4, TIM_OCM_PWM1); // DMA channel 3

  // Use different OC value to avoid DMA request conflicts
  timer_set_oc_value(TIM3, TIM_OC1, 100);
  timer_set_oc_value(TIM3, TIM_OC2, 200);
  timer_set_oc_value(TIM3, TIM_OC3, 300);

  timer_enable_irq(TIM3, TIM_DIER_CC1DE); // enable DMA channel 6 request
  timer_enable_irq(TIM3, TIM_DIER_CC3DE); // enable DMA channel 2 request
  timer_enable_irq(TIM3, TIM_DIER_CC4DE); // enable DMA channel 3 request

  timer_enable_counter(TIM3);
}

int main(void){
  clock_setup();
  gpio_setup();
  pwm_timer_setup();
  dma_setup();
  fade_timer_setup();

  while(1){}

  return 0;
}