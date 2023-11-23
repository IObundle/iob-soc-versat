#include <stdio.h>

#include "bsp.h"

#include "iob_soc_versat_conf.h"
#include "iob_soc_versat_system.h"
#include "iob_soc_versat_periphs.h"

#include "iob-uart.h"
#include "iob_str.h"
#include "printf.h"

#ifdef __cplusplus
extern "C"{
#endif
int RunTest(int versatBase);
#ifdef __cplusplus
}
#endif

int main(int argc,char* argv[]){
   char pass_string[] = "Test passed!";
   char fail_string[] = "Test failed!";

   uart_init(UART0_BASE,FREQ/BAUD);
   printf_init(&uart_putc);

   int result = RunTest(VERSAT0_BASE);

   if(result){
      uart_sendfile("test.log", iob_strlen(fail_string), fail_string);
   } else {
      uart_sendfile("test.log", iob_strlen(pass_string), pass_string);
   }

   printf("Gonna call uart finish\n");
   uart_finish();

   return 0;
}
