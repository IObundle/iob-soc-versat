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
   InitializeSMVM(arena,Type::COO);
   
   int digitSize = std::max(DigitSize(coo.row),
                            std::max(DigitSize(coo.column),DigitSize(coo.values)));
   
   ACCEL_TOP_cycler_amount = size + 24;
   
   //ConfigureGenerator(gen,0,coo.values.size + 2,1);
   {
      int start = 0;
      int range = coo.values.size + 2;
      int increment = 1;

      ACCEL_TOP_gen_iterations = 1;
      ACCEL_TOP_gen_period = range;
      ACCEL_TOP_gen_duty = range;
      ACCEL_TOP_gen_start = start;
      ACCEL_TOP_gen_shift = 1;
      ACCEL_TOP_gen_incr = increment;
   }
   
   //ConfigureSimpleVRead(col,coo.column.size,coo.column.data);
   // Memory side
   {
      printf("Col: %p\n",coo.column.data);
      int numberItems = coo.column.size;
      int* data = coo.column.data;

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
   
   //ConfigureSimpleVRead(row,coo.row.size,coo.row.data);
   // Memory side
   {
      printf("Row: %p\n",coo.row.data);
      int numberItems = coo.row.size;
      int* data = coo.row.data;

      ACCEL_TOP_row_incrA = 1;
      ACCEL_TOP_row_iterA = 1;
      ACCEL_TOP_row_perA = numberItems;
      ACCEL_TOP_row_dutyA = numberItems;
      ACCEL_TOP_row_size = 8;
      ACCEL_TOP_row_int_addr = 0;
      ACCEL_TOP_row_pingPong = 1;

      // B - versat side
      ACCEL_TOP_row_iterB = numberItems;
      ACCEL_TOP_row_incrB = 1;
      ACCEL_TOP_row_perB = 1;
      ACCEL_TOP_row_dutyB = 1;
      ACCEL_TOP_row_ext_addr = (iptr) data;
      ACCEL_TOP_row_length = numberItems - 1; // AXI requires length of len - 1

      Assert(numberItems - 1 <= 0xff);
   }

   //ConfigureSimpleVRead(val,coo.values.size,coo.values.data);
   // Memory side
   {
      int numberItems = coo.values.size;
      int* data = coo.values.data;

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

      Assert(numberItems - 1 <= 0xff);
   }

   //ConfigureSimpleVRead(vec,vec.size,vec.data);
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

      Assert(numberItems - 1 <= 0xff);
   }

   RunAccelerator(1);
  
   RunAccelerator(1);

   int received = ACCEL_TOP_output_stored;
   for(int i = 0; i < received; i++){
      PushGotI(VersatUnitRead(TOP_output_addr,i));
   }
      
}
