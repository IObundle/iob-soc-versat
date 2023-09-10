#include "testbench.h"

void Debug();

void SingleTest(Arena* arena){
   iptr addrA = 0;
   iptr addrB = 4;

   VersatUnitWrite(TOP_simple_addr,0,0xf0);
   VersatUnitWrite(TOP_simple_addr,1,0xf4);

   SimpleInputStart[0] = addrA;
   SimpleInputStart[1] = addrB;
   
   RunAccelerator(1);

   printf("%d\n",SimpleOutputStart[0]);
   printf("%d\n",SimpleOutputStart[1]);
  
   Assert_Eq(0xf0,SimpleOutputStart[0]);
   Assert_Eq(0xf4,SimpleOutputStart[1]);
}
