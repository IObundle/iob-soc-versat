#include "versat_accel.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"
#include "iob-ila.h"

#ifdef __cplusplus
}
#endif

#ifdef PC
#define uart_init(...) ((void)0)
#define uart_finish(...) ((void)0)
#include "stdio.h"
#else

int printf_(const char* format, ...);
#define printf printf_
#endif

typedef union{
   iptr i;
   float f;
} Conv;

static iptr PackInt(float f){
   Conv c = {};
   c.f = f;
   return c.i;
}

static float UnpackInt(iptr i){
   Conv c = {};
   c.i = i;
   return c.f;
}

int main(int argc,char* argv[]){
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);
   ila_init(ILA_BASE);

   printf("Init base modules\n");

   versat_init(VERSAT_BASE);

   printf("%x\n",&ACCEL_TOP_input_0_constant);

   ACCEL_TOP_input_0_constant = PackInt(1.1);
   ACCEL_TOP_input_1_constant = PackInt(2);

   RunAccelerator(1);

   printf("%f\n",UnpackInt(ACCEL_TOP_output_0_currentValue));
   /*
   ACCEL_TOP_input_0_constant = 100;
   ACCEL_TOP_input_1_constant = 20;
   ACCEL_TOP_input_2_constant = 3;

   RunAccelerator(1);

   printf("%d\n",ACCEL_TOP_output_0_currentValue);
   */

   uart_finish();

   return 0;
}









