#include "testbench.hpp"

void SingleTest(Arena* arena){
   iptr addrA = 0;
   iptr addrB = 4;

   VersatUnitWrite(TOP_simple_addr,addrA,0xf0);
   VersatUnitWrite(TOP_simple_addr,addrB,0xf4);

   SimpleInputStart[0] = addrA;
   SimpleInputStart[1] = addrB;
   
   RunAccelerator(1);
  
   Assert_Eq(0xf0,SimpleOutputStart[0]);
   Assert_Eq(0xf4,SimpleOutputStart[1]);
}
