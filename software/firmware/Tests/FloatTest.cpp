#include <cinttypes>
#include <limits>
#include <algorithm>

#include "utils.hpp"

#include "versat_accel.h"

#include "system.h"
#include "periphs.h"

#include "string.h"

#if defined(__cplusplus) && defined(PC)
extern "C" {
#endif

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"
#include "iob-eth.h"

#if defined(__cplusplus) && defined(PC)
}
#endif

#ifdef PC
// Remove having to use console to test firmware
#define uart_init(...) ((void)0)
#define uart_finish(...) ((void)0)
#include "stdio.h"
#else
#include "printf.h"
#endif

int main(int argc,const char* argv[]){
  uart_init(UART_BASE,FREQ/BAUD);
  timer_init(TIMER_BASE);
  ila_init(ILA_BASE);

  versat_init(VERSAT_BASE);

  SeedRandomNumber(1);
  
  for(int i = 0; i < 1000; i++){
    float val1 = PackFloat(GetRandomNumber());
    float val2 = PackFloat(GetRandomNumber());
      
    ACCEL_TOP_input_0_constant = PackInt(val1);
    ACCEL_TOP_input_1_constant = PackInt(val2);
  
    RunAccelerator(1);

    int res = ACCEL_TOP_output_0_currentValue;

    if(!FloatEqual(PackFloat(res),val1 + val2,0.0001f)){
      printf("%d\n",i);
      printf("%f %f\n",PackFloat(res),val1 + val2);
    }
  }
  printf("End\n");
  
  uart_finish();

  return 0;
}

