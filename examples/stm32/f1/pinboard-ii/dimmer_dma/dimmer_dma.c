#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>

static volatile uint16_t adc_value=1;

static void clock_setup(void){
  rcc_clock_setup_in_hse_12mhz_out_72mhz();

  rcc_periph_clock_enable(RCC_GPIOA);

  /* Enable TIM2 clock for PWM output. */
  rcc_periph_clock_enable(RCC_TIM2);

  /* Enable TIM1 clock for ADC start. */
  rcc_periph_clock_enable(RCC_TIM1);

  /* Enable DMA1 clock */
  rcc_periph_clock_enable(RCC_DMA1);

  /* Enable ADC clock */
  rcc_periph_clock_enable(RCC_ADC1);
}

static void gpio_setup(void){
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO1);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
		      GPIO_CNF_INPUT_ANALOG, GPIO5);
}

static void pwm_timer_setup(void){
  timer_reset(TIM2);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  timer_set_period(TIM2, 1024); // ~ 36kHz PWM output in center-aligned mode
  timer_enable_break_main_output(TIM2);

  timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM2, TIM_OC2);
  timer_set_oc_value(TIM2, TIM_OC2, 0);

  timer_enable_counter(TIM2);
}

static void adc_setup(void){
  uint8_t channel_array[16];

  adc_power_off(ADC1);
  adc_disable_scan_mode(ADC1);
  adc_set_right_aligned(ADC1);
  adc_set_single_conversion_mode(ADC1);
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_41DOT5CYC);
  adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_TIM1_CC1);
  adc_disable_analog_watchdog_regular(ADC1);

  channel_array[0] = ADC_CHANNEL5;
  adc_set_regular_sequence(ADC1, 1, channel_array);
  adc_enable_dma(ADC1);

  adc_power_on(ADC1);

  /* Wait for ADC1 and ADC2 starting up. */
  for (int i = 0; i < 800000; i++)    /* Wait a bit. */
    __asm__("nop");

  adc_reset_calibration(ADC1);
  adc_calibrate(ADC1);
}

static void dma_setup(void){
  adc_power_off(ADC1);

  dma_channel_reset(DMA1, DMA_CHANNEL1);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL1, (uint32_t)&(ADC1_DR));
  dma_set_memory_address(DMA1, DMA_CHANNEL1, (uint32_t)&adc_value);
  dma_set_read_from_peripheral(DMA1, DMA_CHANNEL1);
  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL1);
  dma_disable_memory_increment_mode(DMA1, DMA_CHANNEL1);
  dma_set_number_of_data(DMA1, DMA_CHANNEL1, 1);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL1, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL1, DMA_CCR_MSIZE_16BIT);
  dma_enable_circular_mode(DMA1, DMA_CHANNEL1);
  dma_enable_channel(DMA1, DMA_CHANNEL1);

  dma_channel_reset(DMA1, DMA_CHANNEL3);
  dma_set_peripheral_address(DMA1, DMA_CHANNEL3, (uint32_t)&(TIM2_CCR2));
  dma_set_memory_address(DMA1, DMA_CHANNEL3, (uint32_t)&adc_value);
  dma_set_read_from_memory(DMA1, DMA_CHANNEL3);
  dma_disable_peripheral_increment_mode(DMA1, DMA_CHANNEL3);
  dma_disable_memory_increment_mode(DMA1, DMA_CHANNEL3);
  dma_set_number_of_data(DMA1, DMA_CHANNEL3, 1);
  dma_set_peripheral_size(DMA1, DMA_CHANNEL3, DMA_CCR_PSIZE_16BIT);
  dma_set_memory_size(DMA1, DMA_CHANNEL3, DMA_CCR_MSIZE_16BIT);
  dma_enable_circular_mode(DMA1, DMA_CHANNEL3);
  dma_enable_channel(DMA1, DMA_CHANNEL3);
}

static void timer_setup(void){
  timer_reset(TIM1);
  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_set_prescaler(TIM1, 624); // 115.2kHz
  timer_set_period(TIM1, 450); //  256Hz in edge-aligned mode
  timer_enable_break_main_output(TIM1);
  timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1); 
  timer_set_oc_value(TIM1, TIM_OC1, 100);
  timer_enable_oc_output(TIM1, TIM_OC1);
  timer_set_master_mode(TIM1, TIM_CR2_MMS_COMPARE_OC1REF);

  timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM1); 
  timer_set_oc_value(TIM1, TIM_OC2, 200);
  timer_enable_irq(TIM1, TIM_DIER_CC2DE); // DMA channel 3

  timer_enable_counter(TIM1);
}

int main(void){
  clock_setup();
  gpio_setup();
  pwm_timer_setup();
  dma_setup();
  adc_setup();
  timer_setup();

  while(1){
  }

  return 0;
}
