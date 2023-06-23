#include "testbench.hpp"

void SingleTest(){
   int addrA = 0;
   int addrB = 4;

   VersatUnitWrite(addrA,0xf0);
   VersatUnitWrite(addrB,0xf4);

   SimpleInputStart[0] = addrA;
   SimpleInputStart[1] = addrB;

   RunAccelerator(1);

   Assert_Eq(0xf0,SimpleOutputStart[0]);
   Assert_Eq(0xf4,SimpleOutputStart[1]);
}
