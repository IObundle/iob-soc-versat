#include "testbench.hpp"

void SingleTest(Arena* arena){
   TestMergeInputs_SimpleConfig* config = (TestMergeInputs_SimpleConfig*) accelConfig;

   config->input_0.constant = 10;
   config->input_1.constant = 5;

   ActivateMergedAccelerator(TestMergeChild1);

   RunAccelerator(1);

   Assert_Eq(15,ACCEL_TOP_output_output_currentValue);

   ActivateMergedAccelerator(TestMergeChild2);

   RunAccelerator(1);

   Assert_Eq(5,ACCEL_TOP_output_output_currentValue);
}