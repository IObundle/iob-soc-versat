#include "testbench.hpp"

void ConfigureVariety2(volatile EXAMPLE_Variety2Config* config,int* output,int* memory){
   ConfigureSimpleVRead(&config->read,1,memory);

   ConfigureSimpleMemory(&config->mem,1,0);
   VersatUnitWrite(TOP_simple_mem_addr,0,1);
   ConfigureSimpleVWrite(&config->write,1,output);
}

void SingleTest(Arena* arena){
   int memory = 2;
   int* output = (int*) malloc(sizeof(int));

   accelConfig->input_0.constant = 5;
   accelConfig->input_1.constant = 4;
   accelConfig->input_2.constant = 3;
   ConfigureVariety2(&accelConfig->simple,output,&memory);

   RunAccelerator(3);

   Assert_Eq(15,*output);
}