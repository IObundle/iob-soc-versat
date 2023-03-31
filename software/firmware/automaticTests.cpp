#include <cstdarg>
#include <cstdio>
#include <cstdint>

#include "versat.hpp"
#include "versatExtra.hpp"
#include "utils.hpp"
#include "unitConfiguration.hpp"
#include "basicWrapper.inc"
#include "verilogWrapper.inc"
#include "versatSHA.hpp"

#include "memory.hpp"

extern "C"{
#include "../test_vectors.h"
#include "crypto/sha2.h"
#include "crypto/mceliece/mceliece348864/api.h"

#include "iob-ila.h"
int printf_(const char* format, ...);
}

#ifndef PC
#define printf printf_
#endif

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"

// Disable accelerator running in PC if doing an hardware test
#if 0
#if defined(HARDWARE_TEST) && defined(PC)
#define AcceleratorRun(...) ((void)0)
#endif
#endif

struct TestInfo{
   int testsPassed;
   int numberTests;

   TestInfo(){}
   TestInfo(int passed, int numberTests = 1):testsPassed(passed),numberTests(numberTests){};

   TestInfo& operator+=(TestInfo t){
      testsPassed += t.testsPassed;
      numberTests += t.numberTests;

      return *this;
   }

   TestInfo operator&(TestInfo t){
      TestInfo info;
      info.testsPassed = this->testsPassed & t.testsPassed;
      info.numberTests = this->numberTests & t.numberTests;

      return info;
   }
};

#define TEST_FAILED(...) do{ printf("\n[%2d]Test failed: %s\n\t",testNumber,__PRETTY_FUNCTION__); \
                                    printf(__VA_ARGS__); \
                                    printf("\n\n"); \
                                    return TestInfo(0);} while(0)
#define TEST_PASSED return TestInfo(1)

#define FORCE_FAIL 0

// Care with the testNumber variable. Every test must have one
#define EXPECT(...) Expect_(__PRETTY_FUNCTION__,testNumber,__VA_ARGS__)
static TestInfo Expect_(const char* functionName,int testNumber, const char* expected,const char* format, ...) __attribute__ ((format (printf, 4, 5)));

static TestInfo Expect_(const char* functionName,int testNumber, const char* expected,const char* format, ...){
   va_list args;
   va_start(args,format);

   char buffer[2048];
   int size = vsprintf(buffer,format,args);
   Assert(size < 2048);

   va_end(args);

   #if FORCE_FAIL
      expected = "";
   #endif

   bool result = (strcmp(expected,buffer) == 0);
   if(result){
      TEST_PASSED;
   } else {
      printf("\n");
      printf("[%2d]Test failed: %s\n",testNumber,functionName);
      printf("    Expected: %s\n",expected);
      printf("    Result:   %s\n",buffer);
      printf("              ");
      for(int i = 0; expected[i] != '\0'; i++){
         if(buffer[i] == '\0'){
            printf("^");
            break;
         }
         if(buffer[i] != expected[i]){
            printf("^");
         } else {
            printf(" ");
         }
      }

      printf("\n");

      return TestInfo(0);
   }
}

#define TEST(TEST_NAME) static TestInfo TEST_NAME(Versat* versat,Arena* temp,int testNumber)

TEST(TestMStage){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"M_Stage");
   FUInstance* inst = GetInstanceByName(test.accel,"Test","comb","sigma");

   #if 1
   int constants[] = {7,18,3,17,19,10};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      inst->config[i] = constants[i];
   }
   #endif

   int* out = RunSimpleAccelerator(&test,0x5a86b737,0xa9f9be83,0x08251f6d,0xeaea8ee9);

   OutputVersatSource(versat,&test,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("0xb89ab4ca","0x%x",out[0]);
}

TEST(TestFStage){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"F_Stage");

   FUInstance* t = GetInstanceByName(test.accel,"Test","f_stage","t");
   int constants[] = {6,11,25,2,13,22};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      t->config[i] = constants[i];
   }

   int* out = RunSimpleAccelerator(&test,0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19,0x428a2f98,0x5a86b737);

   OutputVersatSource(versat,&test,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 8; i++){
      ptr += sprintf(ptr,"0x%08x ",out[i]);
   }

   return EXPECT("0x568f3f84 0x6a09e667 0xbb67ae85 0x3c6ef372 0xf34e99d9 0x510e527f 0x9b05688c 0x1f83d9ab ","%s",buffer);
}

TEST(VReadToVWrite){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"VReadToVWrite");

   Accelerator* accel = test.accel;

   int readBuffer[16];
   int writeBuffer[16];

   FUInstance* reader = GetInstanceByName(accel,"Test","read");
   FUInstance* writer = GetInstanceByName(accel,"Test","write");

   ConfigureSimpleVRead(reader,16,readBuffer);
   ConfigureSimpleVWrite(writer,16,writeBuffer);

   for(int i = 0; i < 16; i++){
      readBuffer[i] = i;
   }

   #if 1
   AcceleratorRun(accel); // Load vread
   AcceleratorRun(accel); // Load vwrite
   AcceleratorRun(accel); // Write data
   #endif

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"%d ",writeBuffer[i]);
   }

   return EXPECT("0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 ","%s",buffer);
}

TEST(StringHasher){
   int weights[] = {17,67,109,157,199};
   char testString[] = "123249819835894981389Waldo198239812849825899904924oefhcasjngwoeijfjvakjndcoiqwj";

   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"StringHasher");

   Accelerator* accel = test.accel;

   FUInstance* muladd = GetInstanceByName(test.accel,"Test","mul1","mul");

   volatile MuladdConfig* conf = (volatile MuladdConfig*) muladd->config;
   conf->opcode = 0;
   conf->iterations = 99;
   conf->period = 1;
   conf->shift = 0;

   for(int i = 0; i < 5; i++){
      test.inst->config[i] = weights[i];
   }

   FUInstance* bytesIn = GetInstanceByName(accel,"Test","bytesIn");
   FUInstance* bytesOut = GetInstanceByName(accel,"Test","bytesOut");

   for(int i = 0; i < 5; i++){
      VersatUnitWrite(bytesIn,i,(int) ("Waldo"[i]));
   }

   ConfigureMemoryLinear(bytesIn,5);
   ConfigureMemoryReceive(bytesOut,1,1);

   AcceleratorRun(accel);

   int hash = VersatUnitRead(bytesOut,0);

   for(size_t i = 0; i < sizeof(testString); i++){
      VersatUnitWrite(bytesIn,i,(int) testString[i]);
   }

   ConfigureMemoryLinear(bytesIn,sizeof(testString));
   ConfigureMemoryReceive(bytesOut,sizeof(testString)-5,1);

   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   for(size_t i = 0; i < sizeof(testString) - 5; i++){
      int val = VersatUnitRead(bytesOut,i);

      if(hash == val){
         return EXPECT("21","%ld",i);
      }
   }

   TEST_FAILED("Hash wasn't equal to anything");
}

TEST(Convolution){
   const int nSTAGE = 5;
   int pixels[25 * nSTAGE], weights[9 * nSTAGE], bias = 0;

   SeedRandomNumber(0);
   for (int j = 0; j < nSTAGE; j++){
      for (int i = 0; i < 25; i++)
      {
         pixels[25 * j + i] = GetRandomNumber() % 50 - 25;
      }

      for (int i = 0; i < 9; i++)
      {
         weights[9 * j + i] = GetRandomNumber() % 10 - 5;
      }

      if(j == 0)
      {
         bias = GetRandomNumber() % 20 - 10;
      }
   }

   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Convolution");

   Accelerator* accel = test.accel;

   //write data in versat mems
   volatile VReadConfig* pixelConfigs[5];
   for (int j = 0; j < nSTAGE; j++){
      FUInstance* pixel = GetInstanceByName(test.accel,"Test","stage%d",j,"pixels");

      ConfigureSimpleVRead(pixel,25,&pixels[25*j]);
      {
         volatile VReadConfig* config = (volatile VReadConfig*) pixel->config;
         pixelConfigs[j] = config;

         // B - versat side
         config->iterB = 3;
         config->incrB = 1;
         config->perB = 3;
         config->dutyB = 3;
         config->shiftB = 5 - 3;
      }

      FUInstance* weight = GetInstanceByName(accel,"Test","stage%d",j,"weights");

      //write 3x3 kernel and bias in mem1
      for(int i = 0; i < 9; i++){
         VersatUnitWrite(weight,i, weights[9*j + i]);
      }

      if(j == 0){
         FUInstance* bia = GetInstanceByName(accel,"Test","bias");
         bia->config[0] = bias;

         {
            ConfigureMemoryLinear(weight,9);
         }

         {
            FUInstance* muladd = GetInstanceByName(accel,"Test","stage0","muladd");
            volatile MuladdConfig* config = (volatile MuladdConfig*) muladd->config;

            config->iterations = 1;
            config->period = 9;
         }
      }
   }

   FUInstance* res = GetInstanceByName(test.accel,"Test","res");

   volatile MemConfig* resConfig = (volatile MemConfig*) res->config;
   resConfig->iterA = 1;
   resConfig->incrA = 1;
   resConfig->perA = 1;
   resConfig->dutyA = 1;
   resConfig->in0_wr = 1;

   AcceleratorRun(accel); // Load vreads with initial good data

   for(int i = 0; i < 3; i++)
   {
      for(int j = 0; j < 3; j++)
      {
         for(int x = 0; x < 5; x++){
               pixelConfigs[x]->startB = i * 5 + j;
         }

         resConfig->startA = i * 3 + j;

         AcceleratorRun(accel);
      }
   }

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
         ptr += sprintf(ptr,"%d ", VersatUnitRead(res,i * 3 + j));
      }
   }

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("-520 -251 -49 -33 -42 303 -221 -100 -149 ","%s",buffer);
}

TEST(MatrixMultiplication){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"MatrixMultiplication");

   Accelerator* accel = test.accel;

   FUInstance* memA = GetInstanceByName(accel,"Test","matA");
   FUInstance* memB = GetInstanceByName(accel,"Test","matB");
   FUInstance* muladd = GetInstanceByName(accel,"Test","ma");

   FUInstance* res = GetInstanceByName(accel,"Test","res");

   int dimensions = 4;
   int size = dimensions * dimensions;

   ConfigureLeftSideMatrix(memA,dimensions);
   ConfigureRightSideMatrix(memB,dimensions);

   for(int i = 0; i < size; i++){
      //memA->externalMemory[i] = i + 1;
      VersatUnitWrite(memA,i,i+1);
      //memB->externalMemory[i] = i + 1;
      VersatUnitWrite(memB,i,i+1);
   }

   volatile MuladdConfig* conf = (volatile MuladdConfig*) muladd->config;

   conf->opcode = 0;
   conf->iterations = size;
   conf->period = dimensions;
   conf->shift = 0;

   ConfigureMemoryReceive(res,size,dimensions);

   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < dimensions; i++){
      for(int j = 0; j < dimensions; j++){
         ptr += sprintf(ptr,"%d ",VersatUnitRead(res,i*dimensions + j));
         //ptr += sprintf(ptr,"%d ",res->externalMemory[i*dimensions + j]);
      }
   }

   return EXPECT("90 100 110 120 202 228 254 280 314 356 398 440 426 484 542 600 ","%s",buffer);
}

TEST(MatrixMultiplicationVRead){
   #define DIM 4
   int matrixA[DIM*DIM];
   int matrixB[DIM*DIM];
   int matrixRes[DIM*DIM];
   volatile int* resPtr = (volatile int*) matrixRes;
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"MatrixMultiplicationVread");

   Accelerator* accel = test.accel;

   FUInstance* memA = GetInstanceByName(accel,"Test","matA");
   FUInstance* memB = GetInstanceByName(accel,"Test","matB");
   FUInstance* muladd = GetInstanceByName(accel,"Test","ma");

   FUInstance* res = GetInstanceByName(accel,"Test","res");

   int dimensions = DIM;
   int size = dimensions * dimensions;

   ConfigureLeftSideMatrixVRead(memA,dimensions);
   ConfigureRightSideMatrixVRead(memB,dimensions);

   {
   volatile VReadConfig* config = (volatile VReadConfig*) memA->config;
   config->ext_addr = (iptr) matrixA;
   }

   {
   volatile VReadConfig* config = (volatile VReadConfig*) memB->config;
   config->ext_addr = (iptr) matrixB;
   }

   for(int i = 0; i < size; i++){
      matrixA[i] = i + 1;
      matrixB[i] = i + 1;
   }

   volatile MuladdConfig* conf = (volatile MuladdConfig*) muladd->config;

   conf->opcode = 0;
   conf->iterations = size;
   conf->period = dimensions;
   conf->shift = 0;

   ConfigureMatrixVWrite(res,size);
   {
   volatile VWriteConfig* config = (volatile VWriteConfig*) res->config;
   config->ext_addr = (iptr) matrixRes;
   }

   AcceleratorRun(accel);
   AcceleratorRun(accel);
   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < dimensions; i++){
      for(int j = 0; j < dimensions; j++){
         ptr += sprintf(ptr,"%d ",resPtr[i*dimensions + j]);
      }
   }

   return EXPECT("90 100 110 120 202 228 254 280 314 356 398 440 426 484 542 600 ","%s",buffer);
}

TEST(VersatAddRoundKey){
   #if 0
   int input[] = {0x04,0xe0,0x48,0x28,
                  0x66,0xcb,0xf8,0x06,
                  0x81,0x19,0xd3,0x26,
                  0xe5,0x9a,0x7a,0x4c};

   int cypher[] ={0xa0,0x88,0x23,0x2a,
                  0xfa,0x54,0xa3,0x6c,
                  0xfe,0x2c,0x39,0x76,
                  0x17,0xb1,0x39,0x05
                  };
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"AddRoundKey");

   int* out = RunSimpleAccelerator(&test,
                              0x04,0xe0,0x48,0x28, // Cypher
                              0x66,0xcb,0xf8,0x06,
                              0x81,0x19,0xd3,0x26,
                              0xe5,0x9a,0x7a,0x4c,
                              0xa0,0x88,0x23,0x2a, // Key
                              0xfa,0x54,0xa3,0x6c,
                              0xfe,0x2c,0x39,0x76,
                              0x17,0xb1,0x39,0x05);

   const char* expected = "0xa4 0x68 0x6b 0x02 0x9c 0x9f 0x5b 0x6a 0x7f 0x35 0xea 0x50 0xf2 0x2b 0x43 0x49 ";

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT(expected,"%s",buffer);
}

TEST(LookupTable){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"LookupTable");

   int addrA = 0;
   int addrB = 4;

   VersatUnitWrite(test.inst,addrA,0xf0);
   VersatUnitWrite(test.inst,addrB,0xf4);

   int* out = RunSimpleAccelerator(&test,addrA,addrB);

   char buffer[1024];
   sprintf(buffer,"0x%02x 0x%02x",out[0],out[1]);

   return EXPECT("0xf0 0xf4","%s",buffer);
}

static const uint8_t sbox[256] = {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

static void FillSBox(FUInstance* inst){
   for(int i = 0; i < 256; i++){
      VersatUnitWrite(inst,i,sbox[i]);
   }
}

static void FillSubBytes(Accelerator* topLevel,FUInstance* inst){
   for(int i = 0; i < 8; i++){
      FillSBox(GetSubInstanceByName(topLevel,inst,"s%d",i));
   }
}

TEST(VersatSubBytes){
   #if 0
   int input[] = {0x19,0xa0,0x9a,0xe9,
                  0x3d,0xf4,0xc6,0xf8,
                  0xe3,0xe2,0x8d,0x48,
                  0xbe,0x2b,0x2a,0x08};

   int expected[] = {0xd4,0xe0,0xb8,0x1e,
                     0x27,0xbf,0xb4,0x41,
                     0x11,0x98,0x5d,0x52,
                     0xae,0xf1,0xe5,0x30};
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SBox");

   #if 1
   FillSubBytes(test.accel,test.inst);
   #endif

   int* out = RunSimpleAccelerator(&test,0x19,0xa0,0x9a,0xe9,0x3d,0xf4,0xc6,0xf8,0xe3,0xe2,0x8d,0x48,0xbe,0x2b,0x2a,0x08);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xd4 0xe0 0xb8 0x1e 0x27 0xbf 0xb4 0x41 0x11 0x98 0x5d 0x52 0xae 0xf1 0xe5 0x30 ","%s",buffer);
}

TEST(VersatShiftRows){
   #if 0
   int input[] = {0xd4,0xe0,0xb8,0x1e,
                  0x27,0xbf,0xb4,0x41,
                  0x11,0x98,0x5d,0x52,
                  0xae,0xf1,0xe5,0x30};
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ShiftRows");

   int* out = RunSimpleAccelerator(&test,
                  0xd4,0xe0,0xb8,0x1e,
                  0x27,0xbf,0xb4,0x41,
                  0x11,0x98,0x5d,0x52,
                  0xae,0xf1,0xe5,0x30);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xd4 0xe0 0xb8 0x1e 0xbf 0xb4 0x41 0x27 0x5d 0x52 0x11 0x98 0x30 0xae 0xf1 0xe5 ","%s",buffer);
}

static const uint8_t mul2[] = {
   0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
   0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
   0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
   0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
   0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
   0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
   0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
   0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
   0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,
   0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,
   0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,
   0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,
   0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,
   0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,
   0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,
   0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5
};

static const uint8_t mul3[] = {
   0x00,0x03,0x06,0x05,0x0c,0x0f,0x0a,0x09,0x18,0x1b,0x1e,0x1d,0x14,0x17,0x12,0x11,
   0x30,0x33,0x36,0x35,0x3c,0x3f,0x3a,0x39,0x28,0x2b,0x2e,0x2d,0x24,0x27,0x22,0x21,
   0x60,0x63,0x66,0x65,0x6c,0x6f,0x6a,0x69,0x78,0x7b,0x7e,0x7d,0x74,0x77,0x72,0x71,
   0x50,0x53,0x56,0x55,0x5c,0x5f,0x5a,0x59,0x48,0x4b,0x4e,0x4d,0x44,0x47,0x42,0x41,
   0xc0,0xc3,0xc6,0xc5,0xcc,0xcf,0xca,0xc9,0xd8,0xdb,0xde,0xdd,0xd4,0xd7,0xd2,0xd1,
   0xf0,0xf3,0xf6,0xf5,0xfc,0xff,0xfa,0xf9,0xe8,0xeb,0xee,0xed,0xe4,0xe7,0xe2,0xe1,
   0xa0,0xa3,0xa6,0xa5,0xac,0xaf,0xaa,0xa9,0xb8,0xbb,0xbe,0xbd,0xb4,0xb7,0xb2,0xb1,
   0x90,0x93,0x96,0x95,0x9c,0x9f,0x9a,0x99,0x88,0x8b,0x8e,0x8d,0x84,0x87,0x82,0x81,
   0x9b,0x98,0x9d,0x9e,0x97,0x94,0x91,0x92,0x83,0x80,0x85,0x86,0x8f,0x8c,0x89,0x8a,
   0xab,0xa8,0xad,0xae,0xa7,0xa4,0xa1,0xa2,0xb3,0xb0,0xb5,0xb6,0xbf,0xbc,0xb9,0xba,
   0xfb,0xf8,0xfd,0xfe,0xf7,0xf4,0xf1,0xf2,0xe3,0xe0,0xe5,0xe6,0xef,0xec,0xe9,0xea,
   0xcb,0xc8,0xcd,0xce,0xc7,0xc4,0xc1,0xc2,0xd3,0xd0,0xd5,0xd6,0xdf,0xdc,0xd9,0xda,
   0x5b,0x58,0x5d,0x5e,0x57,0x54,0x51,0x52,0x43,0x40,0x45,0x46,0x4f,0x4c,0x49,0x4a,
   0x6b,0x68,0x6d,0x6e,0x67,0x64,0x61,0x62,0x73,0x70,0x75,0x76,0x7f,0x7c,0x79,0x7a,
   0x3b,0x38,0x3d,0x3e,0x37,0x34,0x31,0x32,0x23,0x20,0x25,0x26,0x2f,0x2c,0x29,0x2a,
   0x0b,0x08,0x0d,0x0e,0x07,0x04,0x01,0x02,0x13,0x10,0x15,0x16,0x1f,0x1c,0x19,0x1a
};

static void FillRow(Accelerator* topLevel,FUInstance* row){
   FUInstance* mul2_0 = GetSubInstanceByName(topLevel,row,"mul2_0");
   FUInstance* mul2_1 = GetSubInstanceByName(topLevel,row,"mul2_1");
   FUInstance* mul3_0 = GetSubInstanceByName(topLevel,row,"mul3_0");
   FUInstance* mul3_1 = GetSubInstanceByName(topLevel,row,"mul3_1");

   for(int i = 0; i < 256; i++){
      VersatUnitWrite(mul2_0,i,mul2[i]);
      VersatUnitWrite(mul2_1,i,mul2[i]);
      VersatUnitWrite(mul3_0,i,mul3[i]);
      VersatUnitWrite(mul3_1,i,mul3[i]);
   }
}

TEST(VersatDoRows){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"DoRow");

   FillRow(test.accel,test.inst);

   int* out = RunSimpleAccelerator(&test,0xdb,0x13,0x53,0x45);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 4; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x8e 0x4d 0xa1 0xbc ","%s",buffer);
}

TEST(VersatMixColumns){
   #if 0
   int input[] = {0xd4,0xe0,0xb8,0x1e,
                  0xbf,0xb4,0x41,0x27,
                  0x5d,0x52,0x11,0x98,
                  0x30,0xae,0xf1,0xe5};

   int expected[] = {0x04,0xe0,0x48,0x28,
                     0x66,0xcb,0xf8,0x06,
                     0x81,0x19,0xd3,0x26,
                     0xe5,0x9a,0x7a,0x4c};
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"MixColumns");

   for(int i = 0; i < 4; i++){
      FillRow(test.accel,GetInstanceByName(test.accel,"Test","d%d",i));
   }

   int* out = RunSimpleAccelerator(&test,0xd4,0xe0,0xb8,0x1e,0xbf,0xb4,0x41,0x27,0x5d,0x52,0x11,0x98,0x30,0xae,0xf1,0xe5);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x04 0xe0 0x48 0x28 0x66 0xcb 0xf8 0x06 0x81 0x19 0xd3 0x26 0xe5 0x9a 0x7a 0x4c ","%s",buffer);
}

TEST(FirstLineKey){
   #if 0
   int input[] = {0x09,0xcf,0x4f,0x3c,0x2b,0x7e,0x15,0x16,0x01};
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FirstLineKey");

   for(int i = 0; i < 2; i++){
      FUInstance* table = GetInstanceByName(test.accel,"Test","b%d",i);

      for(int ii = 0; ii < 256; ii++){
         VersatUnitWrite(table,ii,sbox[ii]);
      }
   }

   int* out = RunSimpleAccelerator(&test,0x09,0xcf,0x4f,0x3c,0x2b,0x7e,0x15,0x16,0x01);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 4; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xa0 0xfa 0xfe 0x17 ","%s",buffer);
}

static void FillKeySchedule(Accelerator* topLevel,FUInstance* inst){
   for(int i = 0; i < 2; i++){
      FUInstance* table = GetSubInstanceByName(topLevel,inst,"s","b%d",i);

      FillSBox(table);
   }
}

TEST(KeySchedule){
   #if 0
   int input[] = {0x2b,0x28,0xab,0x09,
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c,
                  0x01}; // rcon
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"KeySchedule");

   FillKeySchedule(test.accel,test.inst);

   int* out = RunSimpleAccelerator(&test,0x2b,0x28,0xab,0x09,0x7e,0xae,0xf7,0xcf,0x15,0xd2,0x15,0x4f,0x16,0xa6,0x88,0x3c,0x01);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xa0 0x88 0x23 0x2a 0xfa 0x54 0xa3 0x6c 0xfe 0x2c 0x39 0x76 0x17 0xb1 0x39 0x05 ","%s",buffer);
}

void FillRound(Accelerator* topLevel,FUInstance* round){
   for(int i = 0; i < 8; i++){
      FillSBox(GetSubInstanceByName(topLevel,round,"subBytes","s%d",i));
   }

   for(int i = 0; i < 4; i++){
      FillRow(topLevel,GetSubInstanceByName(topLevel,round,"mixColumns","d%d",i));
   }
}

TEST(AESRound){
   #if 0
   int input[] = {0x19,0xa0,0x9a,0xe9, // cypher
                  0x3d,0xf4,0xc6,0xf8,
                  0xe3,0xe2,0x8d,0x48,
                  0xbe,0x2b,0x2a,0x08,
                  0xa0,0x88,0x23,0x2a, // key
                  0xfa,0x54,0xa3,0x6c,
                  0xfe,0x2c,0x39,0x76,
                  0x17,0xb1,0x39,0x05};
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"MainRound");

   FillRound(test.accel,test.inst);

   int* out = RunSimpleAccelerator(&test,0x19,0xa0,0x9a,0xe9,0x3d,0xf4,0xc6,0xf8,0xe3,0xe2,0x8d,0x48,0xbe,0x2b,0x2a,0x08,0xa0,0x88,0x23,0x2a,0xfa,0x54,0xa3,0x6c,0xfe,0x2c,0x39,0x76,0x17,0xb1,0x39,0x05);

   OutputVersatSource(versat,&test,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0xa4 0x68 0x6b 0x02 0x9c 0x9f 0x5b 0x6a 0x7f 0x35 0xea 0x50 0xf2 0x2b 0x43 0x49 ","%s",buffer);
}

static void FillAES(Accelerator* topLevel,FUInstance* inst){
   int rcon[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};
   for(int i = 0; i < 10; i++){
      FUInstance* constRcon = GetSubInstanceByName(topLevel,inst,"rcon%d",i);
      constRcon->config[0] = rcon[i];

      FillKeySchedule(topLevel,GetSubInstanceByName(topLevel,inst,"key%d",i));
   }
   FillSubBytes(topLevel,GetSubInstanceByName(topLevel,inst,"subBytes"));

   for(int i = 0; i < 9; i++){
      FillRound(topLevel,GetSubInstanceByName(topLevel,inst,"round%d",i));
   }
}

static void FillAESAccelerator(Accelerator* accel){
   int rcon[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};
   for(int i = 0; i < 10; i++){
      FUInstance* constRcon = GetInstanceByName(accel,"Test","rcon%d",i);
      constRcon->config[0] = rcon[i];

      for(int j = 0; j < 2; j++){
         FUInstance* table = GetInstanceByName(accel,"Test","key%d",i,"s","b%d",j);

         FillSBox(table);
      }
   }
   for(int i = 0; i < 8; i++){
      FillSBox(GetInstanceByName(accel,"Test","subBytes","s%d",i));
   }

   for(int i = 0; i < 9; i++){
      for(int j = 0; j < 8; j++){
         FillSBox(GetInstanceByName(accel,"Test","round%d",i,"subBytes","s%d",j));
      }

      for(int j = 0; j < 4; j++){
         FUInstance* mul2_0 = GetInstanceByName(accel,"Test","round%d",i,"mixColumns","d%d",j,"mul2_0");
         FUInstance* mul2_1 = GetInstanceByName(accel,"Test","round%d",i,"mixColumns","d%d",j,"mul2_1");
         FUInstance* mul3_0 = GetInstanceByName(accel,"Test","round%d",i,"mixColumns","d%d",j,"mul3_0");
         FUInstance* mul3_1 = GetInstanceByName(accel,"Test","round%d",i,"mixColumns","d%d",j,"mul3_1");

         for(int i = 0; i < 256; i++){
            VersatUnitWrite(mul2_0,i,mul2[i]);
            VersatUnitWrite(mul2_1,i,mul2[i]);
            VersatUnitWrite(mul3_0,i,mul3[i]);
            VersatUnitWrite(mul3_1,i,mul3[i]);
         }
      }
   }
}

static void FillAESVReadAccelerator(Accelerator* accel){
   int rcon[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};
   for(int i = 0; i < 10; i++){
      FUInstance* constRcon = GetInstanceByName(accel,"Test","aes","rcon%d",i);
      constRcon->config[0] = rcon[i];

      for(int j = 0; j < 2; j++){
         FUInstance* table = GetInstanceByName(accel,"Test","aes","key%d",i,"s","b%d",j);

         FillSBox(table);
      }
   }
   for(int i = 0; i < 8; i++){
      FillSBox(GetInstanceByName(accel,"Test","aes","subBytes","s%d",i));
   }

   for(int i = 0; i < 9; i++){
      for(int j = 0; j < 8; j++){
         FillSBox(GetInstanceByName(accel,"Test","aes","round%d",i,"subBytes","s%d",j));
      }

      for(int j = 0; j < 4; j++){
         FUInstance* mul2_0 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul2_0");
         FUInstance* mul2_1 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul2_1");
         FUInstance* mul3_0 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul3_0");
         FUInstance* mul3_1 = GetInstanceByName(accel,"Test","aes","round%d",i,"mixColumns","d%d",j,"mul3_1");

         for(int i = 0; i < 256; i++){
            VersatUnitWrite(mul2_0,i,mul2[i]);
            VersatUnitWrite(mul2_1,i,mul2[i]);
            VersatUnitWrite(mul3_0,i,mul3[i]);
            VersatUnitWrite(mul3_1,i,mul3[i]);
         }
      }
   }
}

TEST(AES){
   #if 0
   int input[] = {0x32,0x88,0x31,0xe0, // cypher
                  0x43,0x5a,0x31,0x37,
                  0xf6,0x30,0x98,0x07,
                  0xa8,0x8d,0xa2,0x34,
                  0x2b,0x28,0xab,0x09, // key
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c};
   #endif
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"AES");

   FillAES(test.accel,test.inst);

   int* out = RunSimpleAccelerator(&test,0x32,0x88,0x31,0xe0,0x43,0x5a,0x31,0x37,0xf6,0x30,0x98,0x07,0xa8,0x8d,0xa2,0x34,0x2b,0x28,0xab,0x09,0x7e,0xae,0xf7,0xcf,0x15,0xd2,0x15,0x4f,0x16,0xa6,0x88,0x3c);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

#include <cstdlib>

TEST(ReadWriteAES){
   int cypher[] = {0x32,0x88,0x31,0xe0,
                  0x43,0x5a,0x31,0x37,
                  0xf6,0x30,0x98,0x07,
                  0xa8,0x8d,0xa2,0x34};
   int key[] =    {0x2b,0x28,0xab,0x09,
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c};
   int result[16];
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ReadWriteAES");

   Accelerator* accel = test.accel;

   ConfigureSimpleVRead(GetInstanceByName(accel,"Test","cypher"),16,cypher);
   ConfigureSimpleVRead(GetInstanceByName(accel,"Test","key"),16,key);
   ConfigureSimpleVWrite(GetInstanceByName(accel,"Test","results"),16,result);

   FillAES(accel,GetInstanceByName(accel,"Test","aes"));

   AcceleratorRun(accel);
   AcceleratorRun(accel);
   AcceleratorRun(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",result[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

int SimpleAdderInstance(Accelerator* accel,int a,int b){
   FUInstance* ab = GetInstanceByName(accel,"Test","a");
   FUInstance* a1 = GetInstanceByName(accel,"Test","a1");
   FUInstance* a2 = GetInstanceByName(accel,"Test","a2");
   FUInstance* out = GetInstanceByName(accel,"Test","res");

   a1->config[0] = a;
   a2->config[0] = b;

   AcceleratorRun(accel);

   int result = out->state[0];

   return result;
}

TEST(SimpleAdder){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SimpleAdder");

   int result = SimpleAdderInstance(test.accel,3,4);

   return EXPECT("7","%d",result);
}

int ComplexAdderInstance(Accelerator* accel,int a,int b){
   FUInstance* b1 = GetInstanceByName(accel,"Test","b1");
   FUInstance* b2 = GetInstanceByName(accel,"Test","b2");
   FUInstance* out = GetInstanceByName(accel,"Test","memOut1");

   VersatUnitWrite(b1,0,a);
   VersatUnitWrite(b2,0,b);

   ConfigureMemoryReceive(out,1,1);

   AcceleratorRun(accel);

   int result = VersatUnitRead(out,0);

   return result;
}

int ComplexMultiplierInstance(Accelerator* accel,int a,int b){
   FUInstance* c1 = GetInstanceByName(accel,"Test","c1");
   FUInstance* c2 = GetInstanceByName(accel,"Test","c2");
   FUInstance* out = GetInstanceByName(accel,"Test","memOut2");

   VersatUnitWrite(c1,0,a);
   VersatUnitWrite(c2,0,b);

   ConfigureMemoryReceive(out,1,1);

   AcceleratorRun(accel);

   int result = VersatUnitRead(out,0);

   return result;
}

int SemiComplexAdderInstance(Accelerator* accel,int a,int b){
   FUInstance* d1 = GetInstanceByName(accel,"Test","d1");
   FUInstance* d2 = GetInstanceByName(accel,"Test","d2");
   FUInstance* out = GetInstanceByName(accel,"Test","memOut3");

   d1->config[0] = a;
   VersatUnitWrite(d2,0,b);

   ConfigureMemoryReceive(out,1,1);

   AcceleratorRun(accel);

   int result = VersatUnitRead(out,0);

   return result;
}

TEST(ComplexMultiplier){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ComplexMultiplier");

   int result = ComplexMultiplierInstance(test.accel,4,5);

   return EXPECT("20","%d",result);
}

TEST(Generator){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"TestGenerator");

   FUInstance* gen = GetSubInstanceByName(test.accel,test.inst,"gen");
   FUInstance* mem = GetSubInstanceByName(test.accel,test.inst,"mem");

   ConfigureGenerator(gen,5,10,1);
   ConfigureMemoryReceive(mem,5,1);

   AcceleratorRun(test.accel);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 6; i++){
      ptr += sprintf(ptr,"%d ",VersatUnitRead(mem,i));
   }

   return EXPECT("5 6 7 8 9 0 ","%s",buffer);
}

TEST(Test64Bits){
   TEST_PASSED;
}

TEST(SimpleShareConfig){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SimpleShareConfig");
   Accelerator* accel = test.accel;

   FUInstance* a1 = GetInstanceByName(accel,"Test","a1");
   FUInstance* a2 = GetInstanceByName(accel,"Test","a2");
   FUInstance* b1 = GetInstanceByName(accel,"Test","b1");
   FUInstance* b2 = GetInstanceByName(accel,"Test","b2");
   FUInstance* out0 = GetInstanceByName(accel,"Test","out0");
   FUInstance* out1 = GetInstanceByName(accel,"Test","out1");
   FUInstance* out2 = GetInstanceByName(accel,"Test","out2");

   a1->config[0] = 2;
   AcceleratorRun(accel);
   int res0 = out0->state[0];

   a1->config[0] = 0;
   a2->config[0] = 3;
   AcceleratorRun(accel);
   int res1 = out0->state[0];

   b2->config[0] = 4;
   AcceleratorRun(accel);
   int res2 = out1->state[0];

   a1->config[0] = 0;
   a2->config[0] = 0;
   b1->config[0] = 0;
   b2->config[0] = 0;

   a2->config[0] = 3;
   b2->config[0] = 4;
   AcceleratorRun(accel);
   int res3 = out2->state[0];

   return EXPECT("4 6 8 7","%d %d %d %d",res0,res1,res2,res3);
}

TEST(ComplexShareConfig){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ComplexShareConfig");
   Accelerator* accel = test.accel;

   // Test by changing config for shared 1
   FUInstance* a11 = GetInstanceByName(accel,"Test","shared1","a1");
   FUInstance* a12 = GetInstanceByName(accel,"Test","shared1","a2");
   FUInstance* b11 = GetInstanceByName(accel,"Test","shared1","b1");
   FUInstance* b12 = GetInstanceByName(accel,"Test","shared1","b2");

   // But reading the output of shared 2 (should be the same, since same configuration = same results)
   FUInstance* out20 = GetInstanceByName(accel,"Test","shared2","out0");
   FUInstance* out21 = GetInstanceByName(accel,"Test","shared2","out1");
   FUInstance* out22 = GetInstanceByName(accel,"Test","shared2","out2");

   a11->config[0] = 2;
   AcceleratorRun(accel);
   int res0 = out20->state[0];

   a11->config[0] = 0;
   a12->config[0] = 3;
   AcceleratorRun(accel);
   int res1 = out20->state[0];

   b12->config[0] = 4;
   AcceleratorRun(accel);
   int res2 = out21->state[0];

   a11->config[0] = 0;
   a12->config[0] = 0;
   b11->config[0] = 0;
   b12->config[0] = 0;

   a12->config[0] = 3;
   b12->config[0] = 4;
   AcceleratorRun(accel);
   int res3 = out22->state[0];

   return EXPECT("4 6 8 7","%d %d %d %d",res0,res1,res2,res3);
}

TEST(SimpleStaticConfig){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SimpleStatic");

   FUInstance* c1 = GetInstanceByName(test.accel,"Test","c1","var");
   c1->config[0] = 0;

   FUInstance* c0 = GetInstanceByName(test.accel,"Test","c0","var");
   c0->config[0] = 5;

   AcceleratorRun(test.accel);
   int* out = RunSimpleAccelerator(&test);

   return EXPECT("5","%d",*out);
}

TEST(ComplexStaticConfig){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ComplexStatic");

   FUInstance* c1 = GetInstanceByName(test.accel,"Test","s0","c1","var");
   c1->config[0] = 0;

   FUInstance* c0 = GetInstanceByName(test.accel,"Test","s1","c0","var");
   c0->config[0] = 5;

   AcceleratorRun(test.accel);
   int* out = RunSimpleAccelerator(&test);

   return EXPECT("5","%d",*out);
}

TEST(SimpleFlatten){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SimpleAdder");

   Accelerator* flatten = Flatten(versat,test.accel,1);

   int result = SimpleAdderInstance(flatten,4,5);

   return EXPECT("9","%d",result);
}

TEST(FlattenShareConfig){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ComplexShareConfig");

   Accelerator* flatten = Flatten(versat,test.accel,99);

   // Test by changing config for shared 1
   FUInstance* a11 = GetInstanceByName(flatten,"Test","shared1","a1");
   FUInstance* a12 = GetInstanceByName(flatten,"Test","shared1","a2");
   FUInstance* b11 = GetInstanceByName(flatten,"Test","shared1","b1");
   FUInstance* b12 = GetInstanceByName(flatten,"Test","shared1","b2");

   // But reading the output of shared 2 (should be the same, since same configuration = same results)
   FUInstance* out20 = GetInstanceByName(flatten,"Test","shared2","out0");
   FUInstance* out21 = GetInstanceByName(flatten,"Test","shared2","out1");
   FUInstance* out22 = GetInstanceByName(flatten,"Test","shared2","out2");

   a11->config[0] = 2;
   AcceleratorRun(flatten);
   int res0 = out20->state[0];

   a11->config[0] = 0;
   a12->config[0] = 3;
   AcceleratorRun(flatten);
   int res1 = out20->state[0];

   b12->config[0] = 4;
   AcceleratorRun(flatten);
   int res2 = out21->state[0];

   a11->config[0] = 0;
   a12->config[0] = 0;
   b11->config[0] = 0;
   b12->config[0] = 0;

   a12->config[0] = 3;
   b12->config[0] = 4;
   AcceleratorRun(flatten);
   int res3 = out22->state[0];

   return EXPECT("4 6 8 7","%d %d %d %d",res0,res1,res2,res3);
}

TEST(FlattenSHA){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SHA");

   Accelerator* flatten = Flatten(versat,test.accel,99);

   SetSHAAccelerator(flatten,nullptr);

   InitVersatSHA(versat,true);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   VersatSHA(digest,msg_64,64);

   return EXPECT("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa","%s",GetHexadecimal(digest, HASH_SIZE));
}

TEST(FlattenAES){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"AES");

   test.accel = Flatten(versat,test.accel,99);

   FillAESAccelerator(test.accel);

   RemapSimpleAccelerator(&test,versat);

   int* out = RunSimpleAccelerator(&test,0x32,0x88,0x31,0xe0,0x43,0x5a,0x31,0x37,0xf6,0x30,0x98,0x07,0xa8,0x8d,0xa2,0x34,0x2b,0x28,0xab,0x09,0x7e,0xae,0xf7,0xcf,0x15,0xd2,0x15,0x4f,0x16,0xa6,0x88,0x3c);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

TEST(FlattenAESVRead){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ReadWriteAES");

   test.accel = Flatten(versat,test.accel,99);
   RemapSimpleAccelerator(&test,versat);

   int cypher[] = {0x32,0x88,0x31,0xe0,
                  0x43,0x5a,0x31,0x37,
                  0xf6,0x30,0x98,0x07,
                  0xa8,0x8d,0xa2,0x34};
   int key[] =    {0x2b,0x28,0xab,0x09,
                  0x7e,0xae,0xf7,0xcf,
                  0x15,0xd2,0x15,0x4f,
                  0x16,0xa6,0x88,0x3c};
   int result[16];

   ConfigureSimpleVRead(GetInstanceByName(test.accel,"Test","cypher"),16,cypher);
   ConfigureSimpleVRead(GetInstanceByName(test.accel,"Test","key"),16,key);
   ConfigureSimpleVWrite(GetInstanceByName(test.accel,"Test","results"),16,result);

   FillAESVReadAccelerator(test.accel);

   AcceleratorRun(test.accel);
   AcceleratorRun(test.accel);
   AcceleratorRun(test.accel);

   OutputVersatSource(versat,test.accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",result[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);
}

TEST(SimpleMergeNoCommon){
   FUDeclaration* types[2];
   types[0] = GetTypeByName(versat,STRING("SimpleAdder"));
   types[1] = GetTypeByName(versat,STRING("ComplexMultiplier"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("NoCommonMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,STRING("Test"));

   int resA = 0;
   int resB = 0;

   resA = SimpleAdderInstance(accel,3,4);
   resB = ComplexMultiplierInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("7 6","%d %d",resA,resB);
}

TEST(SimpleMergeUnitCommonNoEdge){
   FUDeclaration* types[2];
   types[0] = GetTypeByName(versat,STRING("SimpleAdder"));
   types[1] = GetTypeByName(versat,STRING("ComplexAdder"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("UnitCommonNoEdgeMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,STRING("Test"));

   int resA = 0;
   int resB = 0;

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,types[0]);
   resA = SimpleAdderInstance(accel,4,5);

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,types[1]);
   resB = ComplexAdderInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("9 5","%d %d",resA,resB);
}

TEST(SimpleMergeUnitAndEdgeCommon){
   FUDeclaration* types[2];
   types[0] = GetTypeByName(versat,STRING("SimpleAdder"));
   types[1] = GetTypeByName(versat,STRING("SemiComplexAdder"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("UnitAndEdgeCommonMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,STRING("Test"));

   int resA = 0;
   int resB = 0;

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,types[0]);
   resA = SimpleAdderInstance(accel,4,5);

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,types[1]);
   resB = SemiComplexAdderInstance(accel,2,3);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("9 5","%d %d",resA,resB);
}

TEST(SimpleMergeInputOutputCommon){
   FUDeclaration* types[2];
   types[0] = GetTypeByName(versat,STRING("ComplexAdder"));
   types[1] = GetTypeByName(versat,STRING("ComplexMultiplier"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("InputOutputCommonMerged"));

   Accelerator* accel = CreateAccelerator(versat);
   FUInstance* inst = CreateFUInstance(accel,merged,STRING("Test"));

   int resA = 0;
   int resB = 0;

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,types[0]);
   resA = ComplexAdderInstance(accel,4,5);
   //DisplayUnitConfiguration(accel);

   ClearConfigurations(accel);
   ActivateMergedAccelerator(versat,accel,types[1]);
   resB = ComplexMultiplierInstance(accel,2,3);
   //DisplayUnitConfiguration(accel);

   OutputVersatSource(versat,accel,"versat_instance.v","versat_defs.vh","versat_data.inc");

   return EXPECT("9 6","%d %d",resA,resB);
}

TEST(CombinatorialMerge){
   FUDeclaration* types[2];
   types[0] = GetTypeByName(versat,STRING("CH"));
   types[1] = GetTypeByName(versat,STRING("Maj"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("CH_Maj"));

   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"CH_Maj");

   // Test CH(a,b,c)  - a & b ^ (~a & c) - a = b1100, b = b1010, c = b0101 - result 1000 ^ 0001 = 1001 = 9
   ActivateMergedAccelerator(versat,test.accel,types[0]);
   int* out = RunSimpleAccelerator(&test,0xc,0xa,0x5);
   TestInfo subtest = EXPECT("9","%x",*out);

   // Test Maj(a,b,c) - (x & y) ^ (x & z) ^ (y & z) - x = b1111, y = b1010, z = b0101 / a1 = x & y = y, a2 = x & z = z, a3 = y & z = 0 / y ^ z = 0xf
   ActivateMergedAccelerator(versat,test.accel,types[1]);
   out = RunSimpleAccelerator(&test,0xf,0xa,0x5);
   subtest = EXPECT("f","%x",*out);

   return subtest;
}

TEST(SimpleThreeMerge){
   FUDeclaration* types[3];
   types[0] = GetTypeByName(versat,STRING("CH"));
   types[1] = GetTypeByName(versat,STRING("Maj"));
   types[2] = GetTypeByName(versat,STRING("MixProduct"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("Merge3"));

   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Merge3");

   ActivateMergedAccelerator(versat,test.accel,types[0]);
   int* out = RunSimpleAccelerator(&test,0xc,0xa,0x5,0x0);
   TestInfo subtest = EXPECT("9","%x",*out);

   ClearConfigurations(test.accel);
   ActivateMergedAccelerator(versat,test.accel,types[1]);
   out = RunSimpleAccelerator(&test,0xf,0xa,0x5,0x0);
   subtest = EXPECT("f","%x",*out);

   ClearConfigurations(test.accel);
   ActivateMergedAccelerator(versat,test.accel,types[2]);
   out = RunSimpleAccelerator(&test,0xf,0xa,0x5,0xf);
   subtest = EXPECT("f","%x",*out);

   return subtest;
}

TEST(ComplexMerge){
   FUDeclaration* types[2];
   types[0] = GetTypeByName(versat,STRING("SHA"));
   types[1] = GetTypeByName(versat,STRING("AES"));

   Array<FUDeclaration*> typeArray = C_ARRAY_TO_ARRAY(types);

   FUDeclaration* merged = Merge(versat,typeArray,STRING("SHA_AES"),MergingStrategy::SIMPLE_COMBINATION);

   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SHA_AES");

   ActivateMergedAccelerator(versat,test.accel,types[0]);

   #if 0
   SetSHAAccelerator(accel,inst);

   InitVersatSHA(versat,true);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   VersatSHA(digest,msg_64,64);

   return EXPECT("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa","%s",GetHexadecimal(digest, HASH_SIZE));
   #endif

   ActivateMergedAccelerator(versat,test.accel,types[1]);
   FillAESAccelerator(test.accel);

   int* out = RunSimpleAccelerator(&test,0x32,0x88,0x31,0xe0,0x43,0x5a,0x31,0x37,0xf6,0x30,0x98,0x07,0xa8,0x8d,0xa2,0x34,0x2b,0x28,0xab,0x09,0x7e,0xae,0xf7,0xcf,0x15,0xd2,0x15,0x4f,0x16,0xa6,0x88,0x3c);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 16; i++){
      ptr += sprintf(ptr,"0x%02x ",out[i]);
   }

   return EXPECT("0x39 0x02 0xdc 0x19 0x25 0xdc 0x11 0x6a 0x84 0x09 0x85 0x0b 0x1d 0xfb 0x97 0x32 ","%s",buffer);

}

TEST(TestSpecificMerge){
   FUDeclaration* type = GetTypeByName(versat,STRING("TestSpecificMerge"));

   SpecificMerge specific = {};
   specific.instA = STRING("input");
   specific.instB = STRING("output");

   FUDeclaration* merged = MergeAccelerators(versat,type,type,STRING("SpecificMerge"),0,MergingStrategy::CONSOLIDATION_GRAPH,&specific,1);

   TEST_PASSED;
}

static FUDeclaration* TestMerge(Versat* versat,const char* first, const char* second, const char* name,MergingStrategy strategy,Arena* arena){
   printf("%s %s\n",first,second);
   FUDeclaration* typeA = GetTypeByName(versat,STRING(first));
   FUDeclaration* typeB = GetTypeByName(versat,STRING(second));

   String naming = PushString(arena,"%s_%s",first,second);
   FUDeclaration* merged = MergeAccelerators(versat,typeA,typeB,naming,99,strategy);
   printf("\n");
   return merged;
}

TEST(TestMerge){
   MergingStrategy strategy = MergingStrategy::CONSOLIDATION_GRAPH;
   //MergingStrategy strategy = MergingStrategy::SIMPLE_COMBINATION;

   /*
      ================================================================================================
      IMPORTANT: Don't forget you changed CreateFUInstance from if(true) to if(!flat). Any weird bug in another place might be because of that

      Idea 1 : Try a max clique where we keep track of everything.
               Every iteration we look at a index and decide if we want to "improve it" (if max not already found) or if we let it be.
               Need to record where we left off, so we first must transform the recursive max clique into a iterative one.
                  The simplest would be to create a "stack frame" structure and dealing ourselves with the push and popping operations.

      Idea 2 : Move away from the Consolidation Graph approach. Make an algorithm that will eventually produce the correct result and then parallelize it.
               The best idea would be to use the fact that we are merging DAGs and try to find something from that.
               The problem is that the Consolidation Graph approach is already the algorithm that will produce the correct result, as it encodes every possible edge mapping or node mapping that exists.
               For example, going down the graph deciding whether to merge or not recursively would do the same thing as the Consolidation graph approach.
                  In fact, with the use of BitArray the Consolidation graph approach would perform much better.
                  Also, if the order in which we go down the graph matters, then we could improve the algorithm, but what we really want is something parallelizable.
                  All we would change is the order in which we are "searching" for the max clique, nothing has fundamentally changed.

      Idea 3 : Partition the graphs and find max cliques on them and then find a way of joining the results into the final graph.
               The thing I think it would work.
               Need to build a great lot for this.
               Also has the problem that it's a fairly trivial solution, unless we crank it up a notch.
                  One idea is to try various different partitions, even if they overlap.
                  Each one can be performed in parallel, but we ran into the problem of still being a simple solution.
               The only way of breaking this pattern is if we can move data in between searches for the solution.
                  If the results of a partitioned max clique can be used in the calculation of other partitioned max cliques.

      Otherwise, I have no other plan. Don't think there is anything else at this point

      ================================================================================================
   */

   #if 0
   TestMerge(versat,"A","B","M1",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"CH","Maj","M1",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"M_Stage","F_Stage","M2",strategy,temp); // 0.4
   #endif

   #if 0
   TestMerge(versat,"DoRow","AddRoundKey","M3",strategy,temp); // 0.4
   #endif

   #if 0
   TestMerge(versat,"KeccakF1600","KeySchedule","B1",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"Blake2G","M_Stage","A1",strategy,temp); // 0.28
   #endif

   #if 0
   TestMerge(versat,"Blake2G","F_Stage","A1",strategy,temp); // 0.27
   #endif

   #if 0
   TestMerge(versat,"MixColumns","F2","M4",strategy,temp); // 10 timeout,
   #endif

   #if 0
   TestMerge(versat,"MainRound","F2","M4",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"StringHasher","Convolution","M4",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"Big0","Big1","M",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"M2","F2","M5",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"M","F","M6",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"M8","F8","M7",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"SHA","AES","M6",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"SHA","Blake","M6",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"SHA","Keccak24Rounds","M6",strategy,temp);
   #endif

   #if 0 // DO NOT NEED IT. ALL ZERO
   TestMerge(versat,"AES","Blake","M6",strategy,temp);
   #endif

   #if 0
   TestMerge(versat,"AES","Keccak24Rounds","M6",strategy,temp);
   #endif

   #if 0 // DO NOT NEED IT. ALL ZERO
   TestMerge(versat,"Blake","Keccak24Rounds","M6",strategy,temp);
   #endif

   TEST_PASSED;
}

TEST(SHA){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SHA");

   SetSHAAccelerator(test.accel,test.inst);

   InitVersatSHA(versat,true);

   //DebugAccelerator(test.accel,temp);

   unsigned char digest[256];
   for(int i = 0; i < 256; i++){
      digest[i] = 0;
   }

   sha256(digest,msg_64,64);
   VersatSHA(digest,msg_64,64);

   return EXPECT("42e61e174fbb3897d6dd6cef3dd2802fe67b331953b06114a65c772859dfc1aa","%s",GetHexadecimal(digest, HASH_SIZE));
}

TEST(MultipleSHATests){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"SHA");

   SetSHAAccelerator(test.accel,test.inst);

   InitVersatSHA(versat,true);

   unsigned char digestSW[256];
   unsigned char digestHW[256];
   int passed = 0;
   for(int i = 0; i < NUM_MSGS; i++){
      for(int ii = 0; ii < 256; ii++){
         digestSW[ii] = 0;
         digestHW[ii] = 0;
      }

      sha256(digestSW,msg_array[i],msg_len[i]);
      VersatSHA(digestHW,msg_array[i],msg_len[i]);

      #if 1
      if(memcmp(digestSW,digestHW,256) == 0){
         passed += 1;
      } else {
         printf("%d\n",i);
      }
      #endif
   }

   return EXPECT("65","%d",passed);
}

TEST(SimpleIterative){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Iterative_SHA_F");

   FUInstance* t = GetInstanceByName(test.accel,"Test","comb","t");

   int constants[] = {6,11,25,2,13,22};
   for(size_t i = 0; i < ARRAY_SIZE(constants); i++){
      t->config[i] = constants[i];
   }

   int* out = RunSimpleAccelerator(&test,0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19,0x428a2f98,0x5a86b737);

   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 8; i++){
      ptr += sprintf(ptr,"0x%08x ",out[i]);
   }

   return EXPECT("0x8be76039 0x831cdb95 0xe63d7a4c 0x8f407b7a 0xa6bf2acc 0x5a261c78 0xae76d450 0x2f03130a ","%s",buffer);
}

TEST(FloatingPointAdd){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatAdd");

   float* out = (float*) RunSimpleAccelerator(&test,PackInt(6.0f),PackInt(4.0f));

   if(FloatEqual(*out,10.0f,0.01f)){
      TEST_PASSED;
   } else {
      TEST_FAILED("Not equal");
   }
}

TEST(FloatingPointSub){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatSub");

   float* out = (float*) RunSimpleAccelerator(&test,PackInt(0.848317862f),PackInt(0.874653578f));

   OutputVersatSource(versat,&test,"versat_instance.v","versat_defs.vh","versat_data.inc");

   if(FloatEqual(*out,-0.0263357162f,0.01f)){
      TEST_PASSED;
   } else {
      TEST_FAILED("Not equal");
   }
}

TEST(FloatingPointMul){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatMul");

   float* out = (float*) RunSimpleAccelerator(&test,PackInt(4.0f),PackInt(6.0f));

   if(FloatEqual(*out,24.0f,0.01f)){
      TEST_PASSED;
   } else {
      TEST_FAILED("Not equal");
   }
}

TEST(FloatingPointDiv){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatDiv");

   #if 1
   SeedRandomNumber(COMPILE_TIME);
   int numberTests = 10;

   int correct = 0;
   for(int i = 0; i < numberTests; i++){
      float f1 = (float) Abs(GetRandomNumber());
      float f2 = (float) (Abs(GetRandomNumber()) + 1);
      float* out = (float*) RunSimpleAccelerator(&test,PackInt(f1),PackInt(f2));
      float expected = f1 / f2;

      if(FloatEqual(*out,expected)){
         correct += 1;
      } else {
         printf("Failed: %d,%f,%f,%f,%f\n",i,f1,f2,expected,*out);
      }
   }
   if(correct != numberTests){
      TEST_FAILED("Failed (%d,%d) [Seed: %d]",correct,numberTests,COMPILE_TIME);
   }
   #endif

   float* out = (float*) RunSimpleAccelerator(&test,PackInt(6.0f),PackInt(2.0f));

   if(FloatEqual(*out,3.0f,0.01f)){
      // Test passed
   } else {
      TEST_FAILED("Failed: %f %f\n",3.0f,*out);
   }

   out = (float*) RunSimpleAccelerator(&test,PackInt(0.0f),PackInt(2.0f));

   if(FloatEqual(*out,0.0f,0.01f)){
      TEST_PASSED;
   } else {
      TEST_FAILED("Failed: %f %f\n",0.0f,*out);
   }
}

#include <math.h>

TEST(FloatingPointSqrt){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatSqrt");

   #if 1
   SeedRandomNumber(COMPILE_TIME);
   int numberTests = 10;

   int correct = 0;
   for(int i = 0; i < numberTests; i++){
      int number = Abs(GetRandomNumber());
      float f = PackFloat(number);
      float* out = (float*) RunSimpleAccelerator(&test,PackInt(f));
      float expected = Sqrt(f);

      if(FloatEqual(*out,expected)){
         correct += 1;
      } else {
         printf("Failed: %d,%f,%f,%f\n",i,f,expected,*out);
      }
   }
   if(correct != numberTests){
      TEST_FAILED("Failed (%d,%d) [Seed: %d]",correct,numberTests,COMPILE_TIME);
   }
   #endif

   float f = 0.1f;
   float* out = (float*) RunSimpleAccelerator(&test,PackInt(f));
   float expected = Sqrt(f);

   if(FloatEqual(*out,expected)){
      TEST_PASSED;
   } else {
      TEST_FAILED("Failed: %f %f\n",expected,*out);
   }
}

TEST(IntSqrt){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"IntSqrt");

   #if 1
   SeedRandomNumber(COMPILE_TIME);
   int numberTests = 10;

   int correct = 0;
   for(int i = 0; i < numberTests; i++){
      int number = Abs(GetRandomNumber());
      int* out = RunSimpleAccelerator(&test,number);
      float f = (float) number;
      int expected = (int) Sqrt(f);

      if(Abs(*out - expected) <= 1){ // Expected might be off by one
         correct += 1;
      } else {
         printf("Failed: %d %d:%d\n",i,*out,expected);
      }
   }
   if(correct != numberTests){
      printf("Seed used:%d\n",COMPILE_TIME);
      TEST_FAILED("Failed");
   }
   #endif

   int i = 25;
   int* out = RunSimpleAccelerator(&test,i);
   int expected = 5;

   if(*out == expected){
      TEST_PASSED;
   } else {
      printf("Failed: %d %d\n",expected,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(Q16Sqrt){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Q16Sqrt");

   int i = 0x00004000; // 0.25
   int* out = RunSimpleAccelerator(&test,i);
   int correct = 0x00008000; // 0.5

   if(*out == correct){
      TEST_PASSED;
   } else {
      printf("Failed: %d:%d\n",correct,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(FloatingPointMax){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatMax");

   float* out = (float*) RunSimpleAccelerator(&test,PackInt(10.0f),PackInt(15.0f));
   float correct = 15.0f;

   if(FloatEqual(*out,correct)){
      TEST_PASSED;
   } else {
      printf("Failed: %f:%f\n",correct,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(FloatingPointMin){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"FloatMin");

   float* out = (float*) RunSimpleAccelerator(&test,PackInt(10.0f),PackInt(15.0f));
   float correct = 10.0f;

   if(FloatEqual(*out,correct)){
      TEST_PASSED;
   } else {
      printf("Failed: %f:%f\n",correct,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(Float2Int){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Float2Int");

   int* out = RunSimpleAccelerator(&test,PackInt(10.1f));
   int correct = 10;

   if(*out == correct){
      TEST_PASSED;
   } else {
      printf("Failed: %d:%d\n",correct,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(Float2UInt){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Float2UInt");

   // TODO: find a better test for this one
   unsigned int* out = (unsigned int*) RunSimpleAccelerator(&test,PackInt(10.1f));
   unsigned int correct = 10;

   if(*out == correct){
      TEST_PASSED;
   } else {
      printf("Failed: %d:%d\n",correct,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(Int2Float){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"Int2Float");
   // TODO: find a better test for this one
   float* out = (float*) RunSimpleAccelerator(&test,10);
   float correct = 10.0f;

   if(FloatEqual(*out,correct)){
      TEST_PASSED;
   } else {
      printf("Failed: %f:%f\n",correct,*out);
      TEST_FAILED("Not equal");
   }
}

TEST(TestConfigOrder){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"TestConfigOrder");

   // Changing order of configure mem with const changes configuration.
   ConfigureMemoryReceive(test.inst,1,0);

   //FUInstance* Const = GetInstanceByName(test.accel,"Test","a1");
   //Const->config[0] = 5;
   //test.inst->config[26] = 5;

   AcceleratorRun(test.accel);

   FUInstance* Mem = GetInstanceByName(test.accel,"Test","store");
   int result = VersatUnitRead(test.inst,0);

   return EXPECT("5","%d",result);
}

TEST(RunSimpleAcceleratorLatency){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"RunSimpleAcceleratorLatency");

   FUInstance* gen = GetSubInstanceByName(test.accel,test.inst,"gen");

   ConfigureGenerator(gen,0,255,1);

   RunSimpleAccelerator(&test,1,1,0x900000);

   TEST_PASSED;
}

TEST(ComplexCalculateDelay){
   SimpleAccelerator test = {};
   InitSimpleAccelerator(&test,versat,"ComplexCalculateDelay");

   RunSimpleAccelerator(&test);

   TEST_PASSED; // Need to look at .dot file to check
}

#if 0
#ifdef PC
#include "debug.hpp"
extern "C"{
#include "crypto/mceliece/rng.h"
#include "crypto/mceliece/mceliece348864/crypto_kem_mceliece348864.h"
}

TEST(McEliece){
   unsigned char pk[CRYPTO_PUBLICKEYBYTES];
   unsigned char sk[CRYPTO_SECRETKEYBYTES];
   unsigned char ct[CRYPTO_CIPHERTEXTBYTES];
   unsigned char ss[CRYPTO_BYTES];
   unsigned char ss1[CRYPTO_BYTES];
   unsigned char entropy_input[48];
   unsigned char seed[48];

   for (int i = 0; i < 48; i++)
      entropy_input[i] = i;
   randombytes_init(entropy_input, NULL, 256);

   randombytes(seed, 48);

   randombytes_init(seed,NULL,256);

   {
   TIME_IT("1");
   int res = crypto_kem_mceliece348864_keypair(pk,sk);
   }
   {
   TIME_IT("2");
   int res2 = crypto_kem_mceliece348864_enc(ct, ss, pk);
   }
   {
   TIME_IT("3");
   int res3 = crypto_kem_mceliece348864_dec(ss1, ct, sk);
   }

   #if 0
   OutputMemoryHex(seed,48);
   printf("\n");
   OutputMemoryHex(pk,CRYPTO_PUBLICKEYBYTES);
   printf("\n");
   OutputMemoryHex(sk,CRYPTO_SECRETKEYBYTES);
   printf("\n");
   OutputMemoryHex(ct,CRYPTO_CIPHERTEXTBYTES);
   printf("\n");
   OutputMemoryHex(ss,CRYPTO_BYTES);
   printf("\n");
   OutputMemoryHex(ss1,CRYPTO_BYTES);
   #endif

   String first = PushMemoryHex(temp,ss,CRYPTO_BYTES);

   if(memcmp(ss,ss1,CRYPTO_BYTES) != 0){
      TEST_FAILED("Not equal");
   }

   return EXPECT("b4 f9 ff 1e 43 90 e3 be 0b bc eb ff 9a 52 5a e8 3b 19 12 11 89 6a a8 78 6c e8 bc 51 1c 9f 78 c3 ","%.*s",UNPACK_SS(first));
}

#include "parser.hpp"

#define FFT_SIZE 1024
void VersatStradedFFT(float real[FFT_SIZE], float img[FFT_SIZE], float real_twid[FFT_SIZE/2], float img_twid[FFT_SIZE/2]);

TEST(FFT){
   FILE* inputData = fopen("../firmware/benchmarks/MachSuite/fft/strided/input.data","r");

   float data_x[FFT_SIZE];
   float data_y[FFT_SIZE];
   float img[FFT_SIZE/2];
   float real[FFT_SIZE/2];
   float data_x_expected[FFT_SIZE];
   float data_y_expected[FFT_SIZE];

   {
   String content = PushFile(temp,"../firmware/benchmarks/MachSuite/fft/strided/input.data");
   Tokenizer tok(content,"",{"%%"});

   tok.AssertNextToken("%%");
   for(int i = 0; i < FFT_SIZE; i++){
      data_x[i] = ParseFloat(tok.NextToken());
   }
   tok.AssertNextToken("%%");
   for(int i = 0; i < FFT_SIZE; i++){
      data_y[i] = ParseFloat(tok.NextToken());
   }
   tok.AssertNextToken("%%");
   for(int i = 0; i < FFT_SIZE/2; i++){
      img[i] = ParseFloat(tok.NextToken());
   }
   tok.AssertNextToken("%%");
   for(int i = 0; i < FFT_SIZE/2; i++){
      real[i] = ParseFloat(tok.NextToken());
   }
   }

   {
   String content = PushFile(temp,"../firmware/benchmarks/MachSuite/fft/strided/check.data");
   Tokenizer tok(content,"",{"%%"});

   tok.AssertNextToken("%%");
   for(int i = 0; i < FFT_SIZE; i++){
      data_x_expected[i] = ParseFloat(tok.NextToken());
   }
   tok.AssertNextToken("%%");
   for(int i = 0; i < FFT_SIZE; i++){
      data_y_expected[i] = ParseFloat(tok.NextToken());
   }
   }

   VersatStradedFFT(data_x,data_y,img,real);

   // The original spec used doubles.
   // We change to float, but need to use a lower comparison epsilon to account for the loss of precision
   for(int i = 0; i < FFT_SIZE; i++){
      if(!FloatEqual(data_x[i],data_x_expected[i],0.01f)){
         TEST_FAILED("Different values [%d] %lf : %lf",i,data_x[i],data_x_expected[i]);
      }
      if(!FloatEqual(data_y[i],data_y_expected[i],0.01f)){
         TEST_FAILED("Different values [%d] %lf : %lf",i,data_y[i],data_y_expected[i]);
      }
   }

   TEST_PASSED;
}

extern "C"{
#include "crypto/fips202.h"
}

TEST(SHA3){
   uint8_t shakeOut[256];
   shake256(shakeOut,256,(uint8_t*)"",0);
   TestInfo shake = EXPECT("46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762fd75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be","%s",GetHexadecimal(shakeOut, 64));

   uint8_t sha3Out[256];
   sha3_256(sha3Out,(uint8_t*)"",0);
   TestInfo sha3 = EXPECT("a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a","%s",GetHexadecimal(sha3Out, 32));

   return sha3 & shake;
}

extern "C"{
   #include "crypto/blake2/blake2.h"
}

TEST(Blake2s){
   // Blake2s is the optimized implemented for 32 bit systems
   unsigned char out[256];

   String testStrIn = STRING("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f40414243");
   unsigned char testIn[128];

   String testKeyIn = STRING("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
   unsigned char testKey[128];

   HexStringToHex(testIn,testStrIn);
   HexStringToHex(testKey,testKeyIn);

   blake2s(out,32,(void*) testIn,sizeof(testStrIn) / 2,(void*) testKey,sizeof(testKeyIn) / 2);

   TestInfo info = EXPECT("af533e022fc9439e4e3cb838ecd18692232adf6fe9839526d3c3dd1b71910b1a","%s",GetHexadecimal(out, 32));

   return info;
}
#endif
#endif

#define DISABLED (REVERSE_ENABLED)

#ifndef HARDWARE_TEST
   #define HARDWARE_TEST -1
   #define ENABLE_TEST(ENABLED) (!(ENABLED) != !(REVERSE_ENABLED))
#else
   #define ENABLE_TEST(ENABLED) (currentTest == hardwareTest)
#endif

#define TEST_INST(ENABLED,TEST_NAME) do { if(ENABLE_TEST( ENABLED )){ \
      ArenaMarker marker(&temp); \
      TestInfo test = TEST_NAME(versat,&temp,currentTest); \
      if(test.testsPassed == test.numberTests) printf("%32s [%02d] - OK\n",#TEST_NAME,currentTest); \
      info += test; \
   } \
   currentTest += 1; } while(0)

// When 1, need to pass 0 to enable test (changes enabler from 1 to 0)
#define REVERSE_ENABLED 1

//                 76543210
#define SEGMENTS 0b00000001

#define SEG0 (SEGMENTS & 0x01)
#define SEG1 (SEGMENTS & 0x02)
#define SEG2 (SEGMENTS & 0x04)
#define SEG3 (SEGMENTS & 0x08)
#define SEG4 (SEGMENTS & 0x10)
#define SEG5 (SEGMENTS & 0x20)
#define SEG6 (SEGMENTS & 0x40)
#define SEG7 (PC && (SEGMENTS & 0x80))

void AutomaticTests(Versat* versat){
   Arena temp = {};
   #ifdef PC
   temp = InitArena(Megabyte(1));
   #else
   temp = InitArena(Kilobyte(1));
   #endif

   TestInfo info = TestInfo(0,0);
   int hardwareTest = HARDWARE_TEST;
   int currentTest = 0;

   #if 0
   EnterDebugTerminal(versat);
   #endif

   TestVersatSide(versat);

#if SEG0
   TEST_INST( 1 ,TestMStage);
   TEST_INST( 1 ,TestFStage);
   TEST_INST( 0 ,SHA);
   TEST_INST( DISABLED ,MultipleSHATests);
   TEST_INST( DISABLED ,VReadToVWrite);
   TEST_INST( 1 ,StringHasher);
   TEST_INST( DISABLED ,Convolution);
   TEST_INST( 1 ,MatrixMultiplication);
   TEST_INST( DISABLED ,MatrixMultiplicationVRead);
   TEST_INST( 1 ,VersatAddRoundKey);
   TEST_INST( 1 ,LookupTable);
   TEST_INST( 1 ,VersatSubBytes);
   TEST_INST( 1 ,VersatShiftRows);
   TEST_INST( 1 ,VersatDoRows);
   TEST_INST( 1 ,VersatMixColumns);
   TEST_INST( 1 ,FirstLineKey);
   TEST_INST( 1 ,KeySchedule);
   TEST_INST( 1 ,AESRound);
   TEST_INST( 1 ,AES);
   TEST_INST( DISABLED ,ReadWriteAES);
   TEST_INST( 1 ,SimpleAdder);
   TEST_INST( 1 ,ComplexMultiplier);
#endif
#if SEG1 // Config sharing
   TEST_INST( 1 ,SimpleShareConfig);
   TEST_INST( 1 ,ComplexShareConfig);
   TEST_INST( 1 ,SimpleStaticConfig);
   TEST_INST( 1 ,ComplexStaticConfig);
#endif
#if SEG2 // Flattening
   TEST_INST( 1 ,SimpleFlatten);
   TEST_INST( 1 ,FlattenShareConfig);
   TEST_INST( DISABLED ,FlattenAES); // Takes a bit of time
   TEST_INST( 0 ,FlattenAESVRead);
   TEST_INST( DISABLED ,FlattenSHA); // Problem on top level static buffers. Maybe do flattening of accelerators with buffers already fixed.
#endif
#if SEG3 // Merging
   TEST_INST( 1 ,SimpleMergeNoCommon);
   TEST_INST( 1 ,SimpleMergeUnitCommonNoEdge);
   TEST_INST( 1 ,SimpleMergeUnitAndEdgeCommon);
   TEST_INST( 1 ,SimpleMergeInputOutputCommon);
   TEST_INST( 1 ,CombinatorialMerge);
   TEST_INST( 1 ,SimpleThreeMerge);
   TEST_INST( 1 ,ComplexMerge);
   TEST_INST( DISABLED ,TestSpecificMerge);
   TEST_INST( DISABLED ,TestMerge);
#endif
#if SEG4 // Iterative units
   TEST_INST( 1 ,SimpleIterative);
#endif
#if SEG5 // Floating point and related units
   TEST_INST( 1 ,FloatingPointAdd);
   TEST_INST( 0 ,FloatingPointSub);
   TEST_INST( 1 ,FloatingPointMul);
   TEST_INST( 1 ,FloatingPointDiv);
   TEST_INST( 1 ,FloatingPointSqrt);
   TEST_INST( 1 ,IntSqrt);
   TEST_INST( 1 ,Q16Sqrt);
   TEST_INST( 1 ,FloatingPointMax);
   TEST_INST( 1 ,FloatingPointMin);
   TEST_INST( 1 ,Float2Int);
   TEST_INST( 1 ,Float2UInt);
   TEST_INST( 1 ,Int2Float);
#endif
#if SEG6 // Individual units
   TEST_INST( 1 ,TestConfigOrder);
   TEST_INST( 0 ,RunSimpleAcceleratorLatency);
   TEST_INST( 0 ,ComplexCalculateDelay);
   TEST_INST( 0 ,Generator);
   TEST_INST( 0 ,Test64Bits);
#endif
#if SEG7
   TEST_INST( DISABLED ,McEliece);
   TEST_INST( 0 ,FFT);
   TEST_INST( 0 ,SHA3);
   TEST_INST( 0 ,Blake2s);
#endif

   #if 0
   EnterDebugTerminal(versat);
   #endif

   #if 0
   Hook(versat,nullptr,nullptr);
   //Free(versat);
   #endif

   printf("\nAutomatic tests done (passed/total): %d / %d\n",info.testsPassed,info.numberTests);
}
