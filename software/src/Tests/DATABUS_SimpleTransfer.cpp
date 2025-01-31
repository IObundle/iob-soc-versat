#include "testbench.hpp"

#include "unitConfiguration.hpp"

#define SIZE 1234

void SingleTest(Arena* arena){
  int* inputBuffer = (int*) PushBytes(arena,sizeof(int) * SIZE * 2);
  int* outputBuffer = (int*) PushBytes(arena,sizeof(int) * SIZE * 2);

  // TODO: This test should offset the address by 1 (test unaligned copies)
  //       Since the CPU might not be able to handle unaligned accesses, need to write an 
  //       interface to allow easy filling of the array with the values that we want.

  for(int i = 0; i < 4; i++){
    printf("Loop: %d\n",i);
    int* input = &inputBuffer[i];
    int* output = &outputBuffer[i];

    printf("%p %p %p\n",arena->mem,input,output);

    for(int i = 0; i < SIZE; i++){
      input[i] = i + 1;
    }
    ClearCache(arena->mem);

    ConfigureSimpleVRead(&accelConfig->read,SIZE,input);
    ConfigureSimpleVWrite(&accelConfig->write,SIZE,output);
  
    RunAccelerator(1);

    accelConfig->write.write_enabled = 0;
    accelConfig->read.read_enabled = 0;

    RunAccelerator(2); // Flush accelerator

    ClearCache(arena->mem);

    bool equal = true;     
    for(int i = 0; i < SIZE; i++){
      if(input[i] != output[i]){
        equal = false;
        printf("Different at %d: %d %d\n",i,input[i],output[i]);
      }
    }
 
    if(equal){
      Assert_Eq(1,1);
    } else {
      Assert_Eq(-1,i);
    }
  }
}
