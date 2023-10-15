#include "testbench.h"

void SingleTest(Arena* arena){
   iptr input[] = {0x04,0xe0,0x48,0x28,0x66,0xcb,0xf8,0x06,0x81,0x19,0xd3,0x26,0xe5,0x9a,0x7a,0x4c,0xa0,0x88,0x23,0x2a,0xfa,0x54,0xa3,0x6c,0xfe,0x2c,0x39,0x76,0x17,0xb1,0x39,0x05};
   VersatMemoryCopy(SimpleInputStart,input,ARRAY_SIZE(input));

   RunAccelerator(1);

   int expected[] = {0xa4,0x68,0x6b,0x02,0x9c,0x9f,0x5b,0x6a,0x7f,0x35,0xea,0x50,0xf2,0x2b,0x43,0x49};

   Assert_Eq_Array(expected,SimpleOutputStart,ARRAY_SIZE(expected));
}









