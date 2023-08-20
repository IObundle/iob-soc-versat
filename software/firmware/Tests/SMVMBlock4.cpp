#define BLOCK_XY

#include "SMVM.hpp"

#define CHANGE
//#define PRINT
#define TEST

static void InitializeBaseAccelerator(){
  //ACCEL_TOP_cycler_amount = size + 24;

  ACCEL_TOP_gen_iterations = 1;
  ACCEL_TOP_gen_start = 0;
  ACCEL_TOP_gen_shift = 0;
  ACCEL_TOP_gen_incr = 1;
  ACCEL_TOP_gen_duty = 1;
  ACCEL_TOP_gen_period = 5;
  //ACCEL_TOP_gen_off_value = 0;
  
  ACCEL_TOP_col_incrA = 1;
  ACCEL_TOP_col_iterA = 1;
  ACCEL_TOP_col_dutyA = ~0;
  ACCEL_TOP_col_size = 8;
  ACCEL_TOP_col_int_addr = 0;
  ACCEL_TOP_col_pingPong = 1;

  ACCEL_TOP_flag_disabled = 1;
  ACCEL_TOP_flag_incrA = 1;
  ACCEL_TOP_flag_iterA = 1;
  ACCEL_TOP_flag_dutyA = ~0;
  ACCEL_TOP_flag_size = 8;
  ACCEL_TOP_flag_int_addr = 0;

  ACCEL_TOP_val_incrA = 1;
  ACCEL_TOP_val_iterA = 1;
  ACCEL_TOP_val_dutyA = ~0;
  ACCEL_TOP_val_size = 8;
  ACCEL_TOP_val_int_addr = 0;
  ACCEL_TOP_val_pingPong = 1;

  ACCEL_TOP_vector_incrA = 1;
  ACCEL_TOP_vector_iterA = 1;
  ACCEL_TOP_vector_dutyA = ~0;
  ACCEL_TOP_vector_size = 8;
  ACCEL_TOP_vector_int_addr = 0;

  ACCEL_TOP_output_disabled = 0;
  //ACCEL_TOP_output_pingPong = 1;
  //ACCEL_TOP_pos_pingPong = 1;
   
#ifndef CHANGE
  ACCEL_TOP_vector_pingPong = 1;
#endif
}

static int lastLoadedX = -1;

#ifdef TEST
static int vectorLoads = 0;
#endif

static void DisableReads(){
  ACCEL_TOP_col_disabled = 1;
  ACCEL_TOP_val_disabled = 1;
  ACCEL_TOP_vector_disabled = 1;
}

static float* outputBuffer;
static int outputIndex = 0;

static int Bound(int min,int val,int max){
  if(val < min){
    return min;
  }
  if(val > max){
    return max;
  }
  return val;
}

static inline void ConfigureToLoad(BlockCSR* block){
  FormatCSR& toLoad = block->csr;
  int xPos = block->x;
  
  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    int* data = CSRColumn(&toLoad);
    //printf("Col: %x, %x, %x\n",data,data[0],data[1]);
      
    ACCEL_TOP_col_perA = numberItems;

    // B - versat side
    ACCEL_TOP_col_ext_addr = (iptr) data;
    ACCEL_TOP_col_length = numberItems * sizeof(int); // AXI requires length of len - 1
    //printf("%d %d\n",numberItems,numberItems * sizeof(int));
  }

  //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
  // Memory side
  {
    int numberItems = toLoad.rowsAmount;
    int* data = CSRRow(&toLoad);
    //printf("Row: %x\n",data);

    ACCEL_TOP_flag_perA = numberItems;

    // B - versat side
    ACCEL_TOP_flag_ext_addr = (iptr) data;
    ACCEL_TOP_flag_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }
  
  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = CSRValue(&toLoad);
    //printf("Val: %x\n",data);
      
    ACCEL_TOP_val_perA = numberItems;
    
    // B - versat side
    ACCEL_TOP_val_ext_addr = (iptr) data;
    ACCEL_TOP_val_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(vec,vec.size,vec.data);
  // Memory side
#ifdef CHANGE
  if(lastLoadedX == xPos){
    ACCEL_TOP_vector_disabled = 0;
  }
  else
#endif
  {
    //printf("Vec: %p %d\n",vec.data,blockSize);
    int numberItems = blockSize;
#ifdef PRINT
    printf("%p\n",&vec.data[xPos * blockSize]);
#endif
    real* data = &vec.data[xPos * blockSize];

#ifdef CHANGE
    ACCEL_TOP_vector_disabled = 0;

#ifdef TEST
    vectorLoads += 1;
#endif
      
    if(xPos > 0)
      ACCEL_TOP_vector_pingPong = 1;
#endif
      
    ACCEL_TOP_vector_perA = numberItems;

    // B - versat side
    ACCEL_TOP_vector_ext_addr = (iptr) data;
    ACCEL_TOP_vector_length = numberItems * sizeof(int); // AXI requires length of len - 1
      
    lastLoadedX = xPos;
  }
}

static void ConfigAccelerator(int frame){
  int blockSize = block->blockSize;
  Array<BlockCSR> blocks = MatrixBlockCSRArray(block);

  int max = blocks.size;
  int toLoad = frame;
  int toRun = frame - 1;
  int toWrite = frame - 2;

  printf("%d %d %d %d\n",toLoad,toRun,toWrite,max);
  
  if(toRun >= 0 && toRun < max){
    BlockCSR* toRunBlock = &blocks[toRun];
    int nonZeros = toRunBlock->csr.nonZeros;
    ACCEL_TOP_cycler_amount = 9 + nonZeros * 6;
    ACCEL_TOP_gen_iterations = nonZeros;

    ACCEL_TOP_flag_disabled = 0;
    Array<int> rows = CSRRowArray(&toRunBlock->csr);
  } else if(toRun >= max) {
    ACCEL_TOP_cycler_amount = 0;
    ACCEL_TOP_gen_iterations = 0;
  }

  if(toWrite >= 0 && toWrite < max){
    ACCEL_TOP_output_disabled = 0;
    ACCEL_TOP_output_ext_addr = (iptr) &outputBuffer[outputIndex];

    BlockCSR* toWriteBlock = &blocks[toWrite];

    outputIndex += toWriteBlock->csr.rowsAmount;
  } else {
    ACCEL_TOP_output_disabled = ~0;
  }

  if(toLoad < max){
    BlockCSR* toLoadBlock = &blocks.data[toLoad];
    ConfigureToLoad(toLoadBlock);
  } else {
    DisableReads();
  }
}

#ifndef PC
extern int timesWaiting;
#endif

void SingleTest(Arena* arena){
  InitializeSMVM(arena,Type::BLOCKCSR);

  InitializeBaseAccelerator();

  Array<float> outputArray = PushArray<float>(arena,2048);
  Memset(outputArray,0.0f);
  outputBuffer = outputArray.data;
  printf("%p\n",outputBuffer);
  
  Array<real> expected = {};
  timeRegion("CPU multiply"){
    expected = MultiplyBlockCSR(block,vec,arena);
  }
  printf("Make sure compiler does not optimize away: %d\n",expected[0]);

  // Initialize ILA
  ila_set_different_signal_storing(1);
  ila_set_time_offset(-1);
  ila_set_trigger_type(1,ILA_TRIGGER_TYPE_CONTINUOUS);
   
  Array<real> res = PushArray<real>(arena,size);
  Memset(res,0.0f);

  int loops = block->numberBlocks;
  int blockSize = block->blockSize;
  Array<BlockCSR> blocks = MatrixBlockCSRArray(block);
  {
    TIME_IT("Accelerator Run");

    ila_set_trigger_enabled(1,true); // Ila start recording. Only a small overhead cost anyway

    ConfigAccelerator(0);

    for(int currentRun = 1; currentRun < loops + 2; currentRun++){
      StartAccelerator();

      ConfigAccelerator(currentRun);

      EndAccelerator();
    }
  }

  StartAccelerator();
  EndAccelerator();

  int index = 0;
  for(int i = 0; i < blocks.size; i++){
    BlockCSR& block = blocks[i];
    int yOffset = block.y * blockSize;
    int size = block.csr.rowsAmount;

    for(int ii = 0; ii < size; ii++){
      printf("%d %d\n",yOffset + ii,index + ii);
      res[yOffset + ii] += outputArray[index + ii];
    }
    index += size;
  }
  
  for(int i = 0; i < outputIndex; i++){
    printf("%f\n",outputBuffer[i]);
  }
  
  //ila_output_everything();

#ifdef TEST
  PushGot(res);
  printf("VecLoads: %d/%d\n",vectorLoads,blocks.size);
#endif 

#ifndef PC
  printf("Times waiting: %d\n",timesWaiting);
#endif
}
