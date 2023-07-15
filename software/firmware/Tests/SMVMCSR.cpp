#include "SMVM.hpp"

void PrintReceived(){
   int received = ACCEL_TOP_output_stored;
   printf("Received: %d\n",received);

   for(int i = 0; i < received; i++){
      printf("%d ",VersatUnitRead(TOP_output_addr,i));
   }
   printf("\n");
}

void SingleTest(Arena* arena){
   InitializeSMVM(arena,Type::CSR);
   
   int digitSize = std::max(DigitSize(csr.row),
                            std::max(DigitSize(csr.column),DigitSize(csr.values)));
     
   ACCEL_TOP_cycler_amount = size + 24;

   //ConfigureGenerator(gen,0,csr.values.size + 2,1);
   {
      int start = 0;
      int range = csr.values.size;
      int increment = 1;

      ACCEL_TOP_gen_iterations = 1;
      ACCEL_TOP_gen_period = range;
      ACCEL_TOP_gen_duty = range;
      ACCEL_TOP_gen_start = start;
      ACCEL_TOP_gen_shift = 1;
      ACCEL_TOP_gen_incr = increment;
   }
   
   //ConfigureSimpleVRead(col,csr.column.size,csr.column.data);
   // Memory side
   {
      int numberItems = csr.column.size;
      int* data = csr.column.data;

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
      for(int i = 0; i < csr.row.size; i++){
         VersatUnitWrite(TOP_row_addr + i,csr.row[i]);
      }
   }

   //ConfigureSimpleVRead(val,csr.values.size,csr.values.data);
   // Memory side
   {
      int numberItems = csr.values.size;
      int* data = csr.values.data;

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

   int received = ACCEL_TOP_output_stored;
   printf("Received: %d\n",received);

   for(int i = 0; i < received; i++){
      PushGotI(VersatUnitRead(TOP_output_addr,i));
   }   
}

   
