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

// 9 bytes long
void ow_crc8(uint8_t data, uint8_t crc){
  for(int i=0; i<8; i++){
    uint8_t bit0 = (crc ^ data) & 0x01;
    crc = (crc >> 1) & 0x7F;
    if (bit0) {
      crc = crc ^ 0x8C; // (0x119 >> 1), X^8+X^5+X^4+X^0
    }
    data = data >> 1;
  }
  return crc;
}
/*
 * 0 8 - themperature LSB
 * 1 7 - themperature MSB (all higher bits are sign)
 * 2 6 - T_H
 * 3 5 - T_L
 * 4 4 - B20: Configuration register (only bits 6/5 valid: 9..12 bits resolution); 0xff for S20
 * 5 3 - 0xff (reserved)
 * 6 2 - (reserved for B20); S20: COUNT_REMAIN (0x0c)
 * 7 1 - COUNT PER DEGR (0x10)
 * 8 0 - CRC
 */
static const uint8_t fractions[] = {0, 1, 1, 2, 3, 3, 4, 4}
int16_t ow_unpack_temp(uint8_t *rom){
  uint8_t crc = 0;
  int16_t result;

  for(int i=0; i<9; i++) crc = ow_crc8(rom[i], crc);
  if (crc) {// wrong CRC
    return ERR_TEMP_VAL;
  }

  msb = rom[1];
  lsb = rom[0];

  if(rom[4] == 0xff){ // DS18S20
    result = 10 * (lsb >> 1);
    if (msb & 0x80) // minus
      result = -result;
    if (lsb & 1) // 0.5
      result += 5;
  }else{
    result = 10 * ((lsb >> 4) | ((msb & 7) << 4));
    if (msb & 0x80){ // minus
      result = -result;
    }
    if (lsb & 8) // 0.5
      result += 5;
    result += fractions[lsb & 7];
  }
  return result;
}
