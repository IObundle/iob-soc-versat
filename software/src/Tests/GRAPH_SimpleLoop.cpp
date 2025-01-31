#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->input_0.constant = 5;
   RunAccelerator(3);
   Assert_Eq(5,accelState->TOP_output_0_currentValue);
 
   RunAccelerator(1);
   Assert_Eq(10,accelState->TOP_output_0_currentValue);
 
   RunAccelerator(1);
   Assert_Eq(15,accelState->TOP_output_0_currentValue);
}