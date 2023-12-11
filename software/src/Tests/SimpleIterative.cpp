#include "testbench.hpp"

void SingleTest(Arena* arena){
  ACCEL_TOP_input_0_constant = 1;
  ACCEL_TOP_simple_const0_constant = 2; 
  ACCEL_TOP_simple_const1_constant = 3; 
  ACCEL_TOP_simple_const2_constant = 4; 
  ACCEL_TOP_simple_const3_constant = 5;

  RunAccelerator(1);
  
  Assert_Eq(1*2*3*4*5,ACCEL_TOP_output_0_currentValue);
}
