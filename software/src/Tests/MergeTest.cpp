#include "testbench.hpp"

void SingleTest(Arena* arena){
  MERGED_0_m0_constant = 10;
  MERGED_0_m1_constant = 5;

  RunAccelerator(1);
  
  Assert_Eq(15,ACCEL_TOP_output_Output_currentValue);

  MERGED_1_M0_constant = 10;
  MERGED_1_M1_constant = 2;
  
  ACCEL_TOP_comb_mux0_sel = 1;
  
  RunAccelerator(1);
  
  Assert_Eq(8,ACCEL_TOP_output_Output_currentValue);
}


