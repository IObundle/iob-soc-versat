#include "testbench.hpp"

void SingleTest(Arena* arena){
  int numberItems = 4;

  ACCEL_TOP_start_iterA = 1;
  ACCEL_TOP_start_incrA = 1;
  ACCEL_TOP_start_dutyA = ~0;
  ACCEL_TOP_start_perA = numberItems;

  for(int i = 0; i < numberItems; i++){
    VersatUnitWrite(TOP_start_addr,i,i + 1);
  }

  ACCEL_TOP_end_iterA = 1;
  ACCEL_TOP_end_incrA = 1;
  ACCEL_TOP_end_dutyA = ~0;
  ACCEL_TOP_end_in0_wr = 1;
  ACCEL_TOP_end_perA = numberItems;

  RunAccelerator(1);
  
  for(int i = 0; i < numberItems; i++){
    int read = VersatUnitRead(TOP_end_addr,i);
    Assert_Eq(read,i + 1);
  }
}
