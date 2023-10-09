#include "testbench.h"

void SingleTest(Arena* arena){
   iptr input[] = {(iptr)0x5a86b737,(iptr)0xa9f9be83,(iptr)0x08251f6d,(iptr)0xeaea8ee9};
   VersatMemoryCopy(SimpleInputStart,input,4);

   ACCEL_Comb_M_Stage_sigma_sigma0_const1_constant = 7;
   ACCEL_Comb_M_Stage_sigma_sigma0_const2_constant = 18;
   ACCEL_Comb_M_Stage_sigma_sigma0_const3_constant = 3;
   ACCEL_Comb_M_Stage_sigma_sigma1_const1_constant = 17;
   ACCEL_Comb_M_Stage_sigma_sigma1_const2_constant = 19;
   ACCEL_Comb_M_Stage_sigma_sigma1_const3_constant = 10;
   
   RunAccelerator(1);

   Assert_Eq(0xb89ab4ca,ACCEL_TOP_output_0_currentValue);
}
