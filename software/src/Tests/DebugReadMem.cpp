#include "testbench.hpp"

void SingleTest(Arena* arena){
   int memToRead = 123;

   ACCEL_TOP_address = (iptr) &memToRead;

   RunAccelerator(1);

   printf("%d\n",ACCEL_TOP_lastRead);
}
