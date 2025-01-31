#include "testbench.hpp"

void SingleTest(Arena* arena){
  int numberItems = 4;

  accelConfig->start.iterA = 1;
  accelConfig->start.incrA = 1;
  accelConfig->start.dutyA = ~0;
  accelConfig->start.perA = numberItems;

  for(int i = 0; i < numberItems; i++){
    VersatUnitWrite(TOP_start_addr,i,i + 1);
  }

  accelConfig->end.iterA = 1;
  accelConfig->end.incrA = 1;
  accelConfig->end.dutyA = ~0;
  accelConfig->end.in0_wr = 1;
  accelConfig->end.perA = numberItems;

  RunAccelerator(3);
  
  for(int i = 0; i < numberItems; i++){
    int read = VersatUnitRead(TOP_end_addr,i);
    Assert_Eq(i + 1,read);
  }
}
