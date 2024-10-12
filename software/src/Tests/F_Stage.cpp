#include "testbench.hpp"

void SingleTest(Arena* arena){
   ACCEL_TOP_input_0_constant = 0x6a09e667;
   ACCEL_TOP_input_1_constant = 0xbb67ae85;
   ACCEL_TOP_input_2_constant = 0x3c6ef372;
   ACCEL_TOP_input_3_constant = 0xa54ff53a;
   ACCEL_TOP_input_4_constant = 0x510e527f;
   ACCEL_TOP_input_5_constant = 0x9b05688c;
   ACCEL_TOP_input_6_constant = 0x1f83d9ab;
   ACCEL_TOP_input_7_constant = 0x5be0cd19;
   ACCEL_TOP_input_8_constant = 0x428a2f98;
   ACCEL_TOP_input_9_constant = 0x5a86b737;
     
   ACCEL_Comb_F_Stage_t_t1_s_const1_constant = 6;
   ACCEL_Comb_F_Stage_t_t1_s_const2_constant = 11;
   ACCEL_Comb_F_Stage_t_t1_s_const3_constant = 25;
   ACCEL_Comb_F_Stage_t_t2_s_const1_constant = 2;
   ACCEL_Comb_F_Stage_t_t2_s_const2_constant = 13;
   ACCEL_Comb_F_Stage_t_t2_s_const3_constant = 22;

   RunAccelerator(3);
     
   Assert_Eq((unsigned int) 0x568f3f84,(unsigned int) ACCEL_TOP_output_0_currentValue);
   Assert_Eq((unsigned int) 0x6a09e667,(unsigned int) ACCEL_TOP_output_1_currentValue);
   Assert_Eq((unsigned int) 0xbb67ae85,(unsigned int) ACCEL_TOP_output_2_currentValue);
   Assert_Eq((unsigned int) 0x3c6ef372,(unsigned int) ACCEL_TOP_output_3_currentValue);
   Assert_Eq((unsigned int) 0xf34e99d9,(unsigned int) ACCEL_TOP_output_4_currentValue);
   Assert_Eq((unsigned int) 0x510e527f,(unsigned int) ACCEL_TOP_output_5_currentValue);
   Assert_Eq((unsigned int) 0x9b05688c,(unsigned int) ACCEL_TOP_output_6_currentValue);
   Assert_Eq((unsigned int) 0x1f83d9ab,(unsigned int) ACCEL_TOP_output_7_currentValue);
}









