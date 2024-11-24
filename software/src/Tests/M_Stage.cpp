#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->input_0.constant = 0x5a86b737;
   accelConfig->input_1.constant = 0xa9f9be83;
   accelConfig->input_2.constant = 0x08251f6d;
   accelConfig->input_3.constant = 0xeaea8ee9;

   accelStatic->Comb_M_Stage_sigma_sigma0_const1_constant = 7; 
   accelStatic->Comb_M_Stage_sigma_sigma0_const2_constant = 18; 
   accelStatic->Comb_M_Stage_sigma_sigma0_const3_constant = 3; 
   accelStatic->Comb_M_Stage_sigma_sigma1_const1_constant = 17; 
   accelStatic->Comb_M_Stage_sigma_sigma1_const2_constant = 19; 
   accelStatic->Comb_M_Stage_sigma_sigma1_const3_constant = 10; 
   
   RunAccelerator(3);

   Assert_Eq((unsigned int) 0xb89ab4ca,(unsigned int) accelState->TOP_output_0_currentValue);
}
