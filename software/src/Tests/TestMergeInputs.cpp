#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->input_0.constant = 10;
   accelConfig->input_1.constant = 5;

   ActivateMergedAccelerator(MergeType_TestMergeInputs1);

   RunAccelerator(3);

   Assert_Eq(15,accelState->TOP_output_0_currentValue);

   ActivateMergedAccelerator(MergeType_TestMergeInputs2);

   RunAccelerator(3);

   Assert_Eq(5,accelState->TOP_output_0_currentValue);
}