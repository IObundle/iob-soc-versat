#include "SMVM.h"

void PrintReceived(){
   int received = ACCEL_TOP_output_stored;
   printf("Received: %d\n",received);

   for(int i = 0; i < received; i++){
      printf("%d ",VersatUnitRead(TOP_output_addr,i));
   }
   printf("\n");
}

void SingleTest(){
   Arena tempInst = InitArena(Megabyte(1));
   Arena* temp = &tempInst;

   Array<int> mat = ExampleMatrix(temp);
   FormatCSR res = ConvertCSR(mat,5,temp);

   int digitSize = std::max(DigitSize(res.row),
                            std::max(DigitSize(res.column),DigitSize(res.values)));

   ACCEL_TOP_cycler_amount = size + 24;

   //ConfigureGenerator(gen,0,res.values.size + 2,1);
   {
      int start = 0;
      int range = res.values.size + 2;
      int increment = 1;

      ACCEL_TOP_gen_iterations = 1;
      ACCEL_TOP_gen_period = range;
      ACCEL_TOP_gen_duty = range;
      ACCEL_TOP_gen_start = start;
      ACCEL_TOP_gen_shift = 1;
      ACCEL_TOP_gen_incr = increment;
   }
   
   //ConfigureSimpleVRead(col,res.column.size,res.column.data);
   // Memory side
   {
      int numberItems = res.column.size;
      int* data = res.column.data;

      ACCEL_TOP_col_incrA = 1;
      ACCEL_TOP_col_iterA = 1;
      ACCEL_TOP_col_perA = numberItems;
      ACCEL_TOP_col_dutyA = numberItems;
      ACCEL_TOP_col_size = 8;
      ACCEL_TOP_col_int_addr = 0;
      ACCEL_TOP_col_pingPong = 1;

      // B - versat side
      ACCEL_TOP_col_iterB = numberItems;
      ACCEL_TOP_col_incrB = 1;
      ACCEL_TOP_col_perB = 1;
      ACCEL_TOP_col_dutyB = 1;
      ACCEL_TOP_col_ext_addr = (iptr) data;
      ACCEL_TOP_col_length = numberItems - 1; // AXI requires length of len - 1
   }
   
   // Memory side
   {
      ACCEL_TOP_row_amount = size;
      for(int i = 0; i < res.row.size; i++){
         VersatUnitWrite(TOP_row_addr + i,res.row[i]);
      }
   }

   //ConfigureSimpleVRead(val,res.values.size,res.values.data);
   // Memory side
   {
      int numberItems = res.values.size;
      int* data = res.values.data;

      ACCEL_TOP_val_incrA = 1;
      ACCEL_TOP_val_iterA = 1;
      ACCEL_TOP_val_perA = numberItems;
      ACCEL_TOP_val_dutyA = numberItems;
      ACCEL_TOP_val_size = 8;
      ACCEL_TOP_val_int_addr = 0;
      ACCEL_TOP_val_pingPong = 1;

      // B - versat side
      ACCEL_TOP_val_iterB = numberItems;
      ACCEL_TOP_val_incrB = 1;
      ACCEL_TOP_val_perB = 1;
      ACCEL_TOP_val_dutyB = 1;
      ACCEL_TOP_val_ext_addr = (iptr) data;
      ACCEL_TOP_val_length = numberItems - 1; // AXI requires length of len - 1
   }

   //ConfigureSimpleVRead(vec,vector.size,vector.data);
   // Memory side
   {
      int numberItems = vec.size;
      int* data = vec.data;

      ACCEL_TOP_vector_incrA = 1;
      ACCEL_TOP_vector_iterA = 1;
      ACCEL_TOP_vector_perA = numberItems;
      ACCEL_TOP_vector_dutyA = numberItems;
      ACCEL_TOP_vector_size = 8;
      ACCEL_TOP_vector_int_addr = 0;
      ACCEL_TOP_vector_pingPong = 1;

      // B - versat side
      ACCEL_TOP_vector_iterB = numberItems;
      ACCEL_TOP_vector_incrB = 1;
      ACCEL_TOP_vector_perB = 1;
      ACCEL_TOP_vector_dutyB = 1;
      ACCEL_TOP_vector_ext_addr = (iptr) data;
      ACCEL_TOP_vector_length = numberItems - 1; // AXI requires length of len - 1
   }

   RunAccelerator(1); // Load initial data

   RunAccelerator(1);

   #if 1
   {
      Array<int> arr = MultiplyCSR(res,vec,temp);

      printf("Expected: ");
      Print(arr);
      printf("\n");
      
      PrintReceived();
   }
   #endif
}

   
