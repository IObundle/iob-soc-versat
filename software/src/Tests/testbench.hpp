#ifndef INCLUDED_TESTBENCH
#define INCLUDED_TESTBENCH

#include "versat_accel.h"

#include <limits>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "iob-uart.h"
#include "printf.h"

#define TEST_PASSED 0
#define TEST_FAILED 1

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define Kilobyte(val) (val * 1024)
#define Megabyte(val) (Kilobyte(val) * 1024)
#define Gigabyte(val) (Megabyte(val) * 1024)

typedef unsigned char Byte;

static bool error = false; // Global keep track if a error occurred. Do not want to print error messages more than once

static const int TEST_BUFFER_AMOUNT = 1000; // 100K should be around 400Kbs per buffer (expected and got)

typedef enum {TEST_INTEGER,TEST_UNSIGNED,TEST_FLOAT} TestValueType;

typedef struct{
  Byte* mem;
  size_t used;
  size_t totalAllocated;
} Arena;

template<typename T>
class ArrayIterator{
public:
   T* ptr;

   inline bool operator!=(const ArrayIterator<T>& iter){return ptr != iter.ptr;};
   inline ArrayIterator<T>& operator++(){++ptr; return *this;};
   inline T& operator*(){return *ptr;};
};

template<typename T>
struct Array{
  T* data;
  int size;

  inline T& operator[](int index) const {assert(index < size); return data[index];}
  ArrayIterator<T> begin(){return ArrayIterator<T>{data};};
  ArrayIterator<T> end(){return ArrayIterator<T>{data + size};};
};

typedef struct {
  union {
    int i;
    unsigned int u;
    float f;
  };
  const char* marker;
  TestValueType type;
} TestValue;

static unsigned int randomSeed = 1;
void SeedRandomNumber(unsigned int val){
  if(val == 0){
    randomSeed = 1;
  } else {
    randomSeed = val;
  }
}

unsigned int GetRandomNumber(){
  // Xorshift
  randomSeed ^= randomSeed << 13;
  randomSeed ^= randomSeed >> 17;
  randomSeed ^= randomSeed << 5;
  return randomSeed;
}

#if 0
int NumberDigitsRepresentation(int number){
  int nDigits = 0;

  if(number == 0){
    return 1;
  }

  int64 num = number;
  if(num < 0){
    nDigits += 1; // minus sign
    num *= -1;
  }

  while(num > 0){
    num /= 10;
    nDigits += 1;
  }

  return nDigits;
}

static char* GetNumberRepr(uint64 number,char* buffer){
  if(number == 0){
    buffer[0] = '0';
    buffer[1] = '\0';
    return buffer;
  }

  uint64 val = number;
  buffer[31] = '\0';
  int index = 30;
  while(val){
    buffer[index] = '0' + (val % 10);
    val /= 10;
    index -= 1;
  }

  return &buffer[index+1];
}

int GetMaxDigitSize(Array<int> array){
  int maxReprSize = 0;
  for(int val : array){
    maxReprSize = std::max(maxReprSize,NumberDigitsRepresentation(val));
  }

  return maxReprSize;
}
#endif

int GetMaxDigitSize(Array<float> array){
  return 2; // Floating points is hard to figure out how many digits. 2 should be enough
}

char GetHexadecimalChar(int value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'a' + (value - 10);
  }
}

char* GetHexadecimal(const char* text, int str_size,char* buffer){
  int i;

  //Assert(str_size * 2 < 64);
  for(i = 0; i < str_size; i++){
    int ch = (int) ((unsigned char) text[i]);

    buffer[64 - 3 - i*2] = GetHexadecimalChar(ch / 16);
    buffer[64 - 3 - i*2+1] = GetHexadecimalChar(ch % 16);
  }

  buffer[64 - 1] = '\0';

  return &buffer[64 - str_size * 2 - 1];
}

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

static TestValue* expectedBuffer = nullptr;
static int expectedIndex;
static TestValue* gotBuffer = nullptr;
static int gotIndex;

static float Abs(float in){
  if(in < 0.0f){
    return -in;
  } else {
    return in;
  }
}

static bool MyFloatEqual(float f0,float f1,float epsilon = 0.00001f){
  if(f0 == f1){
    return true;
  }

  // Values so close to zero that 
  if(Abs(f0) <= epsilon && Abs(f1) <= epsilon){
    return true;
  }

  float norm = Abs(f0) + Abs(f1);
  norm = std::min(norm,std::numeric_limits<float>::max());
  float diff = Abs(f0 - f1);

  bool equal = diff <= norm * epsilon;

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

static TestValue MakeTestValueInt(int i,const char* marker = ""){
  TestValue val = {};
  val.i = i;
  val.type = TEST_INTEGER;
  val.marker = marker;
  return val;
}

static TestValue MakeTestValueUnsigned(unsigned int i,const char* marker = ""){
  TestValue val = {};
  val.i = i;
  val.type = TEST_UNSIGNED;
  val.marker = marker;
  return val;
}

static TestValue MakeTestValueFloat(float f,const char* marker = ""){
  TestValue val = {};
  val.f = f;
  val.type = TEST_FLOAT;
  val.marker = marker;
  return val;
}

static bool TestValueEqual(TestValue v1,TestValue v2){
  if(v1.type != v2.type){
    return false;
  }

  if(strcmp(v1.marker,v2.marker) != 0){
    return false;
  }

  bool res = false;
  switch(v1.type){
  case TEST_INTEGER:{
    res = (v1.i == v2.i);
  }break;
  case TEST_UNSIGNED:{
    res = (v1.u == v2.u);
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
    printf("%d ",val.i);
  }break;
  case TEST_UNSIGNED:{
    printf("%u ",val.u);
  }break;
  case TEST_FLOAT:{
    printf("%f ",val.f);
  }break;
  }
  printf("%s",val.marker);
}

static void PushExpectedI(int val,const char* marker = ""){
  PushExpected(MakeTestValueInt(val,marker));
}

static void PushGotI(int val,const char* marker = ""){
  PushGot(MakeTestValueInt(val,marker));
}

static void PushExpectedU(unsigned int val,const char* marker = ""){
  PushExpected(MakeTestValueUnsigned(val,marker));
}

static void PushGotU(unsigned int val,const char* marker = ""){
  PushGot(MakeTestValueUnsigned(val,marker));
}

static void PushExpectedF(float val,const char* marker = ""){
  PushExpected(MakeTestValueFloat(val,marker));
}

static void PushGotF(float val,const char* marker = ""){
  PushGot(MakeTestValueFloat(val,marker));
}

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

static void Assert_Eq(int val1,int val2,const char* marker = ""){
  PushExpectedI(val1,marker);
  PushGotI(val2,marker);
}

static void Assert_Eq(unsigned int val1,unsigned int val2,const char* marker = ""){
  PushExpectedU(val1,marker);
  PushGotU(val2,marker);
}

static void Assert_Eq(float val1,float val2,const char* marker = ""){
  PushExpectedF(val1,marker);
  PushGotF(val2,marker);
}

Arena InitArena(int amount){
  Arena arena = {};
  arena.mem = (Byte*) malloc(amount);
  arena.totalAllocated = amount;
  return arena;
}

Byte* PushBytes(Arena* arena, size_t size){
   Byte* ptr = &arena->mem[arena->used];

   if(arena->used + size > arena->totalAllocated){
      printf("[%s] Used: %zd, Size: %zd, Total: %zd\n",__PRETTY_FUNCTION__,arena->used,size,arena->totalAllocated);
   }

   //memset(ptr,0,size);
   arena->used += size;

   return ptr;
}

Arena SubArena(Arena* arena,size_t size){
   Byte* mem = PushBytes(arena,size);

   Arena res = {};
   res.mem = mem;
   res.totalAllocated = size;

   return res;
}

Byte* MarkArena(Arena* arena){
   return &arena->mem[arena->used];
}

void PopMark(Arena* arena,Byte* mark){
   arena->used = mark - arena->mem;
}

void SingleTest(Arena* arena);

#include "iob_soc_versat_conf.h"
#include "iob_soc_versat_system.h"
#include "iob_soc_versat_periphs.h"

static int* ddr = (int*) (EXTRA_BASE + (1<<(IOB_SOC_VERSAT_SRAM_ADDR_W + 2)));

extern "C" int RunTest(int versatBase){   
  versat_init(versatBase);

#ifdef PC
  Arena arenaInst = InitArena(Megabyte(16));
#else
  Arena arenaInst = {};
  arenaInst.mem = (Byte*) ddr;
  arenaInst.totalAllocated = Megabyte(16);
#endif

  Arena* arena = &arenaInst;

  // Init testing buffers
  expectedBuffer = (TestValue*) PushBytes(arena,TEST_BUFFER_AMOUNT * sizeof(TestValue));
  gotBuffer      = (TestValue*) PushBytes(arena,TEST_BUFFER_AMOUNT * sizeof(TestValue));
  expectedIndex  = 0;
  gotIndex       = 0;

  SingleTest(arena);

  bool differentIndexes = (expectedIndex != gotIndex);

  int differentValuesCount = 0;
  for(int i = 0; i < expectedIndex; i++){
    if(!TestValueEqual(expectedBuffer[i],gotBuffer[i])){
      differentValuesCount += 1;
    }
  }
  bool differentValues = (differentValuesCount != 0);
  bool noSamples = (expectedIndex == 0 || gotIndex == 0);

  bool error = differentIndexes || differentValues || noSamples;

  if(!error){
    printf("OK (%d samples)\n",gotIndex);
    return TEST_PASSED;
  }

  printf("Error\n");
  if(noSamples){
    printf("Got 0 samples\n");
  } else if(differentIndexes && !differentValues){
    printf("Number of testing samples differ!\n");
    printf("Expected amount: %d\n",expectedIndex);
    printf("Got amount:      %d\n",gotIndex);
  } else if(differentValues){
    printf("Obtained different values for %d cases!\n",differentValuesCount);

    if(differentIndexes){
      printf("The amount of samples also differ\n");
      printf("Its possible that the majority of missmatches occured\n");
      printf("Because samples are not properly matched\n");
      printf("Expected amount: %d\n",expectedIndex);
      printf("Got amount:      %d\n",gotIndex);
    }

    int valuesToShow = std::max(differentValuesCount,10);
    printf("Showcasing the first %d missmatches values\n",valuesToShow);

    for(int i = 0; i < expectedIndex; i++){
      if(!TestValueEqual(expectedBuffer[i],gotBuffer[i])){
        printf("===== Index: %d\n",i);
        printf("Expected: ");
        PrintTestValue(expectedBuffer[i]);
        printf("\n");
        printf("Got:      ");
        PrintTestValue(gotBuffer[i]);
        printf("\n");

        valuesToShow -= 1;
      }

      if(valuesToShow == 0){
        break;
      }
    }
  } else {
    printf("NOT_POSSIBLE");
  }
  
  return TEST_FAILED; // Should not reach this return
}

#endif // INCLUDED_TESTBENCH
