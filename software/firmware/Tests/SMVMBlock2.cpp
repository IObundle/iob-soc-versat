#define BLOCK_XY

#include "SMVM.hpp"

#define CHANGE
//#define PRINT
//#define TEST

void InitializeBaseAccelerator(){
   //ACCEL_TOP_cycler_amount = size + 24;

   ACCEL_TOP_gen_iterations = 1;
   ACCEL_TOP_gen_start = 0;
   ACCEL_TOP_gen_shift = 1;
   ACCEL_TOP_gen_incr = 1;
   //ACCEL_TOP_gen_off_value = 0;

   ACCEL_TOP_col_incrA = 1;
   ACCEL_TOP_col_iterA = 1;
   ACCEL_TOP_col_size = 8;
   ACCEL_TOP_col_int_addr = 0;
   ACCEL_TOP_col_pingPong = 1;

   // B - versat side
   ACCEL_TOP_col_incrB = 1;
   ACCEL_TOP_col_perB = 1;
   ACCEL_TOP_col_dutyB = 1;

   ACCEL_TOP_row_incrA = 1;
   ACCEL_TOP_row_iterA = 1;
   ACCEL_TOP_row_size = 8;
   ACCEL_TOP_row_int_addr = 0;
   ACCEL_TOP_row_pingPong = 1;

   // B - versat side
   ACCEL_TOP_row_incrB = 1;
   ACCEL_TOP_row_perB = 1;
   ACCEL_TOP_row_dutyB = 1;

   ACCEL_TOP_val_incrA = 1;
   ACCEL_TOP_val_iterA = 1;
   ACCEL_TOP_val_size = 8;
   ACCEL_TOP_val_int_addr = 0;
   ACCEL_TOP_val_pingPong = 1;

   // B - versat side
   ACCEL_TOP_val_incrB = 1;
   ACCEL_TOP_val_perB = 1;
   ACCEL_TOP_val_dutyB = 1;

   ACCEL_TOP_vector_incrA = 1;
   ACCEL_TOP_vector_iterA = 1;
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

void ConfigureAccelerator(FormatCOO& toLoad,FormatCOO* toRun,int xPos,int blockSize){
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
      //printf("Col: %p %d\n",toLoad.column.data,toLoad.column.size);
      int numberItems = toLoad.nonZeros;
      int* data = COOColumn(&toLoad);

      ACCEL_TOP_col_perA = numberItems;
      ACCEL_TOP_col_dutyA = numberItems;

      // B - versat side
      ACCEL_TOP_col_iterB = numberItems;
      ACCEL_TOP_col_ext_addr = (iptr) data;
      ACCEL_TOP_col_length = numberItems - 1; // AXI requires length of len - 1
   }

   //ConfigureSimpleVRead(row,toLoad.row.size,toLoad.row.data);
   // Memory side
   {
      //printf("Row: %p %d\n",toLoad.row.data,toLoad.row.size);
      int numberItems = toLoad.nonZeros + 1;
      int* data = COORow(&toLoad);

      ACCEL_TOP_row_perA = numberItems;
      ACCEL_TOP_row_dutyA = numberItems;

      // B - versat side
      ACCEL_TOP_row_iterB = numberItems;
      ACCEL_TOP_row_ext_addr = (iptr) data;
      ACCEL_TOP_row_length = numberItems - 1; // AXI requires length of len - 1

      Assert(numberItems - 1 <= 0xff);
   }

   //ConfigureSimpleVRead(val,toLoad.values.size,toLoad.values.data);
   // Memory side
   {
      //printf("Val: %p %d\n",toLoad.values.data,toLoad.values.size);
      int numberItems = toLoad.nonZeros;
      int* data = COOValue(&toLoad);

      ACCEL_TOP_val_perA = numberItems;
      ACCEL_TOP_val_dutyA = numberItems;

      // B - versat side
      ACCEL_TOP_val_iterB = numberItems;
      ACCEL_TOP_val_ext_addr = (iptr) data;
      ACCEL_TOP_val_length = numberItems - 1; // AXI requires length of len - 1

      Assert(numberItems - 1 <= 0xff);
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
      ACCEL_TOP_vector_dutyA = numberItems;

      // B - versat side
      ACCEL_TOP_vector_iterB = numberItems;
      ACCEL_TOP_vector_ext_addr = (iptr) data;
      ACCEL_TOP_vector_length = numberItems - 1; // AXI requires length of len - 1

      Assert(numberItems - 1 <= 0xff);
      
      lastLoadedX = xPos;
   }
}

void DoOneFully(Array<int> res,int index){
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

Array<int> DoOneFullyTest(int index,Arena* out){
   Array<int> res = PushArray<int>(out,size);
   Memset(res,0);

   DoOneFully(res,index);

   return res;
}

static inline void ConfigAccelerator(int toLoad,int toRun){
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

   timeRegion("CPU multiply"){
      Array<int> expected = MultiplyBlock(block,vec,arena);
      PushExpected(expected);
   }

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

      int startFrame = 0;
      int toLoad = startFrame;

      // Start the initial load of data
      Block& load = blocks[toLoad];
      ConfigureAccelerator(load.coo,nullptr,load.x,blockSize);
      StartAccelerator(); // Accelerator is loading 0 in the background

      // Logic for next load
      toLoad += 1;
      toLoad = std::min(toLoad,loops - 1);

      int toRun = startFrame;
      ConfigAccelerator(toLoad,toRun); // Accelerator is configured to load 1 and run 0

      for(int index = 0; index < loops; index++){
         // Ila start recording
         
         EndAccelerator(); // Wait for accelerator to terminate (loading 0)
         
         if(index == 0){
            ila_set_trigger_enabled(1,true);
         }
         
         StartAccelerator(); // Accelerator is loading 1 and running 0

         Block& runBlock = blocks[toRun]; // toRun is 0
         int currentRun = toRun; // currentRun is 0
         toRun = toLoad; // Next toRun is 1

         // Find value for next iteration
         toLoad += 1;
         toLoad = std::min(toLoad,loops - 1); // Bound toLoad to maximum value

         ConfigAccelerator(toLoad,toRun); // Accelerator is configured to load 2 and run 1

         //#define TEST
#ifdef TEST
         Memset(test,0); // NOTE: DO NOT FORGET TO REMOVE AFTER
#endif

         if(currentRun >= 1){
            Block& toSave = blocks[currentRun-1];
            int yOffset = toSave.y * blockSize; // run block from 0
            int amountGot = ACCEL_TOP_output_stored;
#ifdef PRINT
            printf("Got:%d\n",amountGot);
#endif

            for(int ii = 0; ii < amountGot; ii++){
            int addr = VersatUnitRead(TOP_pos_addr,ii);
            int value = VersatUnitRead(TOP_output_addr,ii);

#ifdef PRINT
            printf("%x %x\n",value,addr);
#endif
            test[yOffset + addr] = value;
            res[yOffset + addr] += value;
            //printf("%d %d %d\n",addr,yOffset + addr,res[yOffset + addr]);
            }
         }

#ifdef TEST
         if(currentRun >= 1){
         region(arena){
            Array<int> good = DoOneFullyTest(currentRun - 1,arena);

            for(int i = 0; i < good.size; i++){
               if(good[i] || test[i]){
                  //Assert_Eq(good[i],test[i]);
                  if(good[i] != test[i]){
                     //printf("%d\n",currentRun);
                     printf("ERROR: %d %x %x %d\n",currentRun,good[i],test[i],i);
                     exit(0);
                  }
                  //printf("%d %d\n",good[i],test[i]);
               }
            }
         }
         }
#endif
      }
   }

   EndAccelerator(); // Wait for accelerator to terminate (loading 0)
   StartAccelerator(); // Accelerator is loading 1 and running 0
   
   {
   Block& toSave = blocks[blocks.size-1];
   int yOffset = toSave.y * blockSize; // run block from 0
   int amountGot = ACCEL_TOP_output_stored;
#ifdef PRINT
   printf("Got:%d\n",amountGot);
#endif

   for(int ii = 0; ii < amountGot; ii++){
      int addr = VersatUnitRead(TOP_pos_addr,ii);
      int value = VersatUnitRead(TOP_output_addr,ii);

#ifdef PRINT
      printf("%x %x\n",value,addr);
#endif
      test[yOffset + addr] = value;
      res[yOffset + addr] += value;
      //printf("%d %d %d\n",addr,yOffset + addr,res[yOffset + addr]);
   }
   }

   //ila_output_everything();
   
   PushGot(res);

#ifdef TEST
   printf("VecLoads: %d/%d\n",vectorLoads,blocks.size);
#endif 

#ifndef PC
   printf("Times waiting: %d\n",timesWaiting);
#endif
}
