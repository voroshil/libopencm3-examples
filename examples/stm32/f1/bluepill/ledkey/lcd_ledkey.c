#include "lcd_ledkey.h"
#include "lcd_tm1628_font.h"

#define MEMORY_SIZE 16
/*
Command 1. Display mode
MSB  LSB
00----bb
b: 00 - 4 GRIDs, 13 SEGs
   01 - 5 GRIDs, 12 SEGs
   10 - 6 GRIDs, 11 SEGs
   11 - 7 GRIDs, 10 SEGs (default)
Command 2. Data settings
MSB  LSB
01--bcdd
b: 0 - Normal Mode
   1 - Test Mode
c: 0 - Auto increment
   1 - Fixed address
d: 00 - Data write
   10 - Data read
Command 3. Set address
MSB  LSB
11--bbbb
b: Memory address 0x00 - 0x0d

Command 4. Display control
MSB  LSB
10--bccc
b: 0 - display off
   1 - display on
c: intensity

*/
// Command 1
#define LCD_CMD_DISPLAY_MODE 0x00
// Command 2
#define LCD_CMD_DATA_SETTING 0x40
// Command 3
#define LCD_CMD_ADDR_SETTING 0xC0
// Command 4
#define LCD_CMD_DISPLAY_CTRL 0x80


// Data settings command arguments
#define DATA_TEST_MODE   8
#define DATA_NORMAL_MODE 0
#define DATA_FIXED       4
#define DATA_INCREMENT   0
#define DATA_READ        2
#define DATA_WRITE       0

#define DISPLAY_ON       8
#define DISPLAY_OFF      0

#define MODE_4x13  0
#define MODE_5x12  1
#define MODE_6x11  2
#define MODE_7x10  3 // default

static uint8_t lk_cursor_pos = 0;

uint8_t buffer[MEMORY_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static void ss_stub(uint8_t cs){}
static uint32_t receive_stub(void){return 0;}
static void send_stub(uint8_t param){}

static lk_ss_t lk_ss_cb = ss_stub;
static lk_receive_t lk_receive_cb = receive_stub;
static lk_send_t lk_send_cb = send_stub;

void lk_send_command(uint8_t data)
{
  lk_ss_cb(0);
  lk_send_cb(data);
  lk_ss_cb(1);
}
void lk_send_data(uint8_t addr, uint8_t data)
{
  lk_send_command(LCD_CMD_DATA_SETTING | DATA_FIXED | DATA_WRITE);
  lk_ss_cb(0);
  lk_send_cb(LCD_CMD_ADDR_SETTING | addr);
  lk_send_cb(data);
  lk_ss_cb(1);
}
void lk_begin(uint8_t active, uint8_t intensity)
{
  lk_send_command(LCD_CMD_DISPLAY_CTRL | (active ? DISPLAY_ON : DISPLAY_OFF) | (intensity > 7 ? 7 : intensity));
}
void lk_update()
{
  uint8_t i;
  for(i=0; i<MEMORY_SIZE; i++){
    lk_send_data(i, buffer[i]);
  }
}

void lk_clear()
{
  uint8_t i;
  for(i=0; i<MEMORY_SIZE; i++){
    buffer[i] = 0x00;
  }
  lk_cursor_pos = 0;
}
void lk_led_toggle(uint8_t led)
{
  if (led < 8){
    buffer[(led<<1)+1] ^= 0x1;
  }else if (led < 16){
    buffer[(led-8)<<1] ^= 0x80;
  }
}
void lk_led_on(uint8_t led)
{
  if (led < 8){
    buffer[(led<<1)+1] |= 0x1;
  }else if (led < 16){
    buffer[(led-8)<<1] |= 0x80;
  }
}
void lk_led_off(uint8_t led)
{
  if (led < 8){
    buffer[(led<<1)+1] &= ~(0x1);
  }else if (led < 16){
    buffer[(led-8)<<1] &= ~(0x80);
  }
}
void lk_led_all(uint8_t on)
{
  uint8_t i;
  if (on){
    for(i=0; i<8; i++){
      lk_led_on(i);
    }
  }else{
    for(i=0; i<8; i++){
      lk_led_off(i);
    }
  }
}
void lk_set_cursor(uint8_t pos)
{
  lk_cursor_pos = pos;
}

void lk_apply_font(uint8_t glyph, uint8_t addr)
{
  if (addr < 8){
    buffer[addr<<1] = (glyph & 0x7f) | (buffer[addr<<1] & 0x80);
  }
}
void lk_set_seg(uint8_t addr, uint8_t num) {
  lk_apply_font(NUMBER_FONT[num], addr);
}

void lk_set_char(uint8_t addr, char chr) {
  lk_apply_font(FONT_DEFAULT[chr - 0x20], addr);
}
void lk_write(uint8_t chr)
{
  if (lk_cursor_pos < 7){
    lk_set_char(lk_cursor_pos, chr);
    lk_cursor_pos++;
  }
}


#if 0
static uint8_t tm1638_read_keys(){
    uint32_t bits = 0;
    uint8_t keys = 0;
    int i;
    digitalWrite(STB, 0);
    tm1638_send_byte(0x42); // read
    pinMode(DIO, INPUT);
    for (i=0; i<32; i++){
        digitalWrite(CLK, 0);
        digitalWrite(CLK, 1);
        bits |= digitalRead(DIO) ? (1<<i) : 0;
    }
    pinMode(DIO, OUTPUT);
    digitalWrite(STB, 1);
    keys |= (bits & 0x00000001) ? 0x01 : 0;
    keys |= (bits & 0x00000100) ? 0x02 : 0;
    keys |= (bits & 0x00010000) ? 0x04 : 0;
    keys |= (bits & 0x01000000) ? 0x80 : 0;
    keys |= (bits & 0x00000010) ? 0x10 : 0;
    keys |= (bits & 0x00001000) ? 0x20 : 0;
    keys |= (bits & 0x00100000) ? 0x40 : 0;
    keys |= (bits & 0x10000000) ? 0x80 : 0;
    return keys;
}
#endif

uint8_t lk_get_buttons(void) {
  uint32_t bits = 0;
  uint8_t keys = 0;

  lk_ss_cb(0);
  lk_send_cb(LCD_CMD_DATA_SETTING | DATA_INCREMENT | DATA_READ);
  bits = lk_receive_cb();
  lk_ss_cb(1);

  keys |= (bits & 0x00000001) ? 0x01 : 0;
  keys |= (bits & 0x00000100) ? 0x02 : 0;
  keys |= (bits & 0x00010000) ? 0x04 : 0;
  keys |= (bits & 0x01000000) ? 0x08 : 0;
  keys |= (bits & 0x00000010) ? 0x10 : 0;
  keys |= (bits & 0x00001000) ? 0x20 : 0;
  keys |= (bits & 0x00100000) ? 0x40 : 0;
  keys |= (bits & 0x10000000) ? 0x80 : 0;

  return keys;
}
void lk_init(lk_receive_t receive_cb, lk_send_t send_cb, lk_ss_t ss_cb){ 
  lk_receive_cb = receive_cb;
  lk_send_cb = send_cb;
  lk_ss_cb = ss_cb;

   /*
      Recommended init flowchart:
       Delay 200ms
       Command 2 (write data)
       Command 3 (Clear RAM) see Note 5
       Command 1
       Command 4
    */

   // Command 2 & Command 3
   lk_clear();
   // Command 1
//   lk_send_command(LCD_CMD_DISPLAY_MODE | MODE_7x11);
   // Command 4
   lk_begin(DISPLAY_ON, 7);

}

void lk_test(lk_delay_t delay_cb)
{
  uint8_t i;

  for (i=0; i<16;i++){
    lk_set_seg(0, i);
    lk_set_seg(1, i);
    lk_set_seg(2, i);
    lk_set_seg(3, i);
    lk_set_seg(4, i);
    lk_set_seg(5, i);
    lk_set_seg(6, i);
    lk_set_seg(7, i);
    lk_update();
    delay_cb();
  }

  lk_clear();
  lk_update();
  for (i=0; i<8; i++){
    lk_led_on(i);
    lk_update();
    delay_cb();
  }
  for (i=0; i<9;i++){
    lk_apply_font(0x7f, i);
    lk_update();
    delay_cb();
  }
  lk_clear();
  lk_update();
}

void lk_test2(lk_delay_t delay_cb)
{
  uint8_t i;

  for (i=0; i<16;i++){
    lk_set_seg(0, i);
    lk_set_seg(1, i);
    lk_set_seg(2, i);
    lk_set_seg(3, i);
    lk_set_seg(4, i);
    lk_set_seg(5, i);
    lk_set_seg(6, i);
    lk_set_seg(7, i);
    lk_update();
    delay_cb();
  }
  lk_clear();
  lk_update();
  for (i=0; i<16; i++){
    lk_led_on(i);
    lk_update();
    delay_cb();
  }
  lk_clear();
  lk_update();
}
