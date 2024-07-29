#include "testbench.hpp"

#include "unitConfiguration.hpp"

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

#define SIZE 1234
//#define SIZE 1536

void SingleTest(Arena* arena){
  int* inputBuffer = (int*) PushBytes(arena,sizeof(int) * SIZE * 2);
  int* outputBuffer = (int*) PushBytes(arena,sizeof(int) * SIZE * 2);

  VReadToVWriteConfig* con = (VReadToVWriteConfig*) accelConfig;

  for(int i = 0; i < 4; i++){
    printf("Loop: %d\n",i);
    int* input = &inputBuffer[i];
    int* output = &outputBuffer[i];

    printf("%p %p %p\n",arena->mem,input,output);

    for(int i = 0; i < SIZE * 2; i++){
      input[i] = i + 1;
    }

    printf("%d\n",0);
  
    int numberItems = SIZE;
    ConfigureSimpleVRead(&con->read,SIZE,input);
    printf("%d\n",1);
    ConfigureSimpleVWrite(&con->write,SIZE,output);
    
    con->write.enableWrite = 0;

    printf("%d\n",1);
  
    RunAccelerator(1);

    printf("%d\n",2);
    con->read.enableRead = 0;

    RunAccelerator(1);

    printf("%d\n",3);
    con->write.enableWrite = 1;

    RunAccelerator(1);

    printf("%d\n",4);

    ClearCache();

    printf("%d\n",5);

    bool equal = true;     
    for(int i = 0; i < numberItems; i++){
      if(input[i] != output[i]){
        equal = false;
        printf("Different at %d: %d %d\n",i,input[i],output[i]);
      }
    }

    printf("%d\n",6);
 
    if(equal){
      Assert_Eq(1,1);
    } else {
      Assert_Eq(-1,i);
    }
    printf("%d\n",7);
  }
}
