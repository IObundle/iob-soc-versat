#define ADDRESS_GEN_VReadMultipleLoad
#define ADDRESS_GEN_VReadMultipleOutput
#define ADDRESS_GEN_VWriteMultipleStore
#define ADDRESS_GEN_VWriteMultipleInput

#include "testbench.hpp"

void SingleTest(Arena* arena){
  int input[] = {1,2,3,4,0,0,0,0,5,6,7,8,0,0,0,0,9,10,11,12,0,0,0,0,13,14,15,16};
  int expectedOutput[] = {1,2,3,4,0,0,5,6,7,8,0,0,9,10,11,12,0,0,13,14,15,16};
  int outputBuffer[] =   {0,0,0,0,0,0,0,0,0,0,0,0,0, 0, 0, 0,0,0, 0, 0, 0, 0};

  // Input reads 4 in 4 with a stride of 4;
  // Output writes 4 in 4 with a stride of 2;

  Configure_VReadMultipleLoad(&accelConfig->read,(iptr) input,4,4,8 * sizeof(int));
  Configure_VReadMultipleOutput(&accelConfig->read,4,4);
  accelConfig->read.read_enabled = 1;

  Configure_VWriteMultipleStore(&accelConfig->write,(iptr) outputBuffer,4,4,6 * sizeof(int));
  Configure_VWriteMultipleInput(&accelConfig->write,4,4);
  accelConfig->write.write_enabled = 1;

  RunAccelerator(3);

  for(int i = 0; i < 22; i++){
    Assert_Eq(expectedOutput[i],outputBuffer[i]);
    //printf("%d ",outputBuffer[i]);
  }
  //printf("\n");
}
