#include "testbench.hpp"

void SingleTest(Arena* arena){
   ActivateMergedAccelerator(MergeType_TestMergeChild1);

   accelConfig->TestMergeChild1.x.constant = 10;
   accelConfig->TestMergeChild1.y.constant = 5;

   RunAccelerator(3);

   Assert_Eq(15,accelState->TOP_output_output_currentValue);

   ActivateMergedAccelerator(MergeType_TestMergeChild2);

   accelConfig->TestMergeChild2.a.constant = 23;
   accelConfig->TestMergeChild2.b.constant = 3;

   RunAccelerator(3);

   Assert_Eq(20,accelState->TOP_output_output_currentValue);
}