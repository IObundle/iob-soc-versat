#include "testbench.hpp"

void ConfigureVariety1(Variety1Config* config){
   int memory = 3;

   ConfigureSimpleVRead(&config->read,1,&memory);

   ConfigureMemoryLinear(&config->mem,1,0);
   VersatUnitWrite(TOP_simple_mem_addr,0,2);

   config->constant.constant = 1;
}

#ifndef UNIT_VARIETY_1
void SingleTest(Arena* arena){
   ACCEL_TOP_input_0_constant = 5;
   ACCEL_TOP_input_1_constant = 4;
   ConfigureVariety1((Variety1Config*) &accelConfig->TOP_simple_read_ext_addr);

   RunAccelerator(3);

   Assert_Eq(15,ACCEL_TOP_output_0_currentValue);
}
#endif