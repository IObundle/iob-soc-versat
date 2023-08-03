#include "testbench.h"

void SingleTest(Arena* arena){
   #if 1
   #define SIZE 512

   int inputBuffer[SIZE * 2];
   int output[SIZE * 2];

   int* view = inputBuffer;
   for(int i = 0; i < SIZE; i++){
      *view = i + 1;
   }

   int numberItems = SIZE;

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
   ACCEL_TOP_read_ext_addr = (iptr) inputBuffer;
   ACCEL_TOP_read_length = numberItems * sizeof(int);

   // Write side
   ACCEL_TOP_write_incrA = 1;
   ACCEL_TOP_write_iterA = 1;
   ACCEL_TOP_write_perA = numberItems;
   ACCEL_TOP_write_dutyA = numberItems;
   ACCEL_TOP_write_size = 4;
   ACCEL_TOP_write_int_addr = 0;
   ACCEL_TOP_write_pingPong = 1;
   ACCEL_TOP_write_length = numberItems * sizeof(int);
   ACCEL_TOP_write_ext_addr =(iptr)  output;

   // Memory side
   ACCEL_TOP_write_iterB = numberItems;
   ACCEL_TOP_write_perB = 1;
   ACCEL_TOP_write_dutyB = 1;
   ACCEL_TOP_write_incrB = 1;

   RunAccelerator(3);

   for(int i = 0; i < numberItems; i++){
      Assert_Eq(inputBuffer[i],output[i]);
   }
   #endif
}
