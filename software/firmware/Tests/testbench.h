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

#if defined(__cplusplus) && defined(PC)
extern "C" {
#endif

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"
#include "iob-eth.h"

#if defined(__cplusplus) && defined(PC)
}
#endif

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

#ifdef PC
static char expectedBuffer[1024*1024];
static char gotBuffer[1024*1024];

static char* expectedPtr = expectedBuffer;
static char* gotPtr = gotBuffer;
#endif

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
#ifdef PC
   expectedPtr = expectedBuffer;
   gotPtr = gotBuffer;
#endif
}

static void PushExpectedI(int val){
#ifdef PC
   expectedPtr += sprintf(expectedPtr,"0x%x ",val);
#endif
}

static void PushGotI(int val){
#ifdef PC
   gotPtr += sprintf(gotPtr,"0x%x ",val);
#endif
}

static void PushExpectedF(float val){ // NOTE: Floating point rounding can make these fail. If so, just push a certain amount of decimal places
#ifdef PC
   expectedPtr += sprintf(expectedPtr,"%f ",val);
#endif
}

static void PushGotF(float val){
#ifdef PC
   gotPtr += sprintf(gotPtr,"%f ",val);
#endif
}

static void PushExpectedS(const char* str){
#ifdef PC
   expectedPtr += sprintf(expectedPtr,"%s ",str);
#endif
}

static void PushGotS(const char* str){
#ifdef PC
   gotPtr += sprintf(gotPtr,"%s ",str);
#endif
}

static void PrintError(){
#ifdef PC
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
#endif
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

void SingleTest(Arena* arena);

int main(int argc,char* argv[]){
   uart_init(UART_BASE,FREQ/BAUD);
   timer_init(TIMER_BASE);
   ila_init(ILA_BASE);

   versat_init(VERSAT_BASE);
   
   Arena arenaInst = InitArena(Megabyte(16));
   Arena* arena = &arenaInst;

   SingleTest(arena);

#ifdef PC
   if(error){
      PrintError();
   } else {
      int expectedDiff = (expectedPtr - expectedBuffer);
      int gotDiff = (gotPtr - gotBuffer);

      bool passed = true;
      if(expectedDiff != gotDiff){
         passed = false;
      } else {
         for(int i = 0; i < expectedDiff; i++){
            if(expectedBuffer[i] != gotBuffer[i]){
               passed = false;
               break;
            }
         }
      }         

      if(passed){
         printf("%s: OK\n",acceleratorTypeName);
      } else {
         printf("%s: ERROR\n",acceleratorTypeName);
#if 0
         printf("Exp:%s\n",expectedBuffer);
         printf("Got:%s\n",gotBuffer);
#endif
      }         
      
#if 0
      if(passed){
         printf("Exp:%s\n",expectedBuffer);
         printf("Got:%s\n",gotBuffer);
      }
#endif
   }
#endif
   
   uart_finish();

   return 0;
}

#endif // INCLUDED_TESTBENCH
