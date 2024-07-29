#include "bsp.h"
#include "iob-timer.h"
#include "iob-uart.h"
#include "iob_soc_versat_conf.h"
#include "iob_soc_versat_periphs.h"
#include "iob_soc_versat_system.h"
#include "printf.h"
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

int RunTest(int versatBase);

#ifdef __cplusplus
}
#endif

static char HexToInt(char ch){
   if('0' <= ch && ch <= '9'){
      return (ch - '0');
   } else if('a' <= ch && ch <= 'f'){
      return ch - 'a' + 10;
   } else if('A' <= ch && ch <= 'F'){
      return ch - 'A' + 10;
   } else {
      printf("Error, invalid character inside hex string:%c",ch);
      return 0;
   }
}

// Make sure that buffer is capable of storing the whole thing. Returns number of bytes inserted
int HexStringToHex(char* buffer,const char* str){
   int inserted = 0;
   for(int i = 0; ; i += 2){
      char upper = str[i];
      char lower = str[i+1];

      if(upper == '\0' || lower == '\0'){
         if(upper != '\0') printf("Warning: HexString was not divisible by 2\n");
         break;
      }   

      buffer[inserted++] = HexToInt(upper) * 16 + HexToInt(lower);
   }

   return inserted;
}

char GetHexadecimalChar(unsigned char value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'A' + (value - 10);
  }
}

char* GetHexadecimal(const char* text,char* buffer,int byte_size){
  int i = 0;
  unsigned char* view = (unsigned char*) text;
  for(; i< byte_size; i++){
    buffer[i*2] = GetHexadecimalChar(view[i] / 16);
    buffer[i*2+1] = GetHexadecimalChar(view[i] % 16);
  }

  buffer[i*2] = '\0';

  return buffer;
}

#define SIM

int main(int argc,char* argv[]){
   char pass_string[] = "Test passed!";
   char fail_string[] = "Test failed!";

   // init uart
   uart_init(UART0_BASE, FREQ / BAUD);
   printf_init(&uart_putc);

   uart_puts("\n\n\nGonna run test\n\n\n");
   
   int result = RunTest(VERSAT0_BASE);

   if(result){
      uart_sendfile("test.log", 12, fail_string);
   } else {
      uart_sendfile("test.log", 12, pass_string);
   }

   printf("Gonna call uart finish\n");
   uart_finish();

   return 0;
}
