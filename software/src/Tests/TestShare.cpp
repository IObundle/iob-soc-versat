#include "testbench.hpp"

void SingleTest(Arena* arena){
   // Any given combination of x and y should still add to the same value

   accelConfig->childs_0.x_0.constant = 1;
   accelConfig->childs_1.y_1.constant = 1;

   RunAccelerator(3);

   Assert_Eq(2,accelState->TOP_outputs_0_currentValue);
   Assert_Eq(2,accelState->TOP_outputs_1_currentValue);
   Assert_Eq(2,accelState->TOP_outputs_2_currentValue);
   Assert_Eq(2,accelState->TOP_outputs_3_currentValue);

   accelConfig->childs_0.x_1.constant = 2;
   accelConfig->childs_1.y_0.constant = 2;

   RunAccelerator(3);

   Assert_Eq(4,accelState->TOP_outputs_0_currentValue);
   Assert_Eq(4,accelState->TOP_outputs_1_currentValue);
   Assert_Eq(4,accelState->TOP_outputs_2_currentValue);
   Assert_Eq(4,accelState->TOP_outputs_3_currentValue);

   accelConfig->childs_1.x_1.constant = 3;
   accelConfig->childs_0.y_0.constant = 3;

   RunAccelerator(3);

   Assert_Eq(6,accelState->TOP_outputs_0_currentValue);
   Assert_Eq(6,accelState->TOP_outputs_1_currentValue);
   Assert_Eq(6,accelState->TOP_outputs_2_currentValue);
   Assert_Eq(6,accelState->TOP_outputs_3_currentValue);

   accelConfig->childs_1.x_0.constant = 4;
   accelConfig->childs_0.y_1.constant = 4;

   RunAccelerator(3);

   Assert_Eq(8,accelState->TOP_outputs_0_currentValue);
   Assert_Eq(8,accelState->TOP_outputs_1_currentValue);
   Assert_Eq(8,accelState->TOP_outputs_2_currentValue);
   Assert_Eq(8,accelState->TOP_outputs_3_currentValue);
}