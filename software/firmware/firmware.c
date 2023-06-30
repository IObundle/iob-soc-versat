#include "versat_accel.hpp"

#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"
#include "iob-ila.h"

int printf_(const char* format, ...);

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

   RunAccelerator(1);

   uart_finish();

   return 0;
}









