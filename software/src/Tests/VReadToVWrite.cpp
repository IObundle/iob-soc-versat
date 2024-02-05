#include "testbench.hpp"

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

#define SIZE 8

void SingleTest(Arena* arena){
  int* outputBuffer = (int*) PushBytes(arena,sizeof(int) * SIZE * 2);
  int* inputBuffer = (int*) PushBytes(arena,sizeof(int) * SIZE * 2);

  for(int i = 0; i < 4; i++){
    printf("Loop: %d\n",i);
    int* output = &outputBuffer[i];
    int* input = &inputBuffer[i];

    printf("%p %p %p\n",arena->mem,output,input);

    for(int i = 0; i < SIZE * 2; i++){
      input[i] = i + 1;
    }
    
    int numberItems = SIZE;

    // Read side
    ACCEL_TOP_read_incrA = 1;
    ACCEL_TOP_read_ext_addr = (iptr) input;
    ACCEL_TOP_read_perA = numberItems;
    ACCEL_TOP_read_length = numberItems * sizeof(int);
    ACCEL_TOP_read_pingPong = 1;
   
    // Dataflow side
    ACCEL_TOP_read_iterB = 1;
    ACCEL_TOP_read_incrB = 1;
    ACCEL_TOP_read_perB = numberItems;
    ACCEL_TOP_read_dutyB = ~0;

    // Write side
    ACCEL_TOP_write_incrA = 1;
    ACCEL_TOP_write_perA = numberItems;
    ACCEL_TOP_write_length = numberItems * sizeof(int);
    ACCEL_TOP_write_ext_addr = (iptr) output;
    ACCEL_TOP_write_pingPong = 1;
   
    // Dataflow side
    ACCEL_TOP_write_iterB = 1;
    ACCEL_TOP_write_incrB = 1;
    ACCEL_TOP_write_dutyB = ~0;
    ACCEL_TOP_write_perB = numberItems;

    RunAccelerator(3);

    ClearCache();
     
    for(int i = 0; i < numberItems; i++){
      Assert_Eq(input[i],output[i]);
    }
  }
}
