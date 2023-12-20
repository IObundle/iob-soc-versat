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
int outputBuffer[SIZE * 4];

// ClearCache is very important otherwise sim-run might read past values
void ClearCache(){
#ifndef PC
  int size = 1024 * 16;
  char* m = (char*) malloc(size); // Should not use malloc but some random fixed ptr in embedded. No use calling malloc since we can always read at any point in memory without worrying about memory protection.

  // volatile and asm are used to make sure that gcc does not optimize away this loop that appears to do nothing
  volatile int val = 0;
  for(int i = 0; i < size; i += 16){
    val += m[i];
    __asm__ volatile("" : "+g" (val) : :);
  }
  free(m);
#endif
}

void SingleTest(Arena* arena){
  int inputBuffer[SIZE * 4];

  for(int i = 0; i < 4; i++){
    printf("Loop: %d\n",i);
    int* output = &outputBuffer[i];
    int* input = &inputBuffer[i];

    for(int i = 0; i < SIZE * 2; i++){
      input[i] = i + 1;
    }
    
    int numberItems = SIZE;

    // Read side
    ACCEL_TOP_read_incrA = 1;
    //ACCEL_TOP_read_iterA = 1;
    //ACCEL_TOP_read_dutyA = ~0;
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
    //ACCEL_TOP_write_iterA = 1;
    //ACCEL_TOP_write_dutyA = ~0;
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
