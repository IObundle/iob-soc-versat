#ifndef INCLUDED_UNIT_CONFIGURATION
#define INCLUDED_UNIT_CONFIGURATION

extern "C" {
   #include "printf.h" 
}

static void IntSet(volatile void* buffer,int value,int byteSize){
   volatile int* asInt = (int*) buffer;

   int nInts = byteSize / 4;

   for(int i = 0; i < nInts; i++){
      asInt[i] = value;
   }
}

#ifdef VERSAT_DEFINED_VRead
static void ConfigureSimpleVReadBare(VReadConfig* inst){
   IntSet(inst,0,sizeof(VReadConfig));

   // Memory side
   inst->read_incr = 1;
   inst->pingPong = 1;

   // B - versat side
   inst->output_iter = 1;
   inst->output_incr = 1;
   inst->output_duty = 1;
}

static void ConfigureSimpleVReadShallow(VReadConfig* inst, int numberItems,int* memory){
   inst->read_enabled = 1;

   // Memory side
   inst->read_per = numberItems;
   inst->read_duty = numberItems;
   inst->ext_addr = (iptr) memory;
   inst->read_length = numberItems * sizeof(int);

   // B - versat side
   inst->output_per = numberItems;
   inst->output_duty = numberItems;
}

static void ConfigureSimpleVRead(VReadConfig* inst, int numberItems,int* memory){
   ConfigureSimpleVReadBare(inst);
   ConfigureSimpleVReadShallow(inst,numberItems,memory);
}
#endif

#ifdef VERSAT_DEFINED_VWrite
static void ConfigureSimpleVWriteBare(VWriteConfig* inst){
   IntSet(inst,0,sizeof(VWriteConfig));

   inst->write_incr = 1;
   inst->pingPong = 1;

   inst->input_iter = 1;
   inst->input_duty = 1;
   inst->input_incr = 1;
}

static void ConfigureSimpleVWriteShallow(VWriteConfig* inst, int numberItems,int* memory){
   inst->write_enabled = 1;

   // Write side
   inst->write_per = numberItems;
   inst->write_duty = numberItems;
   inst->write_length = numberItems * sizeof(int);
   inst->ext_addr = (iptr) memory;

   // Memory side
   inst->input_per = numberItems;
   inst->input_duty = numberItems;
}

static void ConfigureSimpleVWrite(VWriteConfig* inst, int numberItems,int* memory){
   ConfigureSimpleVWriteBare(inst);
   ConfigureSimpleVWriteShallow(inst,numberItems,memory);
}
#endif

#ifdef VERSAT_DEFINED_Mem
static void ConfigureSimpleMemory(MemConfig* inst, int amountOfData, int start){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->startA = start;
}

static void ConfigureSimpleMemory(MemConfig* inst, int amountOfData, int start,MemAddr addr,int* data){
   ConfigureSimpleMemory(inst,amountOfData,start);
   VersatMemoryCopy(addr.addr,data,amountOfData * sizeof(int));
}

static void ConfigureSimpleMemory(MemConfig* inst, int amountOfData){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   //inst->in0_wr = 1;
}

static void ConfigureMemoryReceive(MemConfig* inst, int amountOfData){
   IntSet(inst,0,sizeof(MemConfig));

   inst->iterA = 1;
   inst->perA = amountOfData;
   inst->dutyA = amountOfData;
   inst->incrA = 1;
   inst->in0_wr = 1;
}
#endif

#endif // INCLUDED_UNIT_CONFIGURATION