#include "testbench.hpp"

void Configure(volatile Generator3Config* config,int x){
   config->duty = ~0;

   config->start = 0;
   config->incr = 1;
   config->period = 3;
   config->shift = -2+x;
   config->iterations = 3;
}

void SingleTest(Arena* arena){
   int loopSize = 9;

   ConfigureMemoryReceive((MemConfig*) &accelConfig->mem,loopSize);

   Configure(&accelConfig->gen,10);

   RunAccelerator(3);

   for(int i = 0; i < loopSize; i++){
      printf("%d,",VersatUnitRead(TOP_mem_addr,i));
   }
   printf("\n");

}
