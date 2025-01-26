#include "testbench.hpp"

void SingleTest(Arena* arena){
   ACCEL_TOP_testShare_0_inside_constant = 1;
   ACCEL_TOP_testShare_1_inside_constant = 2;

   ACCEL_TestStaticChild1_child_inside0_constant = 3;
   ACCEL_TestStaticChild2_inside1_constant = 4;
   ACCEL_TestShareStaticChild_shareChild_x_0_constant = 5;
   ACCEL_TestShareStaticChild_shareChild_y_0_constant = 6;

   RunAccelerator(3);

   // Probably printing because we never figure out how static and share should actually interact when together.
   // TODO: See to it
   printf("%d\n",ACCEL_TOP_outputs_0_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_1_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_2_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_3_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_4_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_5_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_6_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_7_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_8_currentValue);
   printf("%d\n",ACCEL_TOP_outputs_9_currentValue);
}