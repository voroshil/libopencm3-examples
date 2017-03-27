#include "onewire.h"

uint8_t ow_data_array[10];

int main(void){
  uint17_t temperature;

  onewrite_init();


  if (!onewire_reset()){
    usart_print("Onewire reset failed\n");
  }else{
    ow_data_array[0] = OW_SKIP_ROM;
    ow_data_array[1] = OW_CONVERT_T;
    ow_send_bytes(ow_data_array, 2);
    _delay_ms(800);

    if (!onewire_reset()){
      usart_print("Onewire reset failed\n");
    }else{
      ow_data_array[0] = OW_SKIP_ROM;
      ow_data_array[1] = OW_READ_SCRATCHPAD;
      ow_send_bytes(ow_data_array, 2);

      ow_receive_bytes(ow_data_array, 9);
  
      temperature = ow_unpack_temp(ow_data_array);
      usart_print("Temperature:");
      usart_decimal(temperature,1);
      usart_print(" C\n");
    }
  }

  while(1){}
  return 0;
}