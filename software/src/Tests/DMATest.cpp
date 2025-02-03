#include "testbench.hpp"

#define SIZE 8

void SingleTest(Arena* arena){
   int bufferToCopy[SIZE];
   int outputBuffer[SIZE];

   for(int i = 0; i < SIZE; i++){
      bufferToCopy[i] = i;
   }

   // This does not work in pc-emul.
   VersatMemoryCopy((void*) TOP_mem_addr,bufferToCopy,sizeof(int) * SIZE);
   VersatMemoryCopy(outputBuffer,(void*) TOP_mem_addr,sizeof(int) * SIZE);

   for(int i = 0; i < SIZE; i++){
      Assert_Eq(i,outputBuffer[i]);
   }
}

