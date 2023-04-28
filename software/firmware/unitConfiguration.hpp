#ifndef INCLUDED_UNIT_CONFIGURATION
#define INCLUDED_UNIT_CONFIGURATION

#include "versat.hpp"

void ConfigureSimpleVRead(FUInstance* inst, int numberItems,int* memory);
void ConfigureSimpleVWrite(FUInstance* inst, int numberItems,int* memory);
void ConfigureLeftSideMatrix(FUInstance* inst,int iterations);
void ConfigureRightSideMatrix(FUInstance* inst, int iterations);
void ConfigureGenerator(FUInstance* inst, int start,int end,int increment);
void ConfigureMemoryLinear(FUInstance* inst, int amountOfData,int start = 0);
void ConfigureMemoryReceive(FUInstance* inst, int amountOfData,int interdataDelay);
void ConfigureLeftSideMatrixVRead(FUInstance* inst, int iterations,void* data);
void ConfigureRightSideMatrixVRead(FUInstance* inst, int iterations,void* data);
void ConfigureMatrixVWrite(FUInstance* inst,int amountOfData,void* data);

#endif // INCLUDED_UNIT_CONFIGURATION
