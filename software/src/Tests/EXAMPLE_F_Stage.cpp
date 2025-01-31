#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->input_0.constant = 0x6a09e667;
   accelConfig->input_1.constant = 0xbb67ae85;
   accelConfig->input_2.constant = 0x3c6ef372;
   accelConfig->input_3.constant = 0xa54ff53a;
   accelConfig->input_4.constant = 0x510e527f;
   accelConfig->input_5.constant = 0x9b05688c;
   accelConfig->input_6.constant = 0x1f83d9ab;
   accelConfig->input_7.constant = 0x5be0cd19;
   accelConfig->input_8.constant = 0x428a2f98;
   accelConfig->input_9.constant = 0x5a86b737;
     
   accelStatic->Comb_F_Stage_t_t1_s_const1_constant = 6;
   accelStatic->Comb_F_Stage_t_t1_s_const2_constant = 11;
   accelStatic->Comb_F_Stage_t_t1_s_const3_constant = 25;
   accelStatic->Comb_F_Stage_t_t2_s_const1_constant = 2;
   accelStatic->Comb_F_Stage_t_t2_s_const2_constant = 13;
   accelStatic->Comb_F_Stage_t_t2_s_const3_constant = 22;

   RunAccelerator(3);
     
   Assert_Eq((unsigned int) 0x568f3f84,(unsigned int) accelState->TOP_output_0_currentValue);
   Assert_Eq((unsigned int) 0x6a09e667,(unsigned int) accelState->TOP_output_1_currentValue);
   Assert_Eq((unsigned int) 0xbb67ae85,(unsigned int) accelState->TOP_output_2_currentValue);
   Assert_Eq((unsigned int) 0x3c6ef372,(unsigned int) accelState->TOP_output_3_currentValue);
   Assert_Eq((unsigned int) 0xf34e99d9,(unsigned int) accelState->TOP_output_4_currentValue);
   Assert_Eq((unsigned int) 0x510e527f,(unsigned int) accelState->TOP_output_5_currentValue);
   Assert_Eq((unsigned int) 0x9b05688c,(unsigned int) accelState->TOP_output_6_currentValue);
   Assert_Eq((unsigned int) 0x1f83d9ab,(unsigned int) accelState->TOP_output_7_currentValue);
}









