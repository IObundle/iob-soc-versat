#include "testbench.hpp"

void SingleTest(Arena* arena){
   SimpleInputStart[0] = PackInt(2.0f);
   SimpleInputStart[1] = PackInt(3.0f);
   SimpleInputStart[2] = PackInt(4.0f);

   RunAccelerator(1);

   Assert_Eq(PackFloat(SimpleOutputStart[0]),2.0f * 3.0f * 4.0f);
   Assert_Eq(PackFloat(SimpleOutputStart[1]),3.0f);
}