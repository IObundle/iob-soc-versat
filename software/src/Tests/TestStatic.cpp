#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->child_0.inside.constant = 1;
   accelConfig->child_1.inside.constant = 2;

   accelStatic->TestStatic_inside_0_constant = 3;
   accelStatic->TestStatic_inside_1_constant = 4;
   accelStatic->TestStaticChild1_child_inside0_constant = 5;
   accelStatic->TestStaticChild2_inside1_constant = 6;

   RunAccelerator(3);

   Assert_Eq(32,accelState->TOP_output_currentValue);
}