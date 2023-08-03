#if 0

#include "testbench.h"

void SingleTest(){
   float f1 = 0.275f;
   float f2 = 0.850241363049f;
   
   ACCEL_TOP_input_0_constant = PackInt(f1);
   ACCEL_TOP_input_1_constant = PackInt(f2);
     
   RunAccelerator(1);
  
   Assert_EqF(f1 * f2,PackFloat(ACCEL_TOP_output_0_currentValue));
}
#endif

int main(int argc,const char* argv[]){
  return 0;
}
