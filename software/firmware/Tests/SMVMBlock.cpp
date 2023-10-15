#define BLOCK_XY

//#define BLOCK_SIMPLE_PRINT
//#define CLEAR_CACHE
//#define HEAVY_PRINT
//#define BLOCK_PRINT
//#define PRINT
//#define PRINT_OUTPUT
//#define PRINT_DIFF

//#define ILA_OUTPUT

#define CHANGE
#define TEST

#include "SMVM.hpp"

#define PERIOD 1
#define ACCELS 4

static void InitializeBaseAccelerator(){
  // Block 0
  ACCEL_TOP_block0_gen_iterations = 1;
  ACCEL_TOP_block0_gen_start = 0;
  ACCEL_TOP_block0_gen_shift = 1;
  ACCEL_TOP_block0_gen_incr = 1;
  ACCEL_TOP_block0_gen_duty = 1;
  ACCEL_TOP_block0_gen_period = PERIOD;

  ACCEL_TOP_block0_col_incrA = 1;
  ACCEL_TOP_block0_col_iterA = 1;
  ACCEL_TOP_block0_col_dutyA = ~0;
  ACCEL_TOP_block0_col_int_addr = 0;
  ACCEL_TOP_block0_col_pingPong = 1;

  ACCEL_TOP_block0_flag_disabled = 1;
  ACCEL_TOP_block0_flag_incrA = 1;
  ACCEL_TOP_block0_flag_iterA = 1;
  ACCEL_TOP_block0_flag_dutyA = ~0;
  ACCEL_TOP_block0_flag_int_addr = 0;
  ACCEL_TOP_block0_flag_maximum = 0;

  ACCEL_TOP_block0_val_incrA = 1;
  ACCEL_TOP_block0_val_iterA = 1;
  ACCEL_TOP_block0_val_dutyA = ~0;
  ACCEL_TOP_block0_val_int_addr = 0;
  ACCEL_TOP_block0_val_pingPong = 1;

  ACCEL_TOP_block0_vector_incrA = 1;
  ACCEL_TOP_block0_vector_iterA = 1;
  ACCEL_TOP_block0_vector_dutyA = ~0;
  ACCEL_TOP_block0_vector_int_addr = 0;

  ACCEL_TOP_block0_output_disabled = 0;

#ifndef CHANGE
  ACCEL_TOP_block0_vector_pingPong = 1;
#endif

  // Block 1
  ACCEL_TOP_block1_gen_iterations = 1;
  ACCEL_TOP_block1_gen_start = 0;
  ACCEL_TOP_block1_gen_shift = 1;
  ACCEL_TOP_block1_gen_incr = 1;
  ACCEL_TOP_block1_gen_duty = 1;
  ACCEL_TOP_block1_gen_period = PERIOD;

  ACCEL_TOP_block1_col_incrA = 1;
  ACCEL_TOP_block1_col_iterA = 1;
  ACCEL_TOP_block1_col_dutyA = ~0;
  ACCEL_TOP_block1_col_int_addr = 0;
  ACCEL_TOP_block1_col_pingPong = 1;

  ACCEL_TOP_block1_flag_disabled = 1;
  ACCEL_TOP_block1_flag_incrA = 1;
  ACCEL_TOP_block1_flag_iterA = 1;
  ACCEL_TOP_block1_flag_dutyA = ~0;
  ACCEL_TOP_block1_flag_int_addr = 0;
  ACCEL_TOP_block1_flag_maximum = 0;

  ACCEL_TOP_block1_val_incrA = 1;
  ACCEL_TOP_block1_val_iterA = 1;
  ACCEL_TOP_block1_val_dutyA = ~0;
  ACCEL_TOP_block1_val_int_addr = 0;
  ACCEL_TOP_block1_val_pingPong = 1;

  ACCEL_TOP_block1_vector_incrA = 1;
  ACCEL_TOP_block1_vector_iterA = 1;
  ACCEL_TOP_block1_vector_dutyA = ~0;
  ACCEL_TOP_block1_vector_int_addr = 0;

  ACCEL_TOP_block1_output_disabled = 0;

#ifndef CHANGE
  ACCEL_TOP_block1_vector_pingPong = 1;
#endif

  // Block 2
  ACCEL_TOP_block2_gen_iterations = 1;
  ACCEL_TOP_block2_gen_start = 0;
  ACCEL_TOP_block2_gen_shift = 1;
  ACCEL_TOP_block2_gen_incr = 1;
  ACCEL_TOP_block2_gen_duty = 1;
  ACCEL_TOP_block2_gen_period = PERIOD;

  ACCEL_TOP_block2_col_incrA = 1;
  ACCEL_TOP_block2_col_iterA = 1;
  ACCEL_TOP_block2_col_dutyA = ~0;
  ACCEL_TOP_block2_col_int_addr = 0;
  ACCEL_TOP_block2_col_pingPong = 1;

  ACCEL_TOP_block2_flag_disabled = 1;
  ACCEL_TOP_block2_flag_incrA = 1;
  ACCEL_TOP_block2_flag_iterA = 1;
  ACCEL_TOP_block2_flag_dutyA = ~0;
  ACCEL_TOP_block2_flag_int_addr = 0;
  ACCEL_TOP_block2_flag_maximum = 0;

  ACCEL_TOP_block2_val_incrA = 1;
  ACCEL_TOP_block2_val_iterA = 1;
  ACCEL_TOP_block2_val_dutyA = ~0;
  ACCEL_TOP_block2_val_int_addr = 0;
  ACCEL_TOP_block2_val_pingPong = 1;

  ACCEL_TOP_block2_vector_incrA = 1;
  ACCEL_TOP_block2_vector_iterA = 1;
  ACCEL_TOP_block2_vector_dutyA = ~0;
  ACCEL_TOP_block2_vector_int_addr = 0;

  ACCEL_TOP_block2_output_disabled = 0;

#ifndef CHANGE
  ACCEL_TOP_block2_vector_pingPong = 1;
#endif

  // Block 3
  ACCEL_TOP_block3_gen_iterations = 1;
  ACCEL_TOP_block3_gen_start = 0;
  ACCEL_TOP_block3_gen_shift = 1;
  ACCEL_TOP_block3_gen_incr = 1;
  ACCEL_TOP_block3_gen_duty = 1;
  ACCEL_TOP_block3_gen_period = PERIOD;

  ACCEL_TOP_block3_col_incrA = 1;
  ACCEL_TOP_block3_col_iterA = 1;
  ACCEL_TOP_block3_col_dutyA = ~0;
  ACCEL_TOP_block3_col_int_addr = 0;
  ACCEL_TOP_block3_col_pingPong = 1;

  ACCEL_TOP_block3_flag_disabled = 1;
  ACCEL_TOP_block3_flag_incrA = 1;
  ACCEL_TOP_block3_flag_iterA = 1;
  ACCEL_TOP_block3_flag_dutyA = ~0;
  ACCEL_TOP_block3_flag_int_addr = 0;
  ACCEL_TOP_block3_flag_maximum = 0;

  ACCEL_TOP_block3_val_incrA = 1;
  ACCEL_TOP_block3_val_iterA = 1;
  ACCEL_TOP_block3_val_dutyA = ~0;
  ACCEL_TOP_block3_val_int_addr = 0;
  ACCEL_TOP_block3_val_pingPong = 1;

  ACCEL_TOP_block3_vector_incrA = 1;
  ACCEL_TOP_block3_vector_iterA = 1;
  ACCEL_TOP_block3_vector_dutyA = ~0;
  ACCEL_TOP_block3_vector_int_addr = 0;

  ACCEL_TOP_block3_output_disabled = 0;

#ifndef CHANGE
  ACCEL_TOP_block3_vector_pingPong = 1;
#endif

}

static int lastLoadedX[ACCELS] = {-1,-1,-1,-1};

#ifdef TEST
static int vectorLoads = 0;
#endif

static void EnableReads(int accel){
  if(accel == 0){
    ACCEL_TOP_block0_col_disabled = 0;
    ACCEL_TOP_block0_val_disabled = 0;
    ACCEL_TOP_block0_vector_disabled = 0;
  }

  if(accel == 1){
    ACCEL_TOP_block1_col_disabled = 0;
    ACCEL_TOP_block1_val_disabled = 0;
    ACCEL_TOP_block1_vector_disabled = 0;
  }

  if(accel == 2){
    ACCEL_TOP_block2_col_disabled = 0;
    ACCEL_TOP_block2_val_disabled = 0;
    ACCEL_TOP_block2_vector_disabled = 0;
  }

  if(accel == 3){
    ACCEL_TOP_block3_col_disabled = 0;
    ACCEL_TOP_block3_val_disabled = 0;
    ACCEL_TOP_block3_vector_disabled = 0;
  }
}

static void EnableAccel(int accel){
  EnableReads(accel);

  if(accel == 0){
    ACCEL_TOP_block0_output_disabled = 0;
    ACCEL_TOP_block0_flag_disabled = 0;
  }

  if(accel == 1){
    ACCEL_TOP_block1_output_disabled = 0;
    ACCEL_TOP_block1_flag_disabled = 0;
  }

  if(accel == 2){
    ACCEL_TOP_block2_output_disabled = 0;
    ACCEL_TOP_block2_flag_disabled = 0;
  }

  if(accel == 3){
    ACCEL_TOP_block3_output_disabled = 0;
    ACCEL_TOP_block3_flag_disabled = 0;
  }
}

static void DisableReads(int accel){
  if(accel == 0){
    ACCEL_TOP_block0_col_disabled = 1;
    ACCEL_TOP_block0_val_disabled = 1;
    ACCEL_TOP_block0_vector_disabled = 1;
  }

  if(accel == 1){
    ACCEL_TOP_block1_col_disabled = 1;
    ACCEL_TOP_block1_val_disabled = 1;
    ACCEL_TOP_block1_vector_disabled = 1;
  }

  if(accel == 2){
    ACCEL_TOP_block2_col_disabled = 1;
    ACCEL_TOP_block2_val_disabled = 1;
    ACCEL_TOP_block2_vector_disabled = 1;
  }

  if(accel == 3){
    ACCEL_TOP_block3_col_disabled = 1;
    ACCEL_TOP_block3_val_disabled = 1;
    ACCEL_TOP_block3_vector_disabled = 1;
  }
}

static void DisableAccel(int accel){
  DisableReads(accel);

  if(accel == 0){
    ACCEL_TOP_block0_output_disabled = 1;
    ACCEL_TOP_block0_flag_disabled = 1;
  }

  if(accel == 1){
    ACCEL_TOP_block1_output_disabled = 1;
    ACCEL_TOP_block1_flag_disabled = 1;
  }

  if(accel == 2){
    ACCEL_TOP_block2_output_disabled = 1;
    ACCEL_TOP_block2_flag_disabled = 1;
  }

  if(accel == 3){
    ACCEL_TOP_block3_output_disabled = 1;
    ACCEL_TOP_block3_flag_disabled = 1;
  }
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

static inline void ConfigureToLoad0(BlockCSR* block){
  FormatCSR& toLoad = block->csr;
  int xPos = block->x;

  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    u16* data = CSRColumn(&toLoad);
    //printf("Col0: %p, %x, %x\n",data,data[0],data[1]);

    ACCEL_TOP_block0_col_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block0_col_ext_addr = (iptr) data;
    ACCEL_TOP_block0_col_length = numberItems * sizeof(u16); // AXI requires length of len - 1
    //printf("%d %d\n",numberItems,numberItems * sizeof(int));
  }

  //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
  // Memory side
  {
    int numberItems = toLoad.rowsAmount;
    u16* data = CSRRow(&toLoad);
    //printf("Row0: %p, %x, %x\n",data,data[0],data[1]);
    
    ACCEL_TOP_block0_flag_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block0_flag_ext_addr = (iptr) data;
    ACCEL_TOP_block0_flag_length = numberItems * sizeof(u16); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = CSRValue(&toLoad);
    //printf("Val0: %p, %f, %f\n",data,data[0],data[1]);

    ACCEL_TOP_block0_val_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block0_val_ext_addr = (iptr) data;
    ACCEL_TOP_block0_val_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(vec,vec.size,vec.data);
  // Memory side
#ifdef CHANGE
  if(lastLoadedX[0] == xPos){
    ACCEL_TOP_block0_vector_disabled = 0;
  }
  else
#endif
  {
    //printf("Vec: %p %d\n",vec.data,blockSize);
    int numberItems = blockSize;
    real* data = &vec.data[xPos * blockSize];

#ifdef CHANGE
    ACCEL_TOP_block0_vector_disabled = 0;

#ifdef TEST
    vectorLoads += 1;
#endif

    if(xPos > 0)
      ACCEL_TOP_block0_vector_pingPong = 1;
#endif

    ACCEL_TOP_block0_vector_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block0_vector_ext_addr = (iptr) data;
    ACCEL_TOP_block0_vector_length = numberItems * sizeof(int); // AXI requires length of len - 1

    lastLoadedX[0] = xPos;
  }
}

static inline void ConfigureToLoad1(BlockCSR* block){
  FormatCSR& toLoad = block->csr;
  int xPos = block->x;

  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    u16* data = CSRColumn(&toLoad);
    //printf("Col1: %p, %d, %d\n",data,data[0],data[1]);

    ACCEL_TOP_block1_col_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block1_col_ext_addr = (iptr) data;
    ACCEL_TOP_block1_col_length = numberItems * sizeof(u16); // AXI requires length of len - 1
    //printf("%d %d\n",numberItems,numberItems * sizeof(int));
  }

  //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
  // Memory side
  {
    int numberItems = toLoad.rowsAmount;
    u16* data = CSRRow(&toLoad);
    //printf("Row1: %p, %d, %d\n",data,data[0],data[1]);
    
    ACCEL_TOP_block1_flag_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block1_flag_ext_addr = (iptr) data;
    ACCEL_TOP_block1_flag_length = numberItems * sizeof(u16); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = CSRValue(&toLoad);
    //printf("Val1: %p, %f, %f\n",data,data[0],data[1]);

    ACCEL_TOP_block1_val_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block1_val_ext_addr = (iptr) data;
    ACCEL_TOP_block1_val_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(vec,vec.size,vec.data);
  // Memory side
#ifdef CHANGE
  if(lastLoadedX[1] == xPos){
    ACCEL_TOP_block1_vector_disabled = 0;
  }
  else
#endif
  {
    //printf("Vec: %p %d\n",vec.data,blockSize);
    int numberItems = blockSize;
    real* data = &vec.data[xPos * blockSize];

#ifdef CHANGE
    ACCEL_TOP_block1_vector_disabled = 0;

#ifdef TEST
    vectorLoads += 1;
#endif

    if(xPos > 0)
      ACCEL_TOP_block1_vector_pingPong = 1;
#endif

    ACCEL_TOP_block1_vector_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block1_vector_ext_addr = (iptr) data;
    ACCEL_TOP_block1_vector_length = numberItems * sizeof(int); // AXI requires length of len - 1

    lastLoadedX[1] = xPos;
  }
}

static inline void ConfigureToLoad2(BlockCSR* block){
  FormatCSR& toLoad = block->csr;
  int xPos = block->x;

  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    u16* data = CSRColumn(&toLoad);
    //printf("Col0: %p, %x, %x\n",data,data[0],data[1]);

    ACCEL_TOP_block2_col_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block2_col_ext_addr = (iptr) data;
    ACCEL_TOP_block2_col_length = numberItems * sizeof(u16); // AXI requires length of len - 1
    //printf("%d %d\n",numberItems,numberItems * sizeof(int));
  }

  //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
  // Memory side
  {
    int numberItems = toLoad.rowsAmount;
    u16* data = CSRRow(&toLoad);
    //printf("Row0: %p, %x, %x\n",data,data[0],data[1]);
    
    ACCEL_TOP_block2_flag_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block2_flag_ext_addr = (iptr) data;
    ACCEL_TOP_block2_flag_length = numberItems * sizeof(u16); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = CSRValue(&toLoad);
    //printf("Val0: %p, %f, %f\n",data,data[0],data[1]);

    ACCEL_TOP_block2_val_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block2_val_ext_addr = (iptr) data;
    ACCEL_TOP_block2_val_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(vec,vec.size,vec.data);
  // Memory side
#ifdef CHANGE
  if(lastLoadedX[2] == xPos){
    ACCEL_TOP_block2_vector_disabled = 0;
  }
  else
#endif
  {
    //printf("Vec: %p %d\n",vec.data,blockSize);
    int numberItems = blockSize;
    real* data = &vec.data[xPos * blockSize];

#ifdef CHANGE
    ACCEL_TOP_block2_vector_disabled = 0;

#ifdef TEST
    vectorLoads += 1;
#endif

    if(xPos > 0)
      ACCEL_TOP_block2_vector_pingPong = 1;
#endif

    ACCEL_TOP_block2_vector_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block2_vector_ext_addr = (iptr) data;
    ACCEL_TOP_block2_vector_length = numberItems * sizeof(int); // AXI requires length of len - 1

    lastLoadedX[2] = xPos;
  }
}

static inline void ConfigureToLoad3(BlockCSR* block){
  FormatCSR& toLoad = block->csr;
  int xPos = block->x;

  //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    u16* data = CSRColumn(&toLoad);
    //printf("Col0: %p, %x, %x\n",data,data[0],data[1]);

    ACCEL_TOP_block3_col_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block3_col_ext_addr = (iptr) data;
    ACCEL_TOP_block3_col_length = numberItems * sizeof(u16); // AXI requires length of len - 1
    //printf("%d %d\n",numberItems,numberItems * sizeof(int));
  }

  //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
  // Memory side
  {
    int numberItems = toLoad.rowsAmount;
    u16* data = CSRRow(&toLoad);
    //printf("Row0: %p, %x, %x\n",data,data[0],data[1]);
    
    ACCEL_TOP_block3_flag_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block3_flag_ext_addr = (iptr) data;
    ACCEL_TOP_block3_flag_length = numberItems * sizeof(u16); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
  // Memory side
  {
    int numberItems = toLoad.nonZeros;
    real* data = CSRValue(&toLoad);
    //printf("Val0: %p, %f, %f\n",data,data[0],data[1]);

    ACCEL_TOP_block3_val_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block3_val_ext_addr = (iptr) data;
    ACCEL_TOP_block3_val_length = numberItems * sizeof(int); // AXI requires length of len - 1
  }

  //ConfigureSimpleVRead(vec,vec.size,vec.data);
  // Memory side
#ifdef CHANGE
  if(lastLoadedX[3] == xPos){
    ACCEL_TOP_block3_vector_disabled = 0;
  }
  else
#endif
  {
    //printf("Vec: %p %d\n",vec.data,blockSize);
    int numberItems = blockSize;
    real* data = &vec.data[xPos * blockSize];

#ifdef CHANGE
    ACCEL_TOP_block3_vector_disabled = 0;

#ifdef TEST
    vectorLoads += 1;
#endif

    if(xPos > 0)
      ACCEL_TOP_block3_vector_pingPong = 1;
#endif

    ACCEL_TOP_block3_vector_perA = numberItems;

    // B - versat side
    ACCEL_TOP_block3_vector_ext_addr = (iptr) data;
    ACCEL_TOP_block3_vector_length = numberItems * sizeof(int); // AXI requires length of len - 1

    lastLoadedX[3] = xPos;
  }
}

struct AccelState{
  int lastToLoad;
  int lastToRun;
};

static AccelState state[ACCELS];

static void ConfigAccelerator(int frame,int accel){
  int blockSize = block->blockSize;
  Array<BlockCSR> blocks = MatrixBlockCSRArray(block);

  int max = blocks.size;
  int toLoad = frame;

  int toRun = state[accel].lastToLoad;
  int toWrite = state[accel].lastToRun;
  
  state[accel].lastToLoad = toLoad;
  state[accel].lastToRun = toRun;
  
#ifdef PRINT
  printf("%d %d %d %d\n",toLoad,toRun,toWrite,max);
#endif

  if(toRun >= 0 && toRun < max){
    BlockCSR* toRunBlock = &blocks[toRun];
    int nonZeros = toRunBlock->csr.nonZeros;
    
    if(accel == 0){
      ACCEL_TOP_block0_cycler_amount = 12 + nonZeros * PERIOD;
      ACCEL_TOP_block0_gen_iterations = nonZeros + 1 + 1 + 2; // Extra one so that FlagRead does not repeat last value 
      ACCEL_TOP_block0_flag_maximum = nonZeros + 1;
      ACCEL_TOP_block0_flag_disabled = 0;
    } else if(accel == 1) {
      ACCEL_TOP_block1_cycler_amount = 12 + nonZeros * PERIOD;
      ACCEL_TOP_block1_gen_iterations = nonZeros + 1 + 1 + 2; // Extra one so that FlagRead does not repeat last value
      ACCEL_TOP_block1_flag_maximum = nonZeros + 1;
      ACCEL_TOP_block1_flag_disabled = 0;
    } else if(accel == 2) {
      ACCEL_TOP_block2_cycler_amount = 12 + nonZeros * PERIOD;
      ACCEL_TOP_block2_gen_iterations = nonZeros + 1 + 1 + 2; // Extra one so that FlagRead does not repeat last value
      ACCEL_TOP_block2_flag_maximum = nonZeros + 1;
      ACCEL_TOP_block2_flag_disabled = 0;
    } else if(accel == 3) {
      ACCEL_TOP_block3_cycler_amount = 12 + nonZeros * PERIOD;
      ACCEL_TOP_block3_gen_iterations = nonZeros + 1 + 1 + 2; // Extra one so that FlagRead does not repeat last value
      ACCEL_TOP_block3_flag_maximum = nonZeros + 1;
      ACCEL_TOP_block3_flag_disabled = 0;
    }
  } else if(toRun >= max) {
    if(accel == 0){
      ACCEL_TOP_block0_cycler_amount = 0;
      ACCEL_TOP_block0_gen_iterations = 0;
      ACCEL_TOP_block0_flag_disabled = 1;
    } else if(accel == 1) {
      ACCEL_TOP_block1_cycler_amount = 0;
      ACCEL_TOP_block1_gen_iterations = 0;
      ACCEL_TOP_block1_flag_disabled = 1;
    } else if(accel == 2) {
      ACCEL_TOP_block2_cycler_amount = 0;
      ACCEL_TOP_block2_gen_iterations = 0;
      ACCEL_TOP_block2_flag_disabled = 1;
    } else if(accel == 3) {
      ACCEL_TOP_block3_cycler_amount = 0;
      ACCEL_TOP_block3_gen_iterations = 0;
      ACCEL_TOP_block3_flag_disabled = 1;
    }
  }

//  printf("ToWrite: %d (%d)\n",toWrite,max);
  if(toWrite >= 0 && toWrite < max){
//    printf("here\n");
    if(accel == 0){
      ACCEL_TOP_block0_output_disabled = 0;
      ACCEL_TOP_block0_output_ext_addr = (iptr) &outputBuffer[outputIndex];
    } else if(accel == 1) {
      ACCEL_TOP_block1_output_disabled = 0;
      ACCEL_TOP_block1_output_ext_addr = (iptr) &outputBuffer[outputIndex];
    } else if(accel == 2) {
      ACCEL_TOP_block2_output_disabled = 0;
      ACCEL_TOP_block2_output_ext_addr = (iptr) &outputBuffer[outputIndex];
    } else if(accel == 3) {
      ACCEL_TOP_block3_output_disabled = 0;
      ACCEL_TOP_block3_output_ext_addr = (iptr) &outputBuffer[outputIndex];
    }      
      
    BlockCSR* toWriteBlock = &blocks[toWrite];

#ifdef PRINT
    printf("%d %d\n",outputIndex,toWriteBlock->csr.rowsAmount);
#endif 

    outputIndex += toWriteBlock->csr.rowsAmount;
  }

  if(toLoad < max){
    BlockCSR* toLoadBlock = &blocks.data[toLoad];

    if(accel == 0) ConfigureToLoad0(toLoadBlock);
    if(accel == 1) ConfigureToLoad1(toLoadBlock);
    if(accel == 2) ConfigureToLoad2(toLoadBlock);
    if(accel == 3) ConfigureToLoad3(toLoadBlock);
  } else {
    DisableReads(accel);
  }
}

#ifndef PC
extern int timesWaiting;

float Abs(float val){
  float res = val;
  if(val < 0.0f){
    res = -val;
  }
  return res;
}

bool FloatEqual(float f0,float f1,float epsilon){
  if(f0 == f1){
    return true;
  }

  float norm = Abs(f0) + Abs(f1);
  norm = std::min(norm,std::numeric_limits<float>::max());
  float diff = Abs(f0 - f1);

  bool equal = diff < norm * epsilon;

  return equal;
}
#endif

void SingleTest(Arena* arena){
  InitializeSMVM(arena,Type::BLOCKCSR);

  InitializeBaseAccelerator();
  
  int loops = block->numberBlocks;
  int blockSize = block->blockSize;
  Array<BlockCSR> blocks = MatrixBlockCSRArray(block);
  
  for(int i = 0; i < ACCELS; i++){
    state[i].lastToLoad = -1;
    state[i].lastToRun = -2;
  }

  int amountOfOutput = 0;
  for(int i = 0; i < loops; i++){
    BlockCSR* block = &blocks[i];
    amountOfOutput += block->csr.rowsAmount;
  }

  PushBytes(arena,Megabyte(1));
  PushPageAlign(arena);
  Array<float> outputArray = PushArray<float>(arena,amountOfOutput);  
  outputBuffer = outputArray.data;
  Memset(outputArray,0.0f);
  PushPageAlign(arena);
  PushBytes(arena,Megabyte(1));

  ClearCache(PushBytes(arena,Megabyte(1)));

#ifdef PRINT
  printf("Size of outputArray: %d\n",outputArray.size);
  printf("%p\n",outputBuffer);
#endif
  
  PushPageAlign(arena);
  Array<real> res = PushArray<real>(arena,size);
  Memset(res,0.0f);
  PushPageAlign(arena);

#ifdef ILA_OUTPUT
  ila_set_different_signal_storing(1);
  ila_set_time_offset(0);
  
  ila_set_trigger_type(0,ILA_TRIGGER_TYPE_CONTINUOUS); // Valid
  //ila_set_trigger_type(1,ILA_TRIGGER_TYPE_SINGLE); // Done
  //ila_set_trigger_type(2,ILA_TRIGGER_TYPE_SINGLE); // Run
  //ila_set_trigger_type(2,ILA_TRIGGER_TYPE_CONTINUOUS); // Run
  //ila_set_trigger_type(2,ILA_TRIGGER_TYPE_SINGLE); // Ready
  
  //ila_set_trigger_negated(1,1);
  
  ila_set_trigger_enabled(0,true);
#if 0
  ila_set_trigger_enabled(1,true);
  ila_set_trigger_enabled(2,true);
  ila_set_trigger_enabled(3,true);
#endif
  
  ila_set_reduce_type(ILA_REDUCE_TYPE_OR);
#endif // ILA_OUTPUT

  Time totalStart = {};
  Time cpuStart = {};
  {
    totalStart = GetTime();

    // Initial configuration before first Start
    for(int i = 0; i < ACCELS; i++){
      ConfigAccelerator(i,i);
    }

    for(int currentRun = ACCELS; currentRun < (loops + ACCELS * 2 + ACCELS);){
      StartAccelerator();
      
      for(int i = 0; i < ACCELS; i++){
        ConfigAccelerator(currentRun,i);
        currentRun += 1;
      }

      EndAccelerator();
    }

#if 0
    // Disable the run side
    for(int i = 0; i < ACCELS; i++){
      DisableReads(i);
    }
    ACCEL_TOP_block0_flag_disabled = 1;
    ACCEL_TOP_block1_flag_disabled = 1;
    ACCEL_TOP_block2_flag_disabled = 1;
    ACCEL_TOP_block3_flag_disabled = 1;
#endif
    
    // Just to write the final memory values;
    StartAccelerator();
    EndAccelerator();
    
    //printf("Accelerator end\n");
#ifdef CLEAR_CACHE
    ClearCache(PushBytes(arena,Megabyte(1)));
#endif

    cpuStart = GetTime();
#if 1
    int index = 0;
    int outputIndex = 0;
    for(int i = 0; i < blocks.size; i++){
      BlockCSR& block = blocks[i];
      int yOffset = block.y * blockSize;
      int size = block.csr.rowsAmount;

      Array<Pair<u16,u16>> offsets = block.offsets; //CSROffsetsArray(&block.csr);

#ifdef PRINT
      printf("Offset: %d\n",offsets.size);
#endif

#ifdef PRINT
      if(offsets.size == 1){
        Pair<u16,u16> p = offsets[0];
        printf("P %d %d\n",p.first,p.second);
      }
#endif

      int offset = 0;
      int pairIndex = 0;
      for(int ii = 0; ii < size; ii++){
        if(pairIndex < offsets.size){
          if(offsets[pairIndex].first == offset){ 
            offset = offsets[pairIndex].second;
            pairIndex += 1; // Accelerator converts every row into one value so if pair is found, we know we can go for the next
          } 
        }

#ifdef PRINT
        printf("%d %d\n",yOffset + offset,index + offset);
#endif

        res[yOffset + offset] += outputArray[outputIndex];
        outputIndex += 1;
        offset += 1;
      }
      index += size;
    }
#endif
  }

  Time endTime = GetTime();

  Time cpuTime = endTime - cpuStart;
  Time totalTime = endTime - totalStart;

  Time accelTime = cpuStart - totalStart;

#ifdef PRINT
  PrintTime(totalStart,"TotalStart");
  PrintTime(cpuStart,"CPUStart");
  PrintTime(endTime,"EndTime");

  PrintTime(totalTime,"Total");
  PrintTime(cpuTime,"CPU");
  PrintTime(accelTime,"Accel");
#endif
  
#ifdef PRINT_OUTPUT
  printf("OutputBuffer: %p\n",outputBuffer);
  for(int i = 0; i < outputIndex; i++){
    printf("%f\n",outputBuffer[i]);
  }
#endif

#ifdef TEST
  SignalLoop();

  PushGot(res);
#ifdef PRINT
  printf("VecLoads: %d/%d\n",vectorLoads,blocks.size);
#endif
#endif

#ifndef PC
  printf("Times waiting: %d\n",timesWaiting);
#endif

#if 0 
  for(int i = 0; i < loops; i++){
    printf("%d %d %d\n",runs[i].loaded,runs[i].runned,runs[i].written);
  }
#endif

#ifdef ILA_OUTPUT
    ila_output_everything();
#endif // ILA_OUTPUT
}
