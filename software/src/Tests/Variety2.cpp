#include "testbench.hpp"

void ConfigureVariety2(Variety2Config* config,int* output){
   int memory = 2;

   ConfigureSimpleVRead(&config->read,1,&memory);

   ConfigureMemoryLinear(&config->memButDiffName,1,0);
   VersatUnitWrite(TOP_simple_memButDiffName_addr,0,1);
   ConfigureSimpleVWrite(&config->write,1,output);
}

#ifndef UNIT_VARIETY_2
void SingleTest(Arena* arena){
   ACCEL_TOP_input_0_constant = 5;
   ACCEL_TOP_input_1_constant = 4;
   ACCEL_TOP_input_2_constant = 3;
   int output = 0;
   ConfigureVariety2((Variety2Config*) &accelConfig->TOP_simple_read_ext_addr,&output);

   RunAccelerator(3);

   Assert_Eq(15,output);
}
#endif