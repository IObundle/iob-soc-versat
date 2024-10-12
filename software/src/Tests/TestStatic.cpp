#include "testbench.hpp"

void SingleTest(Arena* arena){
   ACCEL_TOP_child_0_inside_constant = 1;
   ACCEL_TOP_child_1_inside_constant = 2;

   ACCEL_TestStatic_inside_0_constant = 3;
   ACCEL_TestStatic_inside_1_constant = 4;
   ACCEL_TestStaticChild1_child_inside0_constant = 5;
   ACCEL_TestStaticChild2_inside1_constant = 6;

   RunAccelerator(3);

   Assert_Eq(32,ACCEL_TOP_output_currentValue);
}