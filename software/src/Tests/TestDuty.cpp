#include "testbench.hpp"

void SingleTest(Arena* arena){
   // Still need to look into implementing stride or something similar to handle delay calculation
#if 0
   accelConfig->c.constant = 1;

   accelConfig->accum.duty = 3;
   ConfigureMemoryReceive((MemConfig*) &accelConfig->m,10);
   
   accelConfig->m.perA = 10;
   accelConfig->m.dutyA = 10;
   //accelConfig->m.stride = 3;

   RunAccelerator(3);

   for(int i = 0; i < 10; i++){
      Assert_Eq(3,VersatUnitRead(TOP_m_addr,i));
   }
#endif
}