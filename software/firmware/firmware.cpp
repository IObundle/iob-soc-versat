#include <cstdio>

#include "accel.hpp"

extern "C"{
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"
#include "iob-ila.h"

#include "crypto/sha2.h"

int printf_(const char* format, ...);
}

#ifdef PC
#define uart_finish(...) ((void)0)
#define uart_init(...) ((void)0)
#else
#define printf printf_
#endif

int main(int argc,char* argv[]){
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);
   ila_init(ILA_BASE);

   printf("Init base modules\n");

   versat_init(VERSAT_BASE);

   TOP_input_0_constant = 0x1;
   TOP_input_1_constant = 0x2;

   RunAccelerator(1);

   printf("%d\n",TOP_output_0_currentValue);

   uart_finish();

   return 0;
}









