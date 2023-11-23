#include "versat_accel.h"

#include "system.h"
#include "periphs.h"
#include "string.h"

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"
#include "iob-eth.h"

#include "printf.h"

#ifdef PC
#define uart_init(...) ((void)0)
#define uart_finish(...) ((void)0)
#undef printf
#endif

int main(int argc,const char* argv[]){
  uart_init(UART_BASE,FREQ/BAUD);
  timer_init(TIMER_BASE);
  ila_init(ILA_BASE);

  versat_init(VERSAT_BASE);

  MERGED_0_m0_constant = 10;
  MERGED_0_m1_constant = 5;

  RunAccelerator(1);
  
  printf("%d\n",ACCEL_TOP_output_Output_currentValue);

  MERGED_1_M0_constant = 10;
  MERGED_1_M1_constant = 2;
  
  ACCEL_TOP_comb_mux0_sel = 1;
  
  RunAccelerator(1);
  
  printf("%d\n",ACCEL_TOP_output_Output_currentValue);
  
  uart_finish();
  
  return 0;   
}


