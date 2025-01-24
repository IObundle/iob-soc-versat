#pragma GCC diagnostic ignored "-Wcast-align"

#ifndef INCLUDED_TESTBENCH
#define INCLUDED_TESTBENCH

#include "versat_accel.h"

#include <limits>
#include <algorithm>
#include <cassert>
#include <cstring>

extern "C" {
#include "iob-uart.h"
#include "printf.h"
};

#include "unitConfiguration.hpp"

#define TEST_PASSED 0
#define TEST_FAILED 1

#undef  ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define Kilobyte(val) (val * 1024)
#define Megabyte(val) (Kilobyte(val) * 1024)
#define Gigabyte(val) (Megabyte(val) * 1024)

#define ALIGN_4(val) ((val + 3) & (~0x3))

typedef unsigned char Byte;

static bool error = false; // Global keep track if a error occurred. Do not want to print error messages more than once

typedef struct{
  char* str;
  int size;
} String;

#define STRING(str) (String){str,strlen(str)}

typedef struct{
  Byte* mem;
  int used;
  int totalAllocated;
} Arena;

Arena InitArena(int amount){
  Arena arena = {};
  arena.mem = (Byte*) malloc(amount);
  arena.totalAllocated = amount;
  return arena;
}

Byte* PushBytes(Arena* arena, int size){
   Byte* ptr = &arena->mem[arena->used];

   if(arena->used + size > arena->totalAllocated){
    printf("[%s] Used: %zd, Size: %zd, Total: %zd\n",__PRETTY_FUNCTION__,arena->used,size,arena->totalAllocated);
    return nullptr;
   }

   arena->used += size;

   return ptr;
}

Arena SubArena(Arena* arena,int size){
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

template<typename T>
T* PushType(Arena* arena){
  return (T*) PushBytes(arena,sizeof(T));
}

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

  inline T& operator[](int index) const {if(index >= size){printf("Bad Array access: %d out of %d\n",index,size); exit(0);}; return data[index];}
  ArrayIterator<T> begin(){return ArrayIterator<T>{data};};
  ArrayIterator<T> end(){return ArrayIterator<T>{data + size};};
};

template<typename T>
Array<T> PushArray(Arena* arena,int elements){
  Array<T> res = {};
  res.data = (T*) PushBytes(arena,sizeof(T) * elements);
  res.size = elements;
  return res;
}

template<typename T>
Array<T> PushArray(Arena* arena,Byte* mark){
  Array<T> res = {};
  res.data = (T*) mark;
  res.size = (&arena->mem[arena->used] - mark) / sizeof(T);
  return res;
}

template<typename T>
Array<T> ArrayJoin(Array<T> f,Array<T> s,Arena* out){
   if(f.data > s.data){
      Array<T> temp = f;
      f = s;
      s = temp;
   }

   // Arrays are continouos, concatenate them together
   if(f.data + f.size == s.data){
      Array<T> result = {};
      result.data = f.data;
      result.size = f.size + s.size;
      return result;
   }

   Array<T> result = PushArray<T>(out,f.size + s.size);
   int index = 0;
   for(T& val : f){
      result[index++] = val;
   }
   for(T& val : s){
      result[index++] = val;
   }

   return result;
}

// Even though we only use an assert based approach for now,
// we still keep expected and got separated in the case that we eventually need to implement 
// separated pushExpected pushGot functions.
// Some tests are easier to write if we can separate expected from got generation

enum TestValueType {TestValueType_INTEGER,
                    TestValueType_UNSIGNED,
                    TestValueType_FLOAT,
                    TestValueType_STRING};

struct TestValue{
  int size;
  TestValueType type;
  const char* marker;
};

struct TestValueSimples : public TestValue{
  union{
    int i;
    unsigned int u;
    float f;
  };
};

struct TestValueString : public TestValue{
  char string[];
};

static const int TEST_BUFFER_AMOUNT = Kilobyte(64);

static Arena expectedArena = {};
static Arena gotArena = {};

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

int GetMaxDigitSize(Array<float> array){
  return 2; // Floating points is hard to figure out how many digits. 2 should be enough
}

char GetHexadecimalChar(unsigned char value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'a' + (value - 10);
  }
}

char* GetHexadecimal(const char* text,char* buffer,int str_size){
  int i = 0;
  unsigned char* view = (unsigned char*) text;
  for(; i< str_size; i++){
    buffer[i*2] = GetHexadecimalChar(view[i] / 16);
    buffer[i*2+1] = GetHexadecimalChar(view[i] % 16);
  }

  buffer[i*2] = '\0';

  return buffer;
}

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

static bool TestValueEqual(TestValue* v1,TestValue* v2){
  if(v1->type != v2->type){
    return false;
  }

  if(strcmp(v1->marker,v2->marker) != 0){
    return false;
  }

  TestValueSimples* s1 = (TestValueSimples*) v1;
  TestValueSimples* s2 = (TestValueSimples*) v2;

  bool res = false;
  switch(v1->type){
  case TestValueType_INTEGER:{
    res = (s1->i == s2->i);
  }break;
  case TestValueType_UNSIGNED:{
    res = (s1->u == s2->u);
  }break;
  case TestValueType_FLOAT:{
    res = MyFloatEqual(s1->f,s2->f);
  }break;
  case TestValueType_STRING:{
    TestValueString* S1 = (TestValueString*) v1;
    TestValueString* S2 = (TestValueString*) v2;

    if(S1->size != S2->size){
      return false;
    }

    int stringSize = S1->size - sizeof(TestValue);

    for(int i = 0; i < stringSize; i++){
      if(S1->string[i] != S2->string[i]){
        return false;
      }
      if(S1->string[i] == '\0'){
        break;
      }
    }

    return true;
  } break;
  }

  return res;
}

static void PrintTestValue(TestValue* value){
  TestValueSimples* val = (TestValueSimples*) value;

  switch(value->type){
  case TestValueType_INTEGER:{
    printf("%d ",val->i);
  }break;
  case TestValueType_UNSIGNED:{
    printf("%u ",val->u);
  }break;
  case TestValueType_FLOAT:{
    printf("%f ",val->f);
  }break;
  case TestValueType_STRING:{
    TestValueString* str = (TestValueString*) value;
    printf("%s",str->string);
  }break;
  }
  printf("%s",val->marker);
}

TestValue* PushTestValue(Arena* arena,int val,const char* marker){
  TestValueSimples* s = (TestValueSimples*) PushBytes(arena,sizeof(TestValueSimples));
  s->size = sizeof(TestValueSimples); s->i = val; s->marker = marker; s->type = TestValueType_INTEGER;
  return s;
}

TestValue* PushTestValue(Arena* arena,unsigned int val,const char* marker){
  TestValueSimples* s = (TestValueSimples*) PushBytes(arena,sizeof(TestValueSimples));
  s->size = sizeof(TestValueSimples); s->u = val; s->marker = marker; s->type = TestValueType_UNSIGNED;
  return s;
}

TestValue* PushTestValue(Arena* arena,float val,const char* marker){
  TestValueSimples* s = (TestValueSimples*) PushBytes(arena,sizeof(TestValueSimples));
  s->size = sizeof(TestValueSimples); s->f = val; s->marker = marker; s->type = TestValueType_FLOAT;
  return s;
}

TestValue* PushTestValue(Arena* arena,const char* val,const char* marker){
  int stringSize = strlen(val);
  int totalSize = ALIGN_4(stringSize + sizeof(TestValue) + 1); // + 1 for '\0'

  TestValueString* s = (TestValueString*) PushBytes(arena,totalSize);
  s->size = totalSize; s->marker = marker; s->type = TestValueType_STRING;
  strncpy(s->string,val,stringSize);
  s->string[stringSize] = '\0';
  return s;
}

static void Assert_Eq(int val1,int val2,const char* marker = ""){
  PushTestValue(&expectedArena,val1,marker);
  PushTestValue(&gotArena     ,val2,marker);
}

static void Assert_Eq(unsigned int val1,unsigned int val2,const char* marker = ""){
  PushTestValue(&expectedArena,val1,marker);
  PushTestValue(&gotArena     ,val2,marker);
}

static void Assert_Eq(float val1,float val2,const char* marker = ""){
  PushTestValue(&expectedArena,val1,marker);
  PushTestValue(&gotArena     ,val2,marker);
}

static void Assert_Eq(const char* str1,const char* str2,const char* marker = ""){
  PushTestValue(&expectedArena,str1,marker);
  PushTestValue(&gotArena     ,str2,marker);
}

// ClearCache is very important otherwise sim-run might read past values
static void ClearCache(){
#ifndef PC
  int size = 1024 * 32;
  char* m = (char*) malloc(size); // Should not use malloc but some random fixed ptr in embedded. No use calling malloc since we can always read at any point in memory without worrying about memory protection.

  // volatile and asm are used to make sure that gcc does not optimize away this loop that appears to do nothing
  volatile int val = 0;
  for(int i = 0; i < size; i += 32){
    val += m[i];
    __asm__ volatile("" : "+g" (val) : :);
  }
  free(m);
#endif
}

String PushFile(Arena* arena,const char* filepath){
  char* start = (char*) PushBytes(arena,0);
  uint32_t file_size = uart_recvfile((char*) filepath,start);
  Array<char> testFile = PushArray<char>(arena,file_size + 1);
  testFile[file_size] = '\0';

  return (String){.str = testFile.data,.size = (int) file_size};
}

#if 0
String PushFileFromEthernet(Arena* arena,const char* file_name){
  uart_puts(UART_PROGNAME);
  uart_puts(": requesting to receive file by ethernet\n");

  // send file receive by ethernet request
  uart_putc(0x13);

  // send file name (including end of string)
  uart_puts(file_name);
  uart_putc(0);

  // receive file size
  uint32_t file_size = uart_getc();
  file_size |= ((uint32_t)uart_getc()) << 8;
  file_size |= ((uint32_t)uart_getc()) << 16;
  file_size |= ((uint32_t)uart_getc()) << 24;

  // send ACK before receiving file
  uart_putc(ACK);

  char* testFile = PushArray<char>(arena,file_size + 1);
  eth_rcv_file(testFile,file_size);
  testFile[file_size] = '\0';

  return (String){.str=testFile,.size=file_size};
}
#endif

void SingleTest(Arena* arena);

#include "iob_soc_versat_conf.h"
#include "iob_soc_versat_system.h"
#include "iob_soc_versat_periphs.h"

static int* ddr = (int*) (EXTRA_BASE + (1<<(IOB_SOC_VERSAT_SRAM_ADDR_W + 2)));

extern "C" int RunTest(int versatBase){   
  versat_init(versatBase);

  int memoryReserved = Megabyte(128);

#ifdef PC
  Arena arenaInst = InitArena(memoryReserved);
#else
  Arena arenaInst = {};
  arenaInst.totalAllocated = memoryReserved;
  arenaInst.mem = (Byte*) malloc(arenaInst.totalAllocated);
#endif

  Arena* arena = &arenaInst;
  expectedArena = SubArena(arena,Kilobyte(16));
  gotArena      = SubArena(arena,Kilobyte(16));

  SingleTest(arena);

  bool differentSizes = (expectedArena.used != gotArena.used);

  char* expectedPtr = (char*) expectedArena.mem;
  char* gotPtr      = (char*) gotArena.mem;
  char* expectedEnd = (char*) PushBytes(&expectedArena,0);
  char* gotEnd      = (char*) PushBytes(&gotArena,0);

  int expectedIndex = 0;
  int gotIndex = 0;
  int differentValuesCount = 0;
  while(expectedPtr < expectedEnd || gotPtr < gotEnd){
    TestValue* testExpected = (TestValue*) expectedPtr;
    TestValue* testGot = (TestValue*) gotPtr;

    if(expectedPtr < expectedEnd && gotPtr < gotEnd){
      if(!TestValueEqual(testExpected,testGot)){
        differentValuesCount += 1;
      }

      expectedIndex += 1;
      expectedPtr += testExpected->size;
      
      gotIndex += 1;
      gotPtr += testGot->size;
    } else if(expectedPtr < expectedEnd){
      expectedIndex += 1;
      expectedPtr += testExpected->size;
    } else if(gotPtr < gotEnd){
      gotIndex += 1;
      gotPtr += testGot->size;
    } else {
      printf("Shouldn't reach this testbench.hpp:%d\n",__LINE__);
    }
  }

  bool differentIndexes = (expectedIndex != gotIndex);
  bool differentValues = (differentValuesCount != 0);
  bool error = differentIndexes || differentSizes || differentValues;

  if(!error){
    printf("TEST_RESULT:TEST_PASSED\n");
    printf("OK (%d samples)\n",gotIndex);
    return TEST_PASSED;
  }

  printf("TEST_RESULT:TEST_FAILED\n");
  printf("Error ");
  if(expectedArena.used == 0 || gotArena.used == 0){
    printf("(0 samples)\n");
  } else if(differentIndexes && !differentValues){
    printf("(%d Got/%d Expected)\n",gotIndex,expectedIndex);
  } else if(differentValues){
    printf("(%d Got/%d Expected)\n",gotIndex,expectedIndex);

    printf("Obtained different values for %d cases!\n",differentValuesCount);

    int valuesToShow = std::min(differentValuesCount,10);
    printf("Showcasing the first %d missmatches values\n",valuesToShow);

    char* expectedPtr = (char*) expectedArena.mem;
    char* gotPtr      = (char*) gotArena.mem;
    int i = 0;
    while(expectedPtr < expectedEnd && gotPtr < gotEnd && valuesToShow >= 0){
      TestValue* testExpected = (TestValue*) expectedPtr;
      TestValue* testGot = (TestValue*) gotPtr;

      if(!TestValueEqual(testExpected,testGot)){
        printf("===== Index: %d\n",i);
        printf("Expected: ");
        PrintTestValue(testExpected);
        printf("\n");
        printf("Got:      ");
        PrintTestValue(testGot);
        printf("\n");
        valuesToShow -= 1;
      }
      expectedPtr += testExpected->size;
      gotPtr += testGot->size;

      i += 1;
    }
  } else {
    printf("NOT_POSSIBLE");
  }
  
  return TEST_FAILED;
}

#endif // INCLUDED_TESTBENCH
