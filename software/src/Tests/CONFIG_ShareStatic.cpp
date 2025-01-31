#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->testShare_0.inside.constant = 1;
   accelConfig->testShare_1.inside.constant = 2;

   ACCEL_TestStaticChild1_child_inside0_constant = 3;
   ACCEL_TestStaticChild2_inside1_constant = 4;
   ACCEL_TestShareStaticChild_shareChild_x_0_constant = 5;
   ACCEL_TestShareStaticChild_shareChild_y_0_constant = 6;

   RunAccelerator(3);
}