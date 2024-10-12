#include "testbench.hpp"

void SingleTest(Arena* arena){
   // Any given combination of x and y should still add to the same value

   ACCEL_TOP_childs_0_x_0_constant = 1;
   ACCEL_TOP_childs_1_y_1_constant = 1;

   RunAccelerator(3);

   Assert_Eq(2,ACCEL_TOP_outputs_0_currentValue);
   Assert_Eq(2,ACCEL_TOP_outputs_1_currentValue);
   Assert_Eq(2,ACCEL_TOP_outputs_2_currentValue);
   Assert_Eq(2,ACCEL_TOP_outputs_3_currentValue);

   ACCEL_TOP_childs_0_x_1_constant = 2;
   ACCEL_TOP_childs_1_y_0_constant = 2;

   RunAccelerator(3);

   Assert_Eq(4,ACCEL_TOP_outputs_0_currentValue);
   Assert_Eq(4,ACCEL_TOP_outputs_1_currentValue);
   Assert_Eq(4,ACCEL_TOP_outputs_2_currentValue);
   Assert_Eq(4,ACCEL_TOP_outputs_3_currentValue);

   ACCEL_TOP_childs_1_x_1_constant = 3;
   ACCEL_TOP_childs_0_y_0_constant = 3;

   RunAccelerator(3);

   Assert_Eq(6,ACCEL_TOP_outputs_0_currentValue);
   Assert_Eq(6,ACCEL_TOP_outputs_1_currentValue);
   Assert_Eq(6,ACCEL_TOP_outputs_2_currentValue);
   Assert_Eq(6,ACCEL_TOP_outputs_3_currentValue);

   ACCEL_TOP_childs_1_x_0_constant = 4;
   ACCEL_TOP_childs_0_y_1_constant = 4;

   RunAccelerator(3);

   Assert_Eq(8,ACCEL_TOP_outputs_0_currentValue);
   Assert_Eq(8,ACCEL_TOP_outputs_1_currentValue);
   Assert_Eq(8,ACCEL_TOP_outputs_2_currentValue);
   Assert_Eq(8,ACCEL_TOP_outputs_3_currentValue);
}