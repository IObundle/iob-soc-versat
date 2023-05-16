#include <cstdio>

#include "versat.hpp"
#include "utils.hpp"

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

Versat* versat;

void AutomaticTests(Versat* versat);

int main(int argc,char* argv[]){
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);
   ila_init(ILA_BASE);

   printf("Init base modules\n");

   Versat* versat = InitVersat(VERSAT_BASE,1);

   SetDebug(versat,VersatDebugFlags::OUTPUT_ACCELERATORS_CODE,1);
   SetDebug(versat,VersatDebugFlags::OUTPUT_VERSAT_CODE,1);
   SetDebug(versat,VersatDebugFlags::USE_FIXED_BUFFERS,0);
   SetDebug(versat,VersatDebugFlags::OUTPUT_GRAPH_DOT,0);
   SetDebug(versat,VersatDebugFlags::OUTPUT_VCD,0);

   ParseVersatSpecification(versat,"testVersatSpecification.txt");

   AutomaticTests(versat);

   uart_finish();

   return 0;
}









