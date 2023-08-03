#define BLOCK_XY

#include "SMVM.hpp"

#define CHANGE
//#define PRINT
#define TEST

static void InitializeBaseAccelerator(){
   //ACCEL_TOP_cycler_amount = size + 24;

   ACCEL_TOP_gen_iterations = 1;
   ACCEL_TOP_gen_start = 0;
   ACCEL_TOP_gen_shift = 1;
   ACCEL_TOP_gen_incr = 1;
   //ACCEL_TOP_gen_off_value = 0;

   ACCEL_TOP_col_incrA = 1;
   ACCEL_TOP_col_iterA = 1;
   ACCEL_TOP_col_dutyA = ~0;
   ACCEL_TOP_col_size = 8;
   ACCEL_TOP_col_int_addr = 0;
   ACCEL_TOP_col_pingPong = 1;

   // B - versat side
   ACCEL_TOP_col_incrB = 1;
   ACCEL_TOP_col_perB = 1;
   ACCEL_TOP_col_dutyB = 1;

   ACCEL_TOP_row_incrA = 1;
   ACCEL_TOP_row_iterA = 1;
   ACCEL_TOP_row_dutyA = ~0;
   ACCEL_TOP_row_size = 8;
   ACCEL_TOP_row_int_addr = 0;
   ACCEL_TOP_row_pingPong = 1;

   // B - versat side
   ACCEL_TOP_row_incrB = 1;
   ACCEL_TOP_row_perB = 1;
   ACCEL_TOP_row_dutyB = 1;

   ACCEL_TOP_val_incrA = 1;
   ACCEL_TOP_val_iterA = 1;
   ACCEL_TOP_val_dutyA = ~0;
   ACCEL_TOP_val_size = 8;
   ACCEL_TOP_val_int_addr = 0;
   ACCEL_TOP_val_pingPong = 1;

   // B - versat side
   ACCEL_TOP_val_incrB = 1;
   ACCEL_TOP_val_perB = 1;
   ACCEL_TOP_val_dutyB = 1;

   ACCEL_TOP_vector_incrA = 1;
   ACCEL_TOP_vector_iterA = 1;
   ACCEL_TOP_vector_dutyA = ~0;
   ACCEL_TOP_vector_size = 8;
   ACCEL_TOP_vector_int_addr = 0;
   
   // B - versat side
   ACCEL_TOP_vector_incrB = 1;
   ACCEL_TOP_vector_perB = 1;
   ACCEL_TOP_vector_dutyB = 1;

   ACCEL_TOP_output_pingPong = 1;
   ACCEL_TOP_pos_pingPong = 1;
   
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
   
static void ConfigureAccelerator(FormatCOO& toLoad,FormatCOO* toRun,int xPos,int blockSize){
   if(toRun){
      ACCEL_TOP_cycler_amount = 9 + toRun->nonZeros;
   }

   //ConfigureGenerator(gen,0,toLoad.values.size + 2,1);
   {
      int range = 0;

      if(toRun){
         range = toRun->nonZeros;
      }

      ACCEL_TOP_gen_period = range + 1;
      ACCEL_TOP_gen_duty = range + 1;
   }

   //ConfigureSimpleVRead(col,toLoad.column.size,toLoad.column.data);
   // Memory side
   {
      int numberItems = toLoad.nonZeros;
      int* data = COOColumn(&toLoad);
      //printf("Col: %x, %x, %x\n",data,data[0],data[1]);
      
      ACCEL_TOP_col_perA = numberItems;

      // B - versat side
      ACCEL_TOP_col_iterB = numberItems;
      ACCEL_TOP_col_ext_addr = (iptr) data;
      ACCEL_TOP_col_length = numberItems * sizeof(int); // AXI requires length of len - 1
      printf("%d %d\n",numberItems,numberItems * sizeof(int));
   }

   //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
   // Memory side
   {
      int numberItems = toLoad.nonZeros + 1;
      int* data = COORow(&toLoad);
      //printf("Row: %x\n",data);

      ACCEL_TOP_row_perA = numberItems;

      // B - versat side
      ACCEL_TOP_row_iterB = numberItems;
      ACCEL_TOP_row_ext_addr = (iptr) data;
      ACCEL_TOP_row_length = numberItems * sizeof(int); // AXI requires length of len - 1
   }

   //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
   // Memory side
   {
      int numberItems = toLoad.nonZeros;
      int* data = COOValue(&toLoad);
      //printf("Val: %x\n",data);
      
      ACCEL_TOP_val_perA = numberItems;

      // B - versat side
      ACCEL_TOP_val_iterB = numberItems;
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
      int* data = &vec.data[xPos * blockSize];

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
      ACCEL_TOP_vector_iterB = numberItems;
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
   Array<int> values = COOValueArray(&coo);

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

   #if 0
   if(coo.column.size == 1){ // Accel cannot deal with only one value, I think
      res[b.y * block.blockSize + coo.row[0]] += coo.values[0] * vec[b.x * block.blockSize + coo.column[0]];
      printf("%d %d %d\n",coo.row[0],coo.row[0] + b.x * block.blockSize,res[coo.row[0]]);
      return;
   }
   #endif

   ConfigureAccelerator(coo,&coo,b.x,block->blockSize);

   RunAccelerator(2);

   int amountGot = ACCEL_TOP_output_stored;
   for(int ii = 0; ii < amountGot; ii++){
      int addr = VersatUnitRead(TOP_pos_addr,ii);
      res[b.y * block->blockSize + addr] += VersatUnitRead(TOP_output_addr,ii);
      printf("%d %d %d\n",addr,b.y * block->blockSize + addr,res[b.y * block->blockSize + addr]);
   }
#endif
}

static Array<int> DoOneFullyTest(int index,Arena* out){
   Array<int> res = PushArray<int>(out,size);
   Memset(res,0);

   DoOneFully(res,index);

   return res;
}

static void ConfigAccelerator(int toLoad,int toRun){
   Assert(toRun >= 0);
   
   int blockSize = block->blockSize;
   Array<Block> blocks = MatrixBlockArray(block);

   Block& load = blocks[toLoad];
   Block& runBlock = blocks[toRun];
   FormatCOO* runCOO = &runBlock.coo;

#ifdef PRINT
   printf("Y: %d X: %d\n",runBlock.y,runBlock.x);
#endif
   
   //printf("ToLoad %d ToRun %d\n",toLoad,toRun);
   ConfigureAccelerator(load.coo,runCOO,load.x,blockSize);
}

#ifndef PC
extern int timesWaiting;
#endif

void SingleTest(Arena* arena){
   InitializeSMVM(arena,Type::BLOCK);

   InitializeBaseAccelerator();

   Array<int> expected = {};
   timeRegion("CPU multiply"){
      expected = MultiplyBlock(block,vec,arena);
#ifdef TEST
      //PushExpected(expected);
#endif
   }
   printf("Make sure compiler does not optimize away: %d\n",expected[0]);

   // Initialize ILA
   ila_set_different_signal_storing(1);
   ila_set_time_offset(-1);
   ila_set_trigger_type(1,ILA_TRIGGER_TYPE_CONTINUOUS);
   
   Array<int> test = PushArray<int>(arena,size);
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   int loops = block->numberBlocks;
   int blockSize = block->blockSize;
   Array<Block> blocks = MatrixBlockArray(block);
   {
      TIME_IT("Accelerator Run");

#define START_FRAME 0

      {
         Block& load = blocks.data[START_FRAME]; // toLoad is 0
         ConfigureAccelerator(load.coo,nullptr,load.x,blockSize); // 0 - nullptr
         StartAccelerator(); // Accelerator is loading 0 in the background
      }

#if 1
      if(blocks.size > 1){
         Block& load = blocks.data[1];  // toLoad is 1
         Block& run = blocks.data[START_FRAME];  // toRun is 0
         
         ConfigureAccelerator(load.coo,&run.coo,load.x,blockSize); // 1 - 0 - 0

         EndAccelerator(); // Wait for accelerator to terminate (loading 0)
         StartAccelerator(); // Accelerator is loading 1 and running 0
      } else {
         Block& run = blocks.data[START_FRAME];  // toRun is 0
         ConfigureAccelerator(run.coo,&run.coo,run.x,blockSize); // 1 - 0 - 0
         DisableReads();
         
         EndAccelerator(); // Wait for accelerator to terminate (loading 0)
         StartAccelerator(); // Accelerator is loading 1 and running 0
      }
#endif
      
#if 0
      {
         Block& load = blocks[2]; // toLoad is 2
         Block& runBlock = blocks[1]; // toRun is 1

         ConfigureAccelerator(load.coo,&runBlock.coo,load.x,blockSize); // 2 - 1 - 

         EndAccelerator(); // Wait for accelerator to terminate (loading 0)
      }
      
      int toLoad = 3;
      int toRun = 2;
#else
      int toLoad = 2;
      int toRun = 1;
#endif
      
      ila_set_trigger_enabled(1,true); // Ila start recording. Only a small overhead cost anyway

      for(int currentRun = 1; currentRun < loops; currentRun++){
         StartAccelerator(); // Accelerator is loading 1 and running 0

         Block& runBlock = blocks[toRun]; // toRun is 0
         Block& load = blocks[toLoad];

         toRun = toLoad; // Next toRun is 1
         toLoad = std::min(toLoad + 1,loops - 1); // Bound toLoad to maximum value

         ConfigureAccelerator(load.coo,&runBlock.coo,load.x,blockSize); // 3 - 2

         Block& toSave = blocks[currentRun-1];
         int yOffset = toSave.y * blockSize; // run block from 0
         int amountGot = ACCEL_TOP_output_stored;

         for(int ii = 0; ii < amountGot; ii++){
            int addr = VersatUnitRead(TOP_pos_addr,ii);
            int value = VersatUnitRead(TOP_output_addr,ii);
            res[yOffset + addr] += value;
         }

         EndAccelerator(); // Wait for accelerator to terminate (loading 0)
      }

      EndAccelerator(); // Wait for accelerator to terminate (loading 0)
      DisableReads(); // Stops uncessary work in embedded and makes sure that pc-emul does access bad pointer addresses
      StartAccelerator(); // Pretty much only job is to change pingpongState to allow CPU to read data

      {
         Block& toSave = blocks[blocks.size-1];
         int yOffset = toSave.y * blockSize; // run block from 0
         int amountGot = ACCEL_TOP_output_stored;

         for(int ii = 0; ii < amountGot; ii++){
            int addr = VersatUnitRead(TOP_pos_addr,ii);
            int value = VersatUnitRead(TOP_output_addr,ii);

            res[yOffset + addr] += value;
         }
      }
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
