#include "testbench.hpp"

void SingleTest(){
   int input[] = {0x5a86b737,0xa9f9be83,0x08251f6d,0xeaea8ee9};
   VersatMemoryCopy(SimpleInputStart,input,4);

   // TODO: Finish the template for simple accelerators. Make it super easy to set inputs and outputs.
   // TODO: For outputs, maybe add a immediate mode API and have testbench allocate a buffer that is
   //       used by these functions

   ACCEL_Comb_M_Stage_sigma_sigma0_const1_constant = 7;
   ACCEL_Comb_M_Stage_sigma_sigma0_const2_constant = 18;
   ACCEL_Comb_M_Stage_sigma_sigma0_const3_constant = 3;
   ACCEL_Comb_M_Stage_sigma_sigma1_const1_constant = 17;
   ACCEL_Comb_M_Stage_sigma_sigma1_const2_constant = 19;
   ACCEL_Comb_M_Stage_sigma_sigma1_const3_constant = 10;

   RunAccelerator(1);

   Expect("0xb89ab4ca","0x%x",ACCEL_TOP_output_0_currentValue);
}
