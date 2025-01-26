#include "testbench.hpp"

void ConfigureVariety2(volatile Variety2Config* config,int* output,int* memory){
   ConfigureSimpleVRead(&config->read,1,memory);

   ConfigureSimpleMemory(&config->memButDiffName,1,0);
   VersatUnitWrite(TOP_simple_memButDiffName_addr,0,1);
   ConfigureSimpleVWrite(&config->write,1,output);
}

#ifndef UNIT_VARIETY_2
void SingleTest(Arena* arena){
   int memory = 2;
   int* output = (int*) malloc(sizeof(int));

   ACCEL_TOP_input_0_constant = 5;
   ACCEL_TOP_input_1_constant = 4;
   ACCEL_TOP_input_2_constant = 3;
   ConfigureVariety2(&accelConfig->simple,output,&memory);

   RunAccelerator(3);

   Assert_Eq(15,*output);
}
#endif