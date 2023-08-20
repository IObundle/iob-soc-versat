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

  ACCEL_TOP_row_incrA = 1;
  ACCEL_TOP_row_iterA = 1;
  ACCEL_TOP_row_dutyA = ~0;
  ACCEL_TOP_row_size = 8;
  ACCEL_TOP_row_int_addr = 0;
  ACCEL_TOP_row_pingPong = 1;

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
  ACCEL_TOP_row_disabled = 1;
  ACCEL_TOP_val_disabled = 1;
  ACCEL_TOP_vector_disabled = 1;
}

static float* outputBuffer;
static int outputIndex = 0;

static void ConfigureAccelerator(FormatCOO& toLoad,FormatCOO* toRun,int xPos,int blockSize,int toLoadIndex,int toRunIndex){
  if(toRun){
    ACCEL_TOP_cycler_amount = 9 + toRun->nonZeros * 6;
  }

  ACCEL_TOP_output_ext_addr = (iptr) &outputBuffer[outputIndex];
  outputIndex += 4;
  
  //ConfigureGenerator(gen,0,toLoad.values.size + 2,1);
  {
    int range = 0;

    if(toRun){
      range = toRun->nonZeros;
    }

    ACCEL_TOP_gen_iterations = range;
  }

  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    int* data = COOColumn(&toLoad);
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
    int numberItems = toLoad.nonZeros + 1;
    int* data = COORow(&toLoad);
    //printf("Row: %x\n",data);

    ACCEL_TOP_row_perA = numberItems;

    // B - versat side
    ACCEL_TOP_row_ext_addr = (iptr) data;
    ACCEL_TOP_row_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = COOValue(&toLoad);
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

static void DoOneFully(Array<int> res,int index){
  Block& b = MatrixBlock(block)[index];
  FormatCOO coo = b.coo;

  if(coo.nonZeros == 0){
    return;
  }

  Memset(res,0);
#if 1
  //printf("\n\nI:%d Y:%d X:%d\n",index,b.y,b.x);
  Array<int> column = COOColumnArray(&coo);
  Array<int> row    = COORowArray(&coo);
  Array<real> values = COOValueArray(&coo);

  int yOffset = b.y * block->blockSize;
  int xOffset = b.x * block->blockSize;

  for(int i = 0; i < values.size; i++){
    res[yOffset + row[i]] += values[i] * vec[xOffset + column[i]];
  }

  int lastRow = -1;
  for(int i = 0; i < values.size; i++){
    if(row[i] != lastRow){
      //printf("%d %d %d\n",row[i],row[i] + b.y * block->blockSize,res[b.y * block->blockSize + row[i]]);
      lastRow = row[i];
    }
  }

#else
  printf("=== Accel. %d ===\n",coo.nonZeros);

  ConfigureAccelerator(coo,&coo,b.x,block->blockSize,index,index);

  RunAccelerator(3);
#endif
}

static Array<int> DoOneFullyTest(int index,Arena* out){
  Array<int> res = PushArray<int>(out,size);
  Memset(res,0);

  DoOneFully(res,index);

  return res;
}

static int Bound(int min,int val,int max){
  if(val < min){
    return min;
  }
  if(val > max){
    return max;
  }
  return val;
}

#if 0
static void ConfigAccelerator(int frame){
  int blockSize = block->blockSize;
  Array<Block> blocks = MatrixBlockArray(block);

  int toLoad = frame;
  int toRun = frame - 1;
  int toWrite = frame - 2;

  int max = blocks.size - 1;
  
  toLoad = Bound(0,toLoad,max);
//toRun = Bound(0,toRun,max);
//  toWrite = Bound(0,toWrite,max);

  Block& load = blocks[toLoad];
  Block& runBlock = blocks[toRun];
  FormatCOO* runCOO = &runBlock.coo;

  if(toRun < 0){
    runCOO = nullptr;
  }
  
#ifdef PRINT
  printf("Y: %d X: %d\n",runBlock.y,runBlock.x);
#endif
  
  //printf("ToLoad %d ToRun %d\n",toLoad,toRun);
  ConfigureAccelerator(load.coo,runCOO,load.x,blockSize,toLoad,toRun);
}
#endif

static inline void ConfigureToLoad(Block* block){
  FormatCOO& toLoad = block->coo;
  int xPos = block->x;
  
  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    int* data = COOColumn(&toLoad);
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
    int numberItems = toLoad.nonZeros + 1;
    int* data = COORow(&toLoad);
    //printf("Row: %x\n",data);

    ACCEL_TOP_row_perA = numberItems;

    // B - versat side
    ACCEL_TOP_row_ext_addr = (iptr) data;
    ACCEL_TOP_row_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = COOValue(&toLoad);
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
  Array<Block> blocks = MatrixBlockArray(block);

  int max = blocks.size;
  int toLoad = frame;
  int toRun = frame - 1;
  int toWrite = frame - 2;

  printf("%d %d %d %d\n",toLoad,toRun,toWrite,max);
  
  if(toRun >= 0 && toRun < max){
    Block* toRunBlock = &blocks[toRun];
    int nonZeros = toRunBlock->coo.nonZeros;
    ACCEL_TOP_cycler_amount = 9 + nonZeros * 6;
    ACCEL_TOP_gen_iterations = nonZeros;
  } else if(toRun >= max) {
    ACCEL_TOP_cycler_amount = 0;
    ACCEL_TOP_gen_iterations = 0;
  }

  if(toWrite >= 0 && toWrite < max){
    ACCEL_TOP_output_disabled = 0;
    ACCEL_TOP_output_ext_addr = (iptr) &outputBuffer[outputIndex];
    outputIndex += 4;
  } else {
    ACCEL_TOP_output_disabled = ~0;
  }

  if(toLoad < max){
    Block* toLoadBlock = &blocks.data[toLoad];
    ConfigureToLoad(toLoadBlock);
  } else {
    DisableReads();
  }
}


#ifndef PC
extern int timesWaiting;
#endif

void SingleTest(Arena* arena){
  InitializeSMVM(arena,Type::BLOCK);

  InitializeBaseAccelerator();

  Array<float> outputArray = PushArray<float>(arena,2048);
  Memset(outputArray,0.0f);
  outputBuffer = outputArray.data;
  printf("%p\n",outputBuffer);
  
  Array<real> expected = {};
  timeRegion("CPU multiply"){
    expected = MultiplyBlock(block,vec,arena);
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
  Array<Block> blocks = MatrixBlockArray(block);
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
