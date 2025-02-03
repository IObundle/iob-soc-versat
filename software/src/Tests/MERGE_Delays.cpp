#include "testbench.hpp"

void SingleTest(Arena* arena){
   MERGE_DelaysAddr addr = ACCELERATOR_TOP_ADDR_INIT;

   int data[] = {0,1,2,3,4,5,6,7,8,9};

   ConfigureSimpleMemory(&accelConfig->TestMergeDelay1.mem,ARRAY_SIZE(data),0,addr.mem_mem,data);
   
   // Because the unit is shared, no need to do this, although it would be best in a real usage of the accelerator
   //ConfigureSimpleMemory(&accelConfig->TestMergeDelay2.mem,ARRAY_SIZE(data),0,TOP_mem_mem_addr,data);

   ActivateMergedAccelerator(MergeType_TestMergeDelay1);
   RunAccelerator(3);
   Assert_Eq(3,accelState->TOP_output_output_currentValue);   


   ActivateMergedAccelerator(MergeType_TestMergeDelay2);
   RunAccelerator(3);
   Assert_Eq(5,accelState->TOP_output_output_currentValue);
}