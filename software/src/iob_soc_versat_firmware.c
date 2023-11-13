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

static float fabs(float f){
   if(f < 0.0f){
      return -f;
   } else {
      return f;
   }
}

int main(int argc,char* argv[]){
   char pass_string[] = "Test passed!";
   char fail_string[] = "Test failed!";

   uart_init(UART0_BASE,FREQ/BAUD);
   printf_init(&uart_putc);
   versat_init(VERSAT0_BASE);

   printf("Init base modules\n");

   ACCEL_TOP_input_0_constant = PackInt(1.1);
   ACCEL_TOP_input_1_constant = PackInt(2);

   RunAccelerator(1);

   float result = UnpackInt(ACCEL_TOP_output_0_currentValue); 
   printf("%f\n",result);
   if(fabs(result - 3.1f) < 0.01){
      uart_sendfile("test.log", iob_strlen(pass_string), pass_string);
   } else {
      uart_sendfile("test.log", iob_strlen(fail_string), fail_string);
   }

   uart_finish();

   return 0;
}









