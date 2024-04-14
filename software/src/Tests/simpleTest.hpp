#include "testbench.hpp"
#include "versat_accel.h"

#include "cassert"
#include "cstdlib"

static void SimpleTest(int* output){
   assert(isSimpleAccelerator);
   srand(0);

   for(int i = 0; i < NumberSimpleInputs; i++){
      SimpleInputStart[i] = rand();
   }

   RunAccelerator(1);

   if(!output){
     printf("int output[] = {");
   }

   for(int i = 0; i < NumberSimpleOutputs; i++){
      if(!output){
         printf("0x%x",SimpleOutputStart[i]);
         if(i + 1 != NumberSimpleOutputs){
            printf(",");         
         }
      }
      if(output){
         Assert_Eq(SimpleOutputStart[i],output[i]);
      }
   }

   if(!output){
      printf("};\n");
   }
}
