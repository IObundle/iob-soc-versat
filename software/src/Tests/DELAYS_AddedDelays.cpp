#include "testbench.hpp"

void SingleTest(Arena* arena){
   RunAccelerator(2);

   Assert_Eq(15,accelState->TOP_output_currentValue);
}