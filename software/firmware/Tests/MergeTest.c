#include "versat_accel.h"

#include "system.h"
#include "periphs.h"
#include "string.h"

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"
#include "iob-eth.h"

#include "printf.h"

int main(int argc,const char* argv[]){
  uart_init(UART_BASE,FREQ/BAUD);
  timer_init(TIMER_BASE);
  ila_init(ILA_BASE);
  
  versat_init(VERSAT_BASE);

  uart_finish();
  
  return 0;   
}


