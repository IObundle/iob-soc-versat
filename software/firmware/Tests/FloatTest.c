#include "versat_accel.h"

#include "system.h"
#include "periphs.h"

#include "string.h"

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"
#include "iob-eth.h"

#ifdef PC
// Remove having to use console to test firmware
#define uart_init(...) ((void)0)
#define uart_finish(...) ((void)0)
#include "stdio.h"
#else
#include "printf.h"
#endif

typedef union {
   iptr i;
   float f;
} Conv;

static int PackInt(float f){
   Conv c = {};
   c.f = f;
   return c.i;
}

int main(int argc,const char* argv[]){
  uart_init(UART_BASE,FREQ/BAUD);
  timer_init(TIMER_BASE);
  ila_init(ILA_BASE);

  versat_init(VERSAT_BASE);

#if 0
  {
  float start = 0.5f;
  float max = 1e10f;
  for(float val = start; val < max; val *= 2){
    ACCEL_TOP_input_0_constant = PackInt(val);

    RunAccelerator(1);

    int result = ACCEL_TOP_output_0_currentValue;

    if(1){
      printf("%f %d %d\n",val,result,(int) val);
    }
  }
  }
#endif

#if 0
  {
  float start = -0.5f;
  float max = -1e10f;
  for(float val = start; val > max; val *= 2){
    ACCEL_TOP_input_0_constant = PackInt(val);

    RunAccelerator(1);

    int result = ACCEL_TOP_output_0_currentValue;

    if(1){
      printf("%f %d %d\n",val,result,(int) val);
    }
  }
  }
#endif

#if 1
  float val = -2020.764893f;
  ACCEL_TOP_input_0_constant = PackInt(val);
  RunAccelerator(1);
  int result = ACCEL_TOP_output_0_currentValue;
  printf("%d %d\n",result,(int) val);
#endif
  
  uart_finish();

  return 0;
}

