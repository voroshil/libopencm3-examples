#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>

#include "onewire.h"

void onewire_init(){
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_USART3);

  gpio_set_mode(GPIO_BANK_USART3_TX, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO_USART3_TX); // PB10

  usart_set_baudrate(USART3, 115200);
  usart_set_databits(USART3, 8);
  usart_set_parity(USART3, USART_PARITY_NONE);
  usart_set_stopbits(USART3, USART_STOPBITS_1);
  usart_set_flow_control(USART3, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART3, USART_MODE_TX_RX);
  USART_CR3(USART3) |= USART_CR3_HDSEL;

  usart_enable(USART3);
}
#define OW_R 0xf0
#define OW_1 0xff
#define OW_0 0x00
uint8_t onewire_reset(){
  uint8_t res = 0;
  usart_disable(USART3);
  usart_set_baudrate(USART3, 9600);
  usart_enable(USART3);
  usart_send(USART3, OW_R);
  while ((USART_SR(USART3) & USART_SR_TC) == 0);
  res = usart_recv_blocking(USART3);
  usart_disable(USART3);
  usart_set_baudrate(USART3, 115200);
  usart_enable(USART3);
  return (res >= 0x10 && res <= 0x90);
}
void ow_send_bit(uint8_t v){
  usart_send(USART3, v ? OW_1 : OW_0);
  while ((USART_SR(USART3) & USART_SR_TC) == 0);
  usart_recv_blocking(USART3);
}
uint8_t ow_receive_bit(){
  uint8_t res;
  usart_send(USART3, OW_1);
  while ((USART_SR(USART3) & USART_SR_TC) == 0);
  res = usart_recv_blocking(USART3);
  return res == OW_1;
}
void ow_send_byte(uint8_t v) {
  uint8_t i;

  for (i=0; i<8; i++) {
    ow_send_bit(v & 1);
    v >>= 1;
  }
}

void ow_send_bytes(const uint8_t *buf, uint16_t count) {
  uint16_t i;
  for (i = 0 ; i < count ; i++)
    ow_send_byte(buf[i]);
}

uint8_t ow_receive_byte() {
  uint8_t i;
  uint8_t r = 0;

  for (i = 0; i<8; i++) {
    r >>= 1;
    if (ow_receive_bit()) 
      r |= 0x80;
  }
  return r;
}

void ow_receive_bytes(uint8_t *buf, uint16_t count) {
  uint16_t i;
  for (i = 0 ; i < count ; i++)
    buf[i] = ow_receive_byte();
}
void ow_select(uint8_t rom[8])
{
  uint8_t i;

  ow_send_byte(OW_MATCH_ROM);           // Choose ROM
  for(i=0; i<8; i++) ow_send_byte(rom[i]);
}

void ow_skip()
{
  ow_send_byte(OW_SKIP_ROM);           // Skip ROM
}

