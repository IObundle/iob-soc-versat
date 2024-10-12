#include "testbench.hpp"

void SingleTest(Arena* arena){
   TestMergeConfig* config = (TestMergeConfig*) accelConfig;

   ActivateMergedAccelerator(MergeType_TestMergeChild1);

   config->TestMergeChild1.x.constant = 10;
   config->TestMergeChild1.y.constant = 5;

   RunAccelerator(3);

   Assert_Eq(15,ACCEL_TOP_output_output_currentValue);

   ActivateMergedAccelerator(MergeType_TestMergeChild2);

   config->TestMergeChild2.a.constant = 23;
   config->TestMergeChild2.b.constant = 3;

   RunAccelerator(3);

   Assert_Eq(20,ACCEL_TOP_output_output_currentValue);
}