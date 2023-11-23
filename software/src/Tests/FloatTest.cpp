#include "testbench.hpp"

#include <cmath>

void SingleTest(Arena* arena){
  SeedRandomNumber(1);
  
  for(int i = 0; i < 1000; i++){
    unsigned int rand1 = (GetRandomNumber() % 1000);
    unsigned int rand2 = ((GetRandomNumber() % 1000) + 1);

    float val1 = (float) rand1;
    float val2 = (float) rand2;

    ACCEL_TOP_input_0_constant = PackInt(val1);
    ACCEL_TOP_input_1_constant = PackInt(val2);
    ACCEL_TOP_input_2_constant = rand1;
  
    RunAccelerator(1);

    float res1 = PackFloat(ACCEL_TOP_output_0_currentValue); // add
    float res2 = PackFloat(ACCEL_TOP_output_1_currentValue); // sub
    float res3 = PackFloat(ACCEL_TOP_output_2_currentValue); // mul
    float res4 = PackFloat(ACCEL_TOP_output_3_currentValue); // div
    float res5 = PackFloat(ACCEL_TOP_output_4_currentValue); // max
    float res6 = PackFloat(ACCEL_TOP_output_5_currentValue); // min
    float res7 = PackFloat(ACCEL_TOP_output_6_currentValue); // sqrt
    float res8 = PackFloat(ACCEL_TOP_output_7_currentValue); // int2float
    unsigned int res9 = (unsigned int) ACCEL_TOP_output_8_currentValue;  // float2int

    Assert_Eq(val1 + val2,res1,"+");
    Assert_Eq(val1 - val2,res2,"-");
    Assert_Eq(val1 * val2,res3,"*");
    Assert_Eq(val1 / val2,res4,"/");
    Assert_Eq(std::max(val1,val2),res5,"max");
    Assert_Eq(std::min(val1,val2),res6,"min");
    Assert_Eq(std::sqrt(val1),res7,"sqrt");
    Assert_Eq(val1,res8,"i2f");
    Assert_Eq(rand1,res9,"f2i");
  }
}

