#ifndef INCLUDED_TESTBENCH
#define INCLUDED_TESTBENCH

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
#include "memory.hpp"
#include "utils.hpp"
#endif

#include "versat_accel.h" // C++, include outside

#define Kilobyte(val) (val * 1024)
#define Megabyte(val) (Kilobyte(val) * 1024)
#define Gigabyte(val) (Megabyte(val) * 1024)

#include "system.h"
#include "periphs.h"
#include "string.h"

#include "iob-eth.h"

#if defined(__cplusplus) && defined(PC)
extern "C" {
#endif

#include "iob-uart.h"
#include "iob-timer.h"
#include "iob-ila.h"

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

#ifndef __cplusplus
#define nullptr NULL
#endif

static bool error = false; // Global keep track if a error occurred. Do not want to print error messages more than once

static const int TEST_BUFFER_AMOUNT = 100000; // 100K should be around 400Kbs per buffer (expected and got)

typedef enum {TEST_INTEGER,TEST_FLOAT} TestValueType;

typedef struct {
  union {
    int i;
    float f;
  };
  TestValueType type;
} TestValue;

static TestValue* expectedBuffer = nullptr;
static int expectedIndex;
static TestValue* gotBuffer = nullptr;
static int gotIndex;

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

#ifdef __cplusplus // C++ code already has access to these functions
#include <cmath>
#endif

#include "float.h"
// A C version of the C++ code with fixed epsilon
static bool MyFloatEqual(float f0,float f1){
  if(f0 == f1){
    return true;
  }

  float epsilon = 0.00001f;
  float norm = fabs(f0) + fabs(f1);
  if(norm > FLT_MAX) norm = FLT_MAX;
  float diff = fabs(f0 - f1);

  bool equal = diff < norm * epsilon;

  return equal;
}

static void ResetTestBuffers(){
  expectedIndex = 0;
  gotIndex = 0;
}

static void PushExpected(TestValue val){
  if(expectedIndex < TEST_BUFFER_AMOUNT){
    expectedBuffer[expectedIndex++] = val;
  } else {
    static bool done = false;
    if(!done){
      done = true;
      printf("Reached end of buffer for test samples!!!\n");
    }
  }
}

static void PushGot(TestValue val){
  if(gotIndex < TEST_BUFFER_AMOUNT){
    gotBuffer[gotIndex++] = val;
  } else {
    static bool done = false;
    if(!done){
      done = true;
      printf("Reached end of buffer for test samples!!!\n");
    }
  }}

static TestValue MakeTestValueInt(int i){
  TestValue val = {};
  val.i = i;
  val.type = TEST_INTEGER;
  return val;
}

static TestValue MakeTestValueFloat(float f){
  TestValue val = {};
  val.f = f;
  val.type = TEST_FLOAT;
  return val;
}

static bool TestValueEqual(TestValue v1,TestValue v2){
  if(v1.type != v2.type){
    return false;
  }

  bool res = false;
  switch(v1.type){
  case TEST_INTEGER:{
    res = (v1.i == v2.i);
  }break;
  case TEST_FLOAT:{
    res = MyFloatEqual(v1.f,v2.f);
  }break;
  }

  return res;
}

static void PrintTestValue(TestValue val){
  switch(val.type){
  case TEST_INTEGER:{
    printf("%d",val.i);
  }break;
  case TEST_FLOAT:{
    printf("%f",val.f);
  }break;
  }
}

static void PushExpectedI(int val){
  PushExpected(MakeTestValueInt(val));
}

static void PushGotI(int val){
  PushGot(MakeTestValueInt(val));
}

static void PushExpectedF(float val){ // NOTE: Floating point rounding can make these fail. If so, just push a certain amount of decimal places
  PushExpected(MakeTestValueFloat(val));
}

static void PushGotF(float val){
  PushGot(MakeTestValueFloat(val));
}

#ifdef __cplusplus // C++ code already has access to these functions
static void PushExpected(Array<int> arr){
  for(int f : arr){
    PushExpectedI(f);
  }
}

static void PushExpected(Array<float> arr){
  for(float f : arr){
    PushExpectedF(f);
  }
}

static void PushGot(Array<int> arr){
  for(int f : arr){
    PushGotI(f);
  }
}

static void PushGot(Array<float> arr){
  for(float f : arr){
    PushGotF(f);
  }
}
#endif

// TODO: These functions should assert while running in PC-EMUL and only push values when running in embedded.
//       Also, if we find problems in embedded, it might be useful to have a way of generating a breakpoint
//       that matches the index of the failure in embedded, so that we can know when running the PC-EMUL
//       the position of the error.

static void Assert_Eq(int val1,int val2){
  PushExpectedI(val1);
  PushGotI(val2);
}

static void Assert_EqF(float val1,float val2){
  PushExpectedF(val1);
  PushGotF(val2);
}

#if 0
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
#endif

static void Assert_Eq_Array(int* expected,int* got,int size){
  for(int i = 0; i < size; i++){
    PushExpectedI(expected[i]);
    PushGotI(got[i]);
  }
}

#ifndef __cplusplus
  typedef struct {
    char* mem;
    int used;
    int totalAllocated;
  } Arena;

  Arena InitArena(int amount){
    Arena arena = {};
    arena.mem = malloc(amount);
    arena.totalAllocated = amount;
    return arena;
  }

  void* PushBytes(Arena* arena,int amount){
    //assert(arena->used + amount < arena->totalAllocated);

    void* res = (void*) &arena->mem[arena->used];
    arena->used += amount;

    return res;
  }

#endif

static int *ddr = (int*) (0x80000000 + (1<<(FIRM_ADDR_W+2)));

void SingleTest(Arena* arena);

int main(int argc,char* argv[]){
  uart_init(UART_BASE,FREQ/BAUD);
  timer_init(TIMER_BASE);
  ila_init(ILA_BASE);
  versat_init(VERSAT_BASE);
  
#ifdef PC
  Arena arenaInst = InitArena(Megabyte(16));
#else
  Arena arenaInst = {};
  arenaInst.mem = (Byte*) ddr;
  arenaInst.totalAllocated = Gigabyte(1);

  printf("DDR\n");
  printf("%p\n",&arenaInst); // Local variable
  printf("%p\n",ddr); // Local variable
#endif
 
  Arena* arena = &arenaInst;

  // Init testing buffers
  expectedBuffer = (TestValue*) PushBytes(arena,TEST_BUFFER_AMOUNT * sizeof(TestValue));
  gotBuffer      = (TestValue*) PushBytes(arena,TEST_BUFFER_AMOUNT * sizeof(TestValue));
  expectedIndex  = 0;
  gotIndex       = 0;
  
  SingleTest(arena);

  bool error = false;
  if(expectedIndex != gotIndex){
    error = true;
    printf("%s: ERROR\n",acceleratorTypeName);
    printf("Number of testing samples differ!\n");
    printf("Expected samples: %d\n",expectedIndex);
    printf("Got samples: %d\n",gotIndex);

    return 0;
  }

  for(int i = 0; i < expectedIndex; i++){
    if(!TestValueEqual(expectedBuffer[i],gotBuffer[i])){
      if(!error){
        error = true;
        printf("%s: ERROR\n",acceleratorTypeName);
      }
        
      printf("Obtained different values at: %d!\n",i);
      printf("Expected: ");
      PrintTestValue(expectedBuffer[i]);
      printf("\n");
      printf("Got:      ");
      PrintTestValue(gotBuffer[i]);
      printf("\n");
    }
  }

  if(!error){
    printf("%s: OK\n",acceleratorTypeName);
  }
  
  uart_finish();
  
  return 0;
}

#endif // INCLUDED_TESTBENCH
