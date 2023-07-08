#include "testbench.h"

void SingleTest(){
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

   RunAccelerator(1);

   int out[] = {ACCEL_TOP_output_0_currentValue,
                ACCEL_TOP_output_1_currentValue,
                ACCEL_TOP_output_2_currentValue,
                ACCEL_TOP_output_3_currentValue,
                ACCEL_TOP_output_4_currentValue,
                ACCEL_TOP_output_5_currentValue,
                ACCEL_TOP_output_6_currentValue,
                ACCEL_TOP_output_7_currentValue};
   
   char buffer[1024];
   char* ptr = buffer;
   for(int i = 0; i < 8; i++){
      ptr += sprintf(ptr,"0x%08x ",out[i]);
   }

   Expect("0x568f3f84 0x6a09e667 0xbb67ae85 0x3c6ef372 0xf34e99d9 0x510e527f 0x9b05688c 0x1f83d9ab ","%s",buffer);   
}









