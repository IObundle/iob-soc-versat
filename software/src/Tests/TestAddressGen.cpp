#define ADDRESS_GEN_TestAddressGen
#include "testbench.hpp"

void SingleTest(Arena* arena){
   int xMax = 2;
   int yMax = 2;
   int zMax = 2;
   int wMax = 2;
   int aMax = 2;

   Configure_TestAddressGen(&accelConfig->gen,xMax,yMax,zMax,wMax,aMax);
   int loopSize = LoopSize_TestAddressGen(0,xMax,yMax,zMax,wMax,aMax);

   ConfigureMemoryReceive(&accelConfig->output,loopSize);

   RunAccelerator(3);

   for(int i = 0; i < loopSize; i++){
      printf("%d ",VersatUnitRead(TOP_output_addr,i));
   }
}