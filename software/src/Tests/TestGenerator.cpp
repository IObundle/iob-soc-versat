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

#if 0
   accelConfig->gen.duty = ~0;
   accelConfig->gen.start = 0;
   accelConfig->gen.incr = 1;
   accelConfig->gen.period = 3;
   accelConfig->gen.shift = 8;
   accelConfig->gen.iterations = 3;
   accelConfig->gen.incr2 = 1;
   accelConfig->gen.period2 = 2;
   accelConfig->gen.shift2 = 9;
   accelConfig->gen.iterations2 = 2;
#endif

   RunAccelerator(3);

   for(int i = 0; i < loopSize; i++){
      printf("%d,",VersatUnitRead(TOP_mem_addr,i));
   }
   printf("\n");

}
