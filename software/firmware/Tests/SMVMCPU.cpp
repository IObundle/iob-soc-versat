#include "SMVM.hpp"

void SingleTest(){
   float f1 = 0.275f;
   float f2 = 0.850241363049f;
   
   ACCEL_TOP_input_0_constant = PackInt(f1);
   ACCEL_TOP_input_1_constant = PackInt(f2);
     
   RunAccelerator(1);
   
   Assert_EqF(f1 * f2,PackFloat(ACCEL_TOP_output_0_currentValue));

#if 0
   Arena tempInst = InitArena(Megabyte(1));
   Arena* arena = &tempInst;

   Array<int> mat = ExampleMatrix(arena);
   FormatCSR csr = ConvertCSR(mat,5,arena);
   
   int size = csr.size;
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   ACCEL_TOP_simple_mul_amount = 1;

   for(int i = 0; i < csr.size; i++){
      SignalLoop(); //  int val = 0;

      for(int ii = csr.row[i]; ii < csr.row[i+1]; ii++){
         ACCEL_TOP_input_0_constant = csr.values[ii];
         ACCEL_TOP_input_1_constant = vec[csr.column[ii]];

         RunAccelerator(1);
      }

      res[i] = ACCEL_TOP_output_0_currentValue;
   }

   PrintArray(res,2);
   printf("\n");
#endif
}

#if 0
   SignalLoop();

   ACCEL_TOP_simple_mul_amount = 1;
   ACCEL_TOP_input_0_constant = 5;
   ACCEL_TOP_input_1_constant = 5;

   RunAccelerator(1);

   ACCEL_TOP_input_0_constant = 10;
   ACCEL_TOP_input_1_constant = 10;

   RunAccelerator(1);
   
   printf("%d\n",ACCEL_TOP_output_0_currentValue);

   SignalLoop();

   RunAccelerator(1);
   
   printf("%d\n",ACCEL_TOP_output_0_currentValue);
#endif
   
#if 0
   FormatCSR csr = ConvertCSR(mat,5,arena);

   // CPU. I set some variables and the thing works

   int digitSize = std::max(DigitSize(csr.row),
                            std::max(DigitSize(csr.column),DigitSize(csr.values)));
   
   PrintArray(csr.row,digitSize);
   printf("\n");
   PrintArray(csr.column,digitSize);
   printf("\n");
   PrintArray(csr.values,digitSize);   

   Array<int> mul = MultiplyCSR(csr,vec,arena);

   printf("\n");
   PrintArray(mul,2);
#endif

   
