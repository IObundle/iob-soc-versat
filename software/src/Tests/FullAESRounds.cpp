#include "testbench.hpp"

void SingleTest(Arena* arena){
   ActivateMergedAccelerator(AESFirstAdd);

   iptr inputs[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};

   for(int i = 0; i < 32; i++){
      SimpleInputStart[i] = inputs[i];
   }

   RunAccelerator(1);

   for(int i = 0; i < 16; i++){
      printf("%d ,",SimpleOutputStart[i]);
   }
   printf("\n");

}