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

  ACCEL_TOP_input_0_constant = PackInt(1.0f);
  ACCEL_TOP_input_1_constant = PackInt(1.0f);

  RunAccelerator(1);

  printf("%f\n",ACCEL_TOP_output_0_currentValue);

  uart_finish();

  return 0;
}
