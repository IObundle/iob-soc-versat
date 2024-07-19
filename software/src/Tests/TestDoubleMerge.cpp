#include "testbench.hpp"

void SingleTest(Arena* arena){
   TestDoubleMerge_SimpleConfig* config = (TestDoubleMerge_SimpleConfig*) accelConfig;
   AcceleratorState* state = (AcceleratorState*) accelState;
   TestDoubleMerge_SimpleAddr addr = ACCELERATOR_TOP_ADDR_INIT;

   ActivateMergedAccelerator(MergeType_TestDoubleMerge0_TestDoubleMerge00);

   config->simple.TestDoubleMerge0_TestDoubleMerge00.x00.constant = 10;
   config->simple.TestDoubleMerge0_TestDoubleMerge00.y00.constant = 5;

   RunAccelerator(1);

   printf("15 : %d\n",accelState->TOP_output_0_currentValue);

   ActivateMergedAccelerator(MergeType_TestDoubleMerge0_TestDoubleMerge01);

   int x[] = {7};
   int y[] = {20};

   ConfigureSimpleMemory(&config->simple.TestDoubleMerge0_TestDoubleMerge01.x01,1,0,addr.simple.x01,x);
   ConfigureSimpleMemory(&config->simple.TestDoubleMerge0_TestDoubleMerge01.y01,1,0,addr.simple.y01,y);

   RunAccelerator(1);

   printf("27 : %d\n",accelState->TOP_output_0_currentValue);

   ActivateMergedAccelerator(MergeType_TestDoubleMerge1_TestDoubleMerge10);

   config->simple.TestDoubleMerge1_TestDoubleMerge10.y10.constant = 5;
   ConfigureSimpleMemory(&config->simple.TestDoubleMerge1_TestDoubleMerge10.x10,1,0,addr.simple.x10_y11,x);

   RunAccelerator(1);

   printf("-2 : %d\n",accelState->TOP_output_0_currentValue);

   ActivateMergedAccelerator(MergeType_TestDoubleMerge1_TestDoubleMerge11);
   
   ConfigureSimpleMemory(&config->simple.TestDoubleMerge1_TestDoubleMerge11.y11,1,0,addr.simple.x10_y11,y);
   config->simple.TestDoubleMerge1_TestDoubleMerge11.x11.constant = 2;

   RunAccelerator(1);

   printf("18 : %d\n",accelState->TOP_output_0_currentValue);
}