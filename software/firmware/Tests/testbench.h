#ifndef INCLUDED_TESTBENCH
#define INCLUDED_TESTBENCH

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
#include "memory.hpp"
#include "utils.hpp"
#endif

#include "versat_accel.h" // C++, include outside

#include "system.h"
#include "periphs.h"
#include "string.h"

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"
#include "iob-eth.h"

#ifndef __cplusplus
#define bool unsigned char
#define true 1
#define false 0
#endif

#ifdef PC
// Remove having to use console to test firmware
#define uart_init(...) ((void)0)
#define uart_finish(...) ((void)0)
#include "stdio.h"
#else
#include "printf.h"
#endif

static const int TEST = 5;

static bool error = false; // Global keep track if a error occurred. Do not want to print error messages more than once

static char expectedBuffer[16*1024];
static char gotBuffer[16*1024];

static char* expectedPtr = expectedBuffer;
static char* gotPtr = gotBuffer;

#ifndef __cplusplus // C++ code already has access to these functions
typedef union {
   iptr i;
   float f;
} Conv;

static int PackInt(float f){
   Conv c = {};
   c.f = f;
   return c.i;
}

static float PackFloat(int i){
   Conv c = {};
   c.i = i;
   return c.f;
}
#endif

static void ResetTestBuffers(){
   expectedPtr = expectedBuffer;
   gotPtr = gotBuffer;
}

static void PushExpectedI(int val){
   expectedPtr += sprintf(expectedPtr,"0x%x ",val);
}

static void PushGotI(int val){
   gotPtr += sprintf(gotPtr,"0x%x ",val);
}

static void PushExpectedF(float val){ // NOTE: Floating point rounding can make these fail. If so, just push a certain amount of decimal places
   expectedPtr += sprintf(expectedPtr,"%f ",val);
}

static void PushGotF(float val){
   gotPtr += sprintf(gotPtr,"%f ",val);
}

static void PushExpectedS(const char* str){
   expectedPtr += sprintf(expectedPtr,"%s ",str);
}

static void PushGotS(const char* str){
   gotPtr += sprintf(gotPtr,"%s ",str);
}

static void PrintError(){
   char* expected = expectedBuffer;
   char* got = gotBuffer;

   printf("\n");
   printf("%s: Test Failed\n",acceleratorTypeName);
   printf("    Expected: %s\n",expected);
   printf("    Result:   %s\n",got);
   printf("              ");
   for(int i = 0; expected[i] != '\0'; i++){
      if(got[i] == '\0'){
         printf("^");
         break;
      }
      if(got[i] != expected[i]){
         printf("^");
      } else {
         printf(" ");
      }
   }

   printf("\n");
}

static void Assert_Eq(int val1,int val2){
   PushExpectedI(val1);
   PushGotI(val2);

   if(val1 != val2){
      error = true;
   }
}

static void Assert_EqF(float val1,float val2){
   PushExpectedF(val1);
   PushGotF(val2);

   if(val1 != val2){
      error = true;
   }
}

static void Expect(const char* expected,const char* format, ...){
   va_list args;
   va_start(args,format);

   char buffer[1024 * 16];
   int size = vsprintf(buffer,format,args);

   va_end(args);

   bool result = (strcmp(expected,buffer) == 0);

   if(!result){
      error = true;
      PushExpectedS(expected);
      PushGotS(buffer);
   }
}

static void ExpectMemory(int* expected,int size, int* output){
   bool result = (memcmp(expected,output,size) == 0);

   if(!result){
      error = true;
      char expectedStr[1024 * 16];
      char gotStr[1024 * 16];

      char* expectedPtr = expectedStr;
      char* gotPtr = gotPtr;
      for(int i = 0; i < size; i++){
         PushExpectedI(expected[i]);
         PushGotI(output[i]);
      }
   }
}

void SingleTest();

#define OUTPUT_ALL 1

int main(int argc,char* argv[]){
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);
   ila_init(ILA_BASE);

   versat_init(VERSAT_BASE);

   #if 0
   eth_init(ETHERNET_BASE);
   char* buffer = (char*) malloc(1024 * 1024);

   printf("Waiting for file receive\n");
   
   int size = eth_rcv_variable_file(buffer);

   printf("Received: %d\n",size);
   #endif
   
   SingleTest();

   if(error){
      PrintError();
   } else {
      printf("%s: OK\n",acceleratorTypeName);

      #ifdef OUTPUT_ALL
      printf("Exp:%s\n",expectedBuffer);
      printf("Got:%s\n",gotBuffer);
      #endif
   }
   
   uart_finish();

   return 0;
}

#endif // INCLUDED_TESTBENCH












