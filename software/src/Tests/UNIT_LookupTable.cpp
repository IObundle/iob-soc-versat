#include "testbench.hpp"

void SingleTest(Arena* arena){
   iptr addrA = 0;
   iptr addrB = 1;

   iptr indexA = 0;
   iptr indexB = 4;

   VersatUnitWrite(TOP_table_addr,addrA,0xf0);
   VersatUnitWrite(TOP_table_addr,addrB,0xf4);

   accelConfig->input_0.constant = indexA;
   accelConfig->input_1.constant = indexB;
   
   RunAccelerator(3);
  
   Assert_Eq(0xf0,accelState->TOP_output_0_currentValue);
   Assert_Eq(0xf4,accelState->TOP_output_1_currentValue);
}
