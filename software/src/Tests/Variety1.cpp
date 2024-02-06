#include "testbench.hpp"

void ConfigureVariety1(Variety1Config* config,int* memory){
   ConfigureSimpleVRead(&config->read,1,memory);

   ConfigureSimpleMemory(&config->mem,1,0);
   VersatUnitWrite(TOP_simple_mem_addr,0,2);

   config->constant.constant = 1;
}

#ifndef UNIT_VARIETY_1
void SingleTest(Arena* arena){
   int memory = 3;

   ACCEL_TOP_input_0_constant = 5;
   ACCEL_TOP_input_1_constant = 4;

   Variety1_SimpleConfig* accel = (Variety1_SimpleConfig*) accelConfig;

   ConfigureVariety1(&accel->simple,&memory);

   RunAccelerator(3);

   Assert_Eq(15,ACCEL_TOP_output_0_currentValue);
}
#endif