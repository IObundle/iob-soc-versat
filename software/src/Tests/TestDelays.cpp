#include "testbench.hpp"

void SingleTest(Arena* arena){
   RunAccelerator(1);

   Assert_Eq(15,ACCEL_TOP_output_currentValue);
}