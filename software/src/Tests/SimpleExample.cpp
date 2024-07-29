#include "testbench.hpp"

void SingleTest(Arena* arena){
   accelConfig->a.constant = 10;
   accelConfig->b.constant = 5;

   RunAccelerator(1);

   printf("Result: %d\n",accelState->TOP_result_currentValue);
}