#ifndef INCLUDED_TESTBENCH
#define INCLUDED_TESTBENCH

#include <cstdio>

#include "accel.hpp"

extern "C"{
#include "system.h"
#include "periphs.h"
#include "iob-uart.h"
#include "string.h"

#include "iob-timer.h"
#include "iob-ila.h"

int printf_(const char* format, ...);
}

#ifdef PC
#define uart_finish(...) ((void)0)
#define uart_init(...) ((void)0)
#else
#define printf printf_
#endif

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

static bool error = false; // Global keep track if a error occurred. Do not want to print error messages more than once

static char expectedBuffer[16*1024];
static char gotBuffer[16*1024];

static char* expectedPtr = expectedBuffer;
static char* gotPtr = gotBuffer;

static void ResetTestBuffers(){
   expectedPtr = expectedBuffer;
   gotPtr = gotBuffer;
}

static void PushExpected(int val){
   expectedPtr += sprintf(expectedPtr,"0x%x ",val);
}

static void PushExpected(const char* str){
   expectedPtr += sprintf(expectedPtr,"%s ",str);
}

static void PushGot(int val){
   gotPtr += sprintf(gotPtr,"0x%x ",val);
}

static void PushGot(const char* str){
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
   PushExpected(val1);
   PushGot(val2);

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
      PushExpected(expected);
      PushGot(buffer);
   }
}

static bool ExpectMemory(int* expected,int size, int* output){
   bool result = (memcmp(expected,output,size) == 0);

   if(!result){
      error = true;
      char expectedStr[1024 * 16];
      char gotStr[1024 * 16];

      char* expectedPtr = expectedStr;
      char* gotPtr = gotPtr;
      for(int i = 0; i < size; i++){
         PushExpected(expected[i]);
         PushGot(output[i]);
      }
   }
}

void SingleTest();

int main(int argc,char* argv[]){
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);
   ila_init(ILA_BASE);

   versat_init(VERSAT_BASE);

   SingleTest();

   if(error){
      PrintError();
   } else {
      printf("%s: OK\n",acceleratorTypeName);
   }

   uart_finish();

   return 0;
}

#endif // INCLUDED_TESTBENCH












