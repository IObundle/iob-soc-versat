#include "testbench.h"

#include <type_traits>

// Given a ptr returns it, given an array returns the pointer type
template<class T>
struct CollapseArray{
  typedef T type;
};
template<class T>
struct CollapseArray<T[]>{
  typedef T* type;
};
template<class T, std::size_t N>
struct CollapseArray<T[N]>{
  typedef T* type;
};

#define ADD_BYTE_TO_PTR(PTR,BYTES) ((CollapseArray<decltype(PTR)>::type) (((char*) PTR) + BYTES))

#define SIZE 4
int outputBuffer[SIZE * 2];

void ClearCache(){
  // 16 works
  int size = 1024 * 8;
  char* m = (char*) malloc(size);

  int val = 0;
  for(int i = 0; i < size; i += 8){ // 8 increment works
    val += m[i];
  }
  printf("Got %d\n",val);
}

void SingleTest(Arena* arena){
   #if 1

   int inputBuffer[SIZE * 2];

   for(int i = 0; i < 2; i++){
#if 0
   int* output = ADD_BYTE_TO_PTR(outputBuffer,4 * i);
   int* input = ADD_BYTE_TO_PTR(inputBuffer,4 * i);
#else
  int* output = &outputBuffer[i];
  int* input = &inputBuffer[i];
#endif
  
   printf("I:%x %x\n",inputBuffer,input);
   printf("O:%x %x\n",outputBuffer,output);  
  
   for(int i = 0; i < SIZE * 2; i++){
      input[i] = i + 1;
   }

   ClearCache();
     
   int numberItems = SIZE;

   // Memory side
   ACCEL_TOP_read_incrA = 1;
   ACCEL_TOP_read_perA = numberItems;
   ACCEL_TOP_read_pingPong = 1;
   //ACCEL_TOP_read_dutyA = ~0;
   //ACCEL_TOP_read_shiftA = 1;
   //ACCEL_TOP_read_iterA = 1;
   //ACCEL_TOP_read_size = 8;
   //ACCEL_TOP_read_int_addr = 0;
   
   // B - versat side
   ACCEL_TOP_read_iterB = numberItems;
   ACCEL_TOP_read_incrB = 1;
   ACCEL_TOP_read_perB = 1;
   ACCEL_TOP_read_dutyB = 1;
   ACCEL_TOP_read_ext_addr = (iptr) input;
   ACCEL_TOP_read_length = numberItems * sizeof(int);

   // Write side
   ACCEL_TOP_write_incrA = 1;
   ACCEL_TOP_write_perA = numberItems ;
   ACCEL_TOP_write_pingPong = 1;
   ACCEL_TOP_write_length = numberItems * sizeof(int);
   ACCEL_TOP_write_ext_addr = (iptr) output;
   //ACCEL_TOP_write_iterA = 1;
   //ACCEL_TOP_write_shiftA = 1;
   //ACCEL_TOP_write_dutyA = ~0;
   //ACCEL_TOP_write_size = 4;
   //ACCEL_TOP_write_int_addr = 0;
   
   // Memory side
   ACCEL_TOP_write_iterB = numberItems;
   ACCEL_TOP_write_perB = 1;
   ACCEL_TOP_write_dutyB = 1;
   ACCEL_TOP_write_incrB = 1;

   printf("\n");
   RunAccelerator(1);
   printf("\n");
   RunAccelerator(1);
   printf("\n");
   RunAccelerator(1);
   printf("\n");

   ClearCache();
     
#if 1
   for(int i = 0; i < numberItems; i++){
	 printf("%d: %d - %d\n",i,input[i],output[i]);
     if(input[i] != output[i]){
	   printf("%d: %d - %d\n",i,input[i],output[i]);
	 }
   }
#endif
}
#endif
}
