#include "testbench.hpp"

void SingleTest(){
   #if 1
   int input[] = {1000,200,30,4};
   int output[ARRAY_SIZE(input)];

   int numberItems = ARRAY_SIZE(input);

   // Memory side
   ACCEL_TOP_read_incrA = 1;
   ACCEL_TOP_read_iterA = 1;
   ACCEL_TOP_read_perA = numberItems;
   ACCEL_TOP_read_dutyA = numberItems;
   ACCEL_TOP_read_size = 8;
   ACCEL_TOP_read_int_addr = 0;
   ACCEL_TOP_read_pingPong = 1;

   // B - versat side
   ACCEL_TOP_read_iterB = numberItems;
   ACCEL_TOP_read_incrB = 1;
   ACCEL_TOP_read_perB = 1;
   ACCEL_TOP_read_dutyB = 1;
   ACCEL_TOP_read_ext_addr = (iptr) input;
   ACCEL_TOP_read_length = numberItems - 1; // AXI requires length of len - 1

   // Write side
   ACCEL_TOP_write_incrA = 1;
   ACCEL_TOP_write_iterA = 1;
   ACCEL_TOP_write_perA = numberItems;
   ACCEL_TOP_write_dutyA = numberItems;
   ACCEL_TOP_write_size = 4;
   ACCEL_TOP_write_int_addr = 0;
   ACCEL_TOP_write_pingPong = 1;
   ACCEL_TOP_write_length = numberItems - 1;
   ACCEL_TOP_write_ext_addr =(iptr)  output;

   // Memory side
   ACCEL_TOP_write_iterB = numberItems;
   ACCEL_TOP_write_perB = 1;
   ACCEL_TOP_write_dutyB = 1;
   ACCEL_TOP_write_incrB = 1;

   RunAccelerator(3);

   for(int i = 0; i < numberItems; i++){
      Assert_Eq(input[i],output[i]);
   }
   #endif
}
