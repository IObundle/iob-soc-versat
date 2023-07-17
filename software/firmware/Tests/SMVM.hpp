#include "testbench.h"

#include "memory.hpp"
#include "utils.hpp"

static Array<int> RandomMatrix(Arena* out,int size,int nonZeros,int randomSeed){
   SeedRandomNumber(randomSeed);

   int nElements = size * size;

   // Cannot meet the specifications. Return empty
   if(nonZeros > nElements || nonZeros < size){
      return (Array<int>){};
   }

   Array<int> res = PushArray<int>(out,nElements);

   // Make sure that every column and every row has at least a non zero
   for(int i = 0; i < size; i++){
      res[i * size + i] = RandomNumberBetween(1,10,GetRandomNumber());
   }
   int zerosLeft = nonZeros - size;

   while(zerosLeft){
      int y = RandomNumberBetween(0,size,GetRandomNumber());
      int x = RandomNumberBetween(0,size,GetRandomNumber());

      if(res[y * size + x] == 0){
         res[y * size + x] = RandomNumberBetween(1,10,GetRandomNumber());
         zerosLeft -= 1;
      }
   }

   return res;
}

static Array<int> RandomVec(Arena* out,int size,int randomSeed){
   SeedRandomNumber(randomSeed);

   Array<int> vec = PushArray<int>(out,size);

   for(int i = 0; i < size; i++){
      vec[i] = RandomNumberBetween(1,size*size,GetRandomNumber());
   }

   return vec;
}

static Array<int> ExampleMatrix(Arena* arena){
   Array<int> res = PushArray<int>(arena,25);

   Memset(res,0);
   res[0 * 5 + 0] = 1;
   res[0 * 5 + 1] = 2;
   res[0 * 5 + 3] = 11;
   res[1 * 5 + 1] = 3;
   res[1 * 5 + 2] = 4;
   res[2 * 5 + 1] = 5;
   res[2 * 5 + 2] = 6;
   res[2 * 5 + 3] = 7;
   res[3 * 5 + 3] = 8;
   res[4 * 5 + 3] = 9;
   res[4 * 5 + 4] = 10;

   return res;
}

static int DigitSize(Array<int> arr){
   int size = 0;
   for(int val : arr){
      int s = NumberDigitsRepresentation(val);
      size = std::max(size,s);
   }
   return size;
}

void PrintArray(Array<int> arr, int digitSize){
   for(int val : arr){
      printf("%*d ",digitSize,val);
   }
}

void Identity(Array<int> matrix,int size){
   Memset(matrix,0);

   for(int i = 0; i < size; i++){
      matrix[i * size + i] = 1;
   }
}

void PrintMatrix(Array<int> matrix,int size){
   int digitSize = GetMaxDigitSize(matrix);
   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         printf("%*d ",digitSize,matrix[y * size + x]);
      }
      printf("\n");
   }
}

int NonZero(Array<int> matrix){
   int count = 0;
   for(int val : matrix){
      if(val) count++;
   }
   return count;
}

struct FormatCOO{
   int* data;

#ifndef PC
   int _align0; // Because pointers are 8 bytes in PC, need to add a small int to align data.
#endif

   int nonZeros;
   int _align1;

   //int size;
   //Array<int> row;
   //Array<int> column;
   //Array<int> values;
};

#define COOColumn(COO_PTR) ((COO_PTR)->data)
#define COORow(COO_PTR)    ((COO_PTR)->data + (COO_PTR)->nonZeros)
#define COOValue(COO_PTR)  ((COO_PTR)->data + (COO_PTR)->nonZeros * 2 + 1)

#define COOColumnArray(COO_PTR) {COOColumn(COO_PTR),(COO_PTR)->nonZeros}
#define COORowArray(COO_PTR)    {COORow(COO_PTR)   ,(COO_PTR)->nonZeros + 1}
#define COOValueArray(COO_PTR)  {COOValue(COO_PTR) ,(COO_PTR)->nonZeros}

void Print(FormatCOO* coo){
   Array<int> column = COOColumnArray(coo);
   Array<int> row    = COORowArray(coo);
   Array<int> values = COOValueArray(coo);

#ifndef PC
   printf("%d %d %d %d\n",(int) coo->data,coo->_align0,coo->nonZeros,coo->_align1);
#endif
   
   printf("Coo: %d %d %d\n",column.size,row.size,values.size);
   printf("Col: "); Print(column); printf("\n");
   printf("Row: "); Print(row); printf("\n");
   printf("Val: "); Print(values); printf("\n");
}

FormatCOO ConvertCOO(Array<int> matrix,int size,Arena* arena){
   int nonZero = NonZero(matrix);

   FormatCOO res = {};
   res.data = PushArray<int>(arena,nonZero * 3 + 1).data;
   res.nonZeros = nonZero;

   Array<int> column = COOColumnArray(&res);
   Array<int> row    = COORowArray(&res);
   Array<int> values = COOValueArray(&res);

   int index = 0;
   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         int val = matrix[y * size + x];
         if(val){
            row[index] = y;
            column[index] = x;
            values[index] = val;
            index += 1;
         }
      }
   }
   Assert(index == nonZero);
   row[index] = -1;

   return res;
}

struct FormatCSR{
   int size;
   Array<int> row;
   Array<int> column;
   Array<int> values;
};

FormatCSR ConvertCSR(Array<int> matrix,int size,Arena* arena){
   int nonZero = NonZero(matrix);

   FormatCSR res = {};
   res.row = PushArray<int>(arena,size + 1);
   res.column = PushArray<int>(arena,nonZero);
   res.values = PushArray<int>(arena,nonZero);
   res.size = size;

   int index = 0;
   int rowIndex = 0;
   for(int y = 0; y < size; y++){
      bool seenFirst = true;
      for(int x = 0; x < size; x++){
         int val = matrix[y * size + x];
         if(val){
            if(seenFirst){
               res.row[rowIndex++] = index;
               seenFirst = false;
            }

            res.column[index] = x;
            res.values[index] = val;
            index += 1;
         }
      }
   }
   res.row[rowIndex] = index;
   Assert(index == nonZero);

   return res;
}

Array<int> Multiply(Array<int> matrix,int size,Array<int> vector,Arena* arena){
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         res[y] += vector[x] * matrix[y * size + x];
      }
   }

   return res;
}

Array<int> MultiplyCOO(FormatCOO coo,Array<int> vector,Arena* arena){
   int size = vector.size;
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   Array<int> column = COOColumnArray(&coo);
   Array<int> row    = COORowArray(&coo);
   Array<int> values = COOValueArray(&coo);

   for(int i = 0; i < values.size; i++){
      res[row[i]] += values[i] * vector[column[i]];
   }
   return res;
}

Array<int> MultiplyCSR(FormatCSR csr,Array<int> vector,Arena* arena){
   int size = csr.size;
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   for(int i = 0; i < csr.size; i++){
      int val = 0;
      for(int ii = csr.row[i]; ii < csr.row[i+1]; ii++){
         val += csr.values[ii] * vector[csr.column[ii]];
      }
      res[i] = val;
   }
   return res;
}

FormatCOO RandomCOO(Arena* temp,int amountOfNZ,int matrixSize){
   int size = matrixSize;

   FormatCOO res = {};
   res.data = PushArray<int>(temp,amountOfNZ * 3 + 1).data;
   res.nonZeros = amountOfNZ;

   Array<int> column = COOColumnArray(&res);
   Array<int> row    = COORowArray(&res);
   Array<int> values = COOValueArray(&res);

   BLOCK_REGION(temp);

   Array<int> matrix = PushArray<int>(temp,size * size);
   Memset(matrix,0);

   SeedRandomNumber(1);
   for(int i = 0; i < amountOfNZ;){
      int x = RandomNumberBetween(0,size-1,GetRandomNumber());
      int y = RandomNumberBetween(0,size-1,GetRandomNumber());

      if(matrix[y * size + x] != 0){
         continue;
      }

      matrix[y * size + x] = RandomNumberBetween(1,size*size,GetRandomNumber());
      i += 1;
   }

   int index = 0;
   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         int val = matrix[y * size + x];
         if(val){
            row[index] = y;
            column[index] = x;
            values[index] = val;
            index += 1;
         }
      }
   }
   row[index] = -1;

   return res;
}

void PushExpected(Array<int> vec){
   for(int val : vec){
      PushExpectedI(val);
   }
}

void PushGot(Array<int> vec){
   for(int val : vec){
      PushGotI(val);
   }
}

// Should not be needed. Check if there is no bug elsewhere
void ClearCache(void* validStart){
#ifndef PC
   char* ptr = (char*) validStart;

   char count = 0;
   for(int i = 0; i < 1024 * 256; i += 16){
      count += ptr[i];
   }

   printf("Clear cache count: %d\n",(int) count);
#endif
}

// Follows the COO format
struct Block{
   int x;
   int y;
   FormatCOO coo;
};

struct MatrixBlock{
   int size;
   int blockSize;
   int numberBlocks;
   // Followed by array of blocks
   //Array<Block> blocks;
};

#define MatrixBlock(MATRIX_PTR) ((Block*) (&(MATRIX_PTR)[1]))
#define MatrixBlockArray(MATRIX_PTR) {MatrixBlock(MATRIX_PTR),(MATRIX_PTR)->numberBlocks}

MatrixBlock* UnpackMatrixBlock(void* base){
   MatrixBlock* matrix = (MatrixBlock*) base;

   Array<Block> blocks = MatrixBlockArray(matrix);

   int* startOfCOOData = (int*) &blocks.data[blocks.size];

   for(Block& block : blocks){
      block.coo.data = startOfCOOData;

      int size = block.coo.nonZeros * 3 + 1;
      startOfCOOData += size;
   }

   return matrix;
}

MatrixBlock* ConvertMatBlock(Array<int> mat,int size,Arena* arena){
   int blockSize = 32;

   int numberOfBlocks = ((size / blockSize) + 1) * ((size / blockSize) + 1);

   MatrixBlock* matrix = PushStruct<MatrixBlock>(arena);
   matrix->blockSize = blockSize;

   Array<Block> blocks = PushArray<Block>(arena,numberOfBlocks);

   int blockIndex = 0;
#ifdef BLOCK_XY
   for(int xStart = 0; xStart < size; xStart += blockSize){
      for(int yStart = 0; yStart < size; yStart += blockSize){
#else
   for(int yStart = 0; yStart < size; yStart += blockSize){
      for(int xStart = 0; xStart < size; xStart += blockSize){
#endif
         int nonZeros = 0;
         for(int y = yStart; y < std::min(yStart+blockSize,size); y++){
            for(int x = xStart; x < std::min(xStart+blockSize,size); x++){
               if(mat[y * size + x] != 0){
                  nonZeros += 1;
               }
            }
         }

         if(nonZeros == 0){ // Do not store info for empty blocks.
            continue;
         }

         if(nonZeros > 255){
            printf("Higher than 255\n");
         }

         FormatCOO coo = {};
         coo.nonZeros = nonZeros;
         coo.data = PushArray<int>(arena,nonZeros * 3 + 1).data;

         Array<int> column = COOColumnArray(&coo);
         Array<int> row    = COORowArray(&coo);
         Array<int> values = COOValueArray(&coo);

         int arrayIndex = 0;
         for(int y = yStart; y < std::min(yStart+blockSize,size); y++){
            for(int x = xStart; x < std::min(xStart+blockSize,size); x++){
               if(mat[y * size + x] != 0){
                  row[arrayIndex] = y - yStart;
                  column[arrayIndex] = x - xStart;
                  values[arrayIndex] = mat[y * size + x];
                  arrayIndex += 1;
               }
            }
         }
         row[arrayIndex] = -1;

         blocks[blockIndex].y = yStart / blockSize;
         blocks[blockIndex].x = xStart / blockSize;
         blocks[blockIndex].coo = coo;
         blockIndex += 1;
      }
   }

   matrix->numberBlocks = blockIndex; // Since zero blocks are removed, update index to reflect that

   return matrix;
}

Array<int> MultiplyBlock(MatrixBlock* block,Array<int> vector,Arena* arena){
   int size = vector.size;
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   Array<Block> blocks = MatrixBlockArray(block);

   for(Block& b : blocks){
      FormatCOO coo = b.coo;

      Array<int> column = COOColumnArray(&coo);
      Array<int> row    = COORowArray(&coo);
      Array<int> values = COOValueArray(&coo);

      int yOffset = b.y * block->blockSize;
      int xOffset = b.x * block->blockSize;
      for(int i = 0; i < values.size; i++){
         res[yOffset + row[i]] += values[i] * vector[xOffset + column[i]];
      }
   }
   return res;
}

void Print(MatrixBlock* block){
   Array<Block> blocks = MatrixBlockArray(block);

   printf("Blocks: %d\n",blocks.size);
   
   for(Block& b : blocks){
      printf("%d %d ",b.y,b.x);
      Print(&b.coo);
      printf("\n");
   }
}

int size = 100;
int amountNZ = 1000;
Array<int> vec;
Array<int> mat;
FormatCSR csr;
FormatCOO coo;
MatrixBlock* block;

enum Type {COO,CSR,BLOCK};

//set pointer to DDR base
#if (RUN_EXTMEM==0)  //running firmware from SRAM
    #define DATA_BASE_ADDR (EXTRA_BASE)
#else //running firmware from DDR
    #define DATA_BASE_ADDR ((1<<(FIRM_ADDR_W)))
#endif

void InitializeSMVM(Arena* arena,Type type){
   // Read from file
   int val = 0;

   printf("here %d\n",val);
   if(val){
#ifdef PC
      String content = PushFile(arena,"../../example1");
      block = UnpackMatrixBlock((void*) content.data);
#else
      eth_init(ETHERNET_BASE);
      
      char* buffer = (char*) DATA_BASE_ADDR;
      int* view = (int*) buffer;

      printf("%x\n",(int) buffer);
      printf("Waiting for file receive\n");

      //int size = uart_recvfile("example1",buffer);
      int size = eth_rcv_variable_file(buffer);

      printf("Received: %d\n",size);

      block = UnpackMatrixBlock(buffer);
#endif
      
      int numberBlocks = block->numberBlocks;
      
      size = block->size;
      vec = PushArray<int>(arena,size);

      //printf("Vec: ");
      for(int i = 0; i < size; i++){
         vec[i] = RandomNumberBetween(1,size,GetRandomNumber());

         //printf("%d ",vec[i]);
      }
      //printf("\n");

   } else {
      vec = PushArray<int>(arena,size);
      for(int i = 0; i < size; i++){
         vec[i] = RandomNumberBetween(1,size,GetRandomNumber());
      }

      mat = RandomMatrix(arena,size,amountNZ,1);
      printf("Generated mat\n");

      switch(type){
      case Type::COO:{
         coo = ConvertCOO(mat,size,arena);
      }break;
      case Type::CSR:{
         csr = ConvertCSR(mat,size,arena);
      }break;
      case Type::BLOCK:{
         block = ConvertMatBlock(mat,size,arena);
         //Print(block);
      }break;
      }

      region(arena){
         //Array<int> expected = Multiply(mat,size,vec,arena);
         //PushExpected(expected);
      }
      printf("Finished SMVM init\n");
   }
}
