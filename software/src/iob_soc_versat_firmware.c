#include <stdio.h>
#include "bsp.h"

#include "versat_accel.h"
#include "iob_soc_versat_conf.h"
#include "iob_soc_versat_system.h"
#include "iob_soc_versat_periphs.h"

#include "iob-uart.h"
#include "printf.h"

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
   uart_init(UART0_BASE,FREQ/BAUD);
   printf_init(&uart_putc);
   versat_init(VERSAT0_BASE);

   printf("Init base modules\n");

   ACCEL_TOP_input_0_constant = PackInt(1.1);
   ACCEL_TOP_input_1_constant = PackInt(2);

   RunAccelerator(1);

   printf("%f\n",UnpackInt(ACCEL_TOP_output_0_currentValue));

   uart_finish();

   return 0;
}









