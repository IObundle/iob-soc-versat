#include "testbench.hpp"

void SingleTest(Arena* arena){
   ACCEL_TOP_input_0_constant = 0x5a86b737;
   ACCEL_TOP_input_1_constant = 0xa9f9be83;
   ACCEL_TOP_input_2_constant = 0x08251f6d;
   ACCEL_TOP_input_3_constant = 0xeaea8ee9;

   ACCEL_Comb_M_Stage_sigma_sigma0_const1_constant = 7;
   ACCEL_Comb_M_Stage_sigma_sigma0_const2_constant = 18;
   ACCEL_Comb_M_Stage_sigma_sigma0_const3_constant = 3;
   ACCEL_Comb_M_Stage_sigma_sigma1_const1_constant = 17;
   ACCEL_Comb_M_Stage_sigma_sigma1_const2_constant = 19;
   ACCEL_Comb_M_Stage_sigma_sigma1_const3_constant = 10;
   
   RunAccelerator(3);

   Assert_Eq((unsigned int) 0xb89ab4ca,(unsigned int) ACCEL_TOP_output_0_currentValue);
}
