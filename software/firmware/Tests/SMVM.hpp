#include "testbench.h"

#include "memory.hpp"
#include "utils.hpp"

typedef float real;

#ifdef SIM
// Because compilation with .cpp files does not work as the embedded system does not include the necessary files.
// Just copy the implementations for now.
void FlushStdout(){
  fflush(stdout);
}

static char* GetNumberRepr(uint64 number){
  static char buffer[32];

  if(number == 0){
    buffer[0] = '0';
    buffer[1] = '\0';
    return buffer;
  }

  uint64 val = number;
  buffer[31] = '\0';
  int index = 30;
  while(val){
    buffer[index] = '0' + (val % 10);
    val /= 10;
    index -= 1;
  }

  return &buffer[index+1];
}

int NumberDigitsRepresentation(int64 number){
  int nDigits = 0;

  if(number == 0){
    return 1;
  }

  int64 num = number;
  if(num < 0){
    nDigits += 1; // minus sign
    num *= -1;
  }

  while(num > 0){
    num /= 10;
    nDigits += 1;
  }

  return nDigits;
}

int GetMaxDigitSize(Array<int> array){
  int maxReprSize = 0;
  for(int val : array){
    maxReprSize = std::max(maxReprSize,NumberDigitsRepresentation(val));
  }

  return maxReprSize;
}

int GetMaxDigitSize(Array<float> array){
  return 2; // Floating points is hard to figure out how many digits. 2 should be enough
}

void TimeIt::Output(){
  // Cannot use floating point because of embedded
  Time end = GetTime();

  Assert(end > start);

  printf("[TimeIt] %s: ",id);

  {
    uint64 diff = end.seconds - start.seconds;
    int digits = NumberDigitsRepresentation(diff);
    char* secondsRepr = GetNumberRepr(diff);

    for(int i = 0; i < digits; i++){
      printf("%c",secondsRepr[i]);
    }
  }
  printf(".");
  {
    uint64 diff = end.nanoSeconds - start.nanoSeconds;
    int digits = NumberDigitsRepresentation(diff);
    char* nanoRepr = GetNumberRepr(diff);

    for(int i = 0; i < digits; i++){
      printf("%c",nanoRepr[i]);
    }
  }
  printf("\n");
}

void Print(Array<float> array,int digitSize){
  if(digitSize == 0){
    digitSize = GetMaxDigitSize(array);
  }

  for(float val : array){
    printf("%.*f ",digitSize,val);
  }
}

void Print(Array<int> array,int digitSize){
  if(digitSize == 0){
    digitSize = GetMaxDigitSize(array);
  }

  for(int val : array){
    printf("%*d ",digitSize,val);
  }
}
#endif

static Array<real> RandomMatrix(Arena* out,int size,int nonZeros,int randomSeed){
   SeedRandomNumber(randomSeed);

   int nElements = size * size;

   // Cannot meet the specifications. Return empty
   if(nonZeros > nElements || nonZeros < size){
      return (Array<real>){};
   }

   Array<real> res = PushArray<real>(out,nElements);

   // Make sure that every column and every row has at least a non zero
   for(int i = 0; i < size; i++){
      res[i * size + i] = RandomNumberBetween(1,10);
   }
   int zerosLeft = nonZeros - size;

   while(zerosLeft){
      int y = RandomNumberBetween(0,size);
      int x = RandomNumberBetween(0,size);

      if(res[y * size + x] == 0){
        res[y * size + x] = (real) RandomNumberBetween(1,10);
        zerosLeft -= 1;
      }
   }

   return res;
}

static Array<real> RandomVec(Arena* out,int size,int randomSeed){
   SeedRandomNumber(randomSeed);

   Array<real> vec = PushArray<real>(out,size);

   for(int i = 0; i < size; i++){
     vec[i] = (real) RandomNumberBetween(1,size*size);
   }

   return vec;
}

static Array<real> ExampleMatrix(Arena* arena){
   Array<real> res = PushArray<real>(arena,25);

   Memset(res,0.0f);
   res[0 * 5 + 0] = 1.0;
   res[0 * 5 + 1] = 2.0;
   res[0 * 5 + 3] = 11.0;
   res[1 * 5 + 1] = 3.0;
   res[1 * 5 + 2] = 4.0;
   res[2 * 5 + 1] = 5.0;
   res[2 * 5 + 2] = 6.0;
   res[2 * 5 + 3] = 7.0;
   res[3 * 5 + 3] = 8.0;
   res[4 * 5 + 3] = 9.0;
   res[4 * 5 + 4] = 10.0;

   return res;
}

static int DigitSize(Array<real> arr){
   int size = 0;
   for(real val : arr){
      int s = NumberDigitsRepresentation(val);
      size = std::max(size,s);
   }
   return size;
}

void PrintArray(Array<real> arr, int digitSize){
   for(real val : arr){
      printf("%*f ",digitSize,val);
   }
}

void Identity(Array<real> matrix,int size){
   Memset(matrix,0.0f);

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

int NonZero(Array<real> matrix){
   int count = 0;
   for(real val : matrix){
     if(std::abs(val) < 0.00001f) count++;
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
   //Array<real> values;
};

#define COOColumn(COO_PTR) ((COO_PTR)->data)
#define COORow(COO_PTR)    ((COO_PTR)->data + (COO_PTR)->nonZeros)
#define COOValue(COO_PTR)  ((real*) ((COO_PTR)->data + (COO_PTR)->nonZeros * 2 + 1))

#define COOColumnArray(COO_PTR) {COOColumn(COO_PTR),(COO_PTR)->nonZeros}
#define COORowArray(COO_PTR)    {COORow(COO_PTR)   ,(COO_PTR)->nonZeros + 1}
#define COOValueArray(COO_PTR)  {COOValue(COO_PTR) ,(COO_PTR)->nonZeros}

struct FormatCSR{
  int* data;
  int rowsAmount;
  int nonZeros;
  int _align0;

  // Array<real> values;
  // Array<int> column;
  // Array<int> row;
};

#define CSRValue(CSR_PTR) ((real*) (CSR_PTR)->data)
#define CSRColumn(CSR_PTR) ((CSR_PTR)->data + (CSR_PTR)->nonZeros)
#define CSRRow(CSR_PTR) ((CSR_PTR)->data + (CSR_PTR)->nonZeros * 2)

#define CSRValueArray(CSR_PTR) {CSRValue(CSR_PTR),(CSR_PTR)->nonZeros}
#define CSRColumnArray(CSR_PTR) {CSRColumn(CSR_PTR),(CSR_PTR)->nonZeros}
#define CSRRowArray(CSR_PTR) {CSRRow(CSR_PTR),(CSR_PTR)->rowsAmount}

int CSRSize(int nonZeros,int rows){
  int size = sizeof(float) * nonZeros + sizeof(int) * nonZeros + sizeof(int) * rows;
  return size;
}

void Print(FormatCOO* coo){
   Array<int> column = COOColumnArray(coo);
   Array<int> row    = COORowArray(coo);
   Array<real> values = COOValueArray(coo);

#ifndef PC
   printf("%d %d %d %d\n",(int) coo->data,coo->_align0,coo->nonZeros,coo->_align1);
#endif

   printf("Coo: %d %d %d\n",column.size,row.size,values.size);
   printf("Col: "); Print(column); printf("\n");
   printf("Row: "); Print(row); printf("\n");
   printf("Val: "); Print(values); printf("\n");
}

void Print(FormatCSR* csr){
   Array<int> column = CSRColumnArray(csr);
   Array<int> row    = CSRRowArray(csr);
   Array<real> values = CSRValueArray(csr);

#ifndef PC
  printf("%d %d %d %d\n",(int) csr->data,csr->rowsAmount,csr->nonZeros,csr->_align0);
#endif

   printf("CSR: %d %d %d\n",column.size,row.size,values.size);
   printf("Col: "); Print(column); printf("\n");
   printf("Row: "); Print(row); printf("\n");
   printf("Val: "); Print(values); printf("\n");
}


FormatCOO ConvertCOO(Array<real> matrix,int size,Arena* arena){
   int nonZero = NonZero(matrix);

   FormatCOO res = {};
   res.data = PushArray<int>(arena,nonZero * 3 + 1).data;
   res.nonZeros = nonZero;

   Array<int> column = COOColumnArray(&res);
   Array<int> row    = COORowArray(&res);
   Array<real> values = COOValueArray(&res);

   int index = 0;
   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         real val = matrix[y * size + x];
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

Array<int> Rows(Array<real> matrix,int size,Arena* arena){
  Array<int> rowCount = PushArray<int>(arena,size);
  Memset(rowCount,0);

  for(int y = 0; y < size; y++){
    for(int x = 0; x < size; x++){
      real val = matrix[y * size + x];

      if(val){
        rowCount[y] += 1;
      }
    }
  }

  return rowCount;
}

int RowsCount(Array<real> matrix,int size,Arena* arena){
  BLOCK_REGION(arena);

  Array<bool> seenRow = PushArray<bool>(arena,size);
  Memset(seenRow,false);

  int count = 0;
  for(int y = 0; y < size; y++){
    for(int x = 0; x < size; x++){
      real val = matrix[y * size + x];

      if(val){
        if(!seenRow[y]){
          count += 1;
          seenRow[y] = true;
        }
      }
    }
  }

  return count;
}

FormatCSR ConvertCSR(Array<real> matrix,int size,Arena* arena){
   int nonZero = NonZero(matrix);
   int rows = RowsCount(matrix,size,arena);

   FormatCSR res = {};
   res.data = PushArray<int>(arena,CSRSize(nonZero,rows)).data;
   res.nonZeros = nonZero;
   res.rowsAmount = rows;

   Array<int> column = CSRColumnArray(&res);
   Array<int> row    = CSRRowArray(&res);
   Array<real> values = CSRValueArray(&res);

   int index = 0;
   int rowIndex = 0;
   for(int y = 0; y < size; y++){
      bool seenFirst = true;
      for(int x = 0; x < size; x++){
         real val = matrix[y * size + x];
         if(val){
           if(seenFirst){
             if(index != 0){
               row[rowIndex++] = index;
             }
             seenFirst = false;
           }

            column[index] = x;
            values[index] = val;
            index += 1;
         }
      }
   }
   row[rowIndex] = index;
   Assert(index == nonZero);

   return res;
}

Array<real> Multiply(Array<real> matrix,int size,Array<real> vector,Arena* arena){
   Array<real> res = PushArray<real>(arena,size);
   Memset(res,0.0f);

   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         res[y] += vector[x] * matrix[y * size + x];
      }
   }

   return res;
}

Array<real> MultiplyCOO(FormatCOO coo,Array<real> vector,Arena* arena){
   int size = vector.size;
   Array<real> res = PushArray<real>(arena,size);
   Memset(res,0.0f);

   Array<int> column = COOColumnArray(&coo);
   Array<int> row    = COORowArray(&coo);
   Array<real> values = COOValueArray(&coo);

   for(int i = 0; i < values.size; i++){
      res[row[i]] += values[i] * vector[column[i]];
   }
   return res;
}

Array<real> MultiplyCSR(FormatCSR csr,Array<real> vector,Arena* arena){
   int size = vector.size;
   Array<real> res = PushArray<real>(arena,size);
   Memset(res,0.0f);

   Array<int> column = CSRColumnArray(&csr);
   Array<int> row    = CSRRowArray(&csr);
   Array<real> values = CSRValueArray(&csr);

   for(int i = 0; i < row.size; i++){
      int val = 0;
      for(int ii = row[i]; ii < row[i+1]; ii++){
         val += values[ii] * vector[column[ii]];
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
   Array<real> values = COOValueArray(&res);

   BLOCK_REGION(temp);

   Array<real> matrix = PushArray<real>(temp,size * size);
   Memset(matrix,0.0f);

   SeedRandomNumber(1);
   for(int i = 0; i < amountOfNZ;){
      int x = RandomNumberBetween(0,size-1);
      int y = RandomNumberBetween(0,size-1);

      if(matrix[y * size + x] != 0){
         continue;
      }

      matrix[y * size + x] = RandomNumberBetween(1,size*size);
      i += 1;
   }

   int index = 0;
   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         real val = matrix[y * size + x];
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

void PushExpected(Array<real> vec){
   for(real val : vec){
      PushExpectedF(val);
   }
}

void PushGot(Array<real> vec){
   for(real val : vec){
      PushGotF(val);
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

struct BlockCSR{
   int x;
   int y;
   FormatCSR csr;
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

#define MatrixBlockCSR(MATRIX_PTR) ((BlockCSR*) (&(MATRIX_PTR)[1]))
#define MatrixBlockCSRArray(MATRIX_PTR) {MatrixBlockCSR(MATRIX_PTR),(MATRIX_PTR)->numberBlocks}

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

MatrixBlock* UnpackMatrixBlockCSR(void* base){
  NOT_IMPLEMENTED;
#if 0
  MatrixBlock* matrix = (MatrixBlock*) base;

   Array<Block> blocks = MatrixBlockArray(matrix);

   int* startOfCOOData = (int*) &blocks.data[blocks.size];

   for(Block& block : blocks){
      block.coo.data = startOfCOOData;

      int size = block.coo.nonZeros * 3 + 1;
      startOfCOOData += size;
   }

   return matrix;
#endif
   return nullptr;
}

static const int MAXIMUM_NZ_PER_BLOCK = 500;

// Default values for random example
static int size = 4;
static int amountNZ = 16;
static int blockSize = 3;

MatrixBlock* ConvertMatBlock(Array<real> mat,int size,Arena* arena){
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

         if(nonZeros > MAXIMUM_NZ_PER_BLOCK){
            printf("Higher than %d\n",MAXIMUM_NZ_PER_BLOCK);
         }

         FormatCOO coo = {};
         coo.nonZeros = nonZeros;
         coo.data = PushArray<int>(arena,nonZeros * 3 + 1).data;

         Array<int> column = COOColumnArray(&coo);
         Array<int> row    = COORowArray(&coo);
         Array<real> values = COOValueArray(&coo);

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

MatrixBlock* ConvertMatBlockCSR(Array<real> mat,int size,Arena* arena){
   int numberOfBlocks = ((size / blockSize) + 1) * ((size / blockSize) + 1);

   MatrixBlock* matrix = PushStruct<MatrixBlock>(arena);
   matrix->blockSize = blockSize;

   Array<BlockCSR> blocks = PushArray<BlockCSR>(arena,numberOfBlocks);

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

         if(nonZeros > MAXIMUM_NZ_PER_BLOCK){
            printf("Higher than %d\n",MAXIMUM_NZ_PER_BLOCK);
         }

         int rows = 0;
         region(arena){
           Array<bool> seenRow = PushArray<bool>(arena,size);
           Memset(seenRow,false);

           for(int y = yStart; y < std::min(yStart+blockSize,size); y++){
             for(int x = xStart; x < std::min(xStart+blockSize,size); x++){
               real val = mat[y * size + x];

               if(val){
                 if(!seenRow[y]){
                   rows += 1;
                   seenRow[y] = true;
                 }
               }
             }
           }
         }

         FormatCSR csr = {};
         csr.data = PushArray<int>(arena,CSRSize(nonZeros,rows)).data;
         csr.nonZeros = nonZeros;
         csr.rowsAmount = rows;

         Array<int> column = CSRColumnArray(&csr);
         Array<int> row    = CSRRowArray(&csr);
         Array<real> values = CSRValueArray(&csr);

         int index = 0;
         int rowIndex = 0;
         for(int y = yStart; y < std::min(yStart+blockSize,size); y++){
           bool seenFirst = true;
            for(int x = xStart; x < std::min(xStart+blockSize,size); x++){
             real val = mat[y * size + x];
             if(val){
               if(seenFirst){
                 if(index != 0){
                  row[rowIndex++] = index;
                 }
                 seenFirst = false;
               }

               column[index] = x - xStart;
               values[index] = val;
               index += 1;
             }
           }
         }
         row[rowIndex] = index;
         Assert(index == nonZeros);

         blocks[blockIndex].y = yStart / blockSize;
         blocks[blockIndex].x = xStart / blockSize;
         blocks[blockIndex].csr = csr;
         blockIndex += 1;
      }
   }

   matrix->numberBlocks = blockIndex; // Since zero blocks are removed, update index to reflect that

   return matrix;
}

Array<real> MultiplyBlock(MatrixBlock* block,Array<real> vector,Arena* arena){
   int size = vector.size;
   Array<real> res = PushArray<real>(arena,size);
   Memset(res,0.0f);

   Array<Block> blocks = MatrixBlockArray(block);

   for(Block& b : blocks){
      FormatCOO coo = b.coo;

      Array<int> column = COOColumnArray(&coo);
      Array<int> row    = COORowArray(&coo);
      Array<real> values = COOValueArray(&coo);

      int yOffset = b.y * block->blockSize;
      int xOffset = b.x * block->blockSize;
      for(int i = 0; i < values.size; i++){
         res[yOffset + row[i]] += values[i] * vector[xOffset + column[i]];
      }
   }
   return res;
}

Array<real> MultiplyBlockCSR(MatrixBlock* block,Array<real> vector,Arena* arena){
   int size = vector.size;
   Array<real> res = PushArray<real>(arena,size);
   Memset(res,0.0f);

   Array<BlockCSR> blocks = MatrixBlockCSRArray(block);

   for(BlockCSR& b : blocks){
      FormatCSR csr = b.csr;

      Array<real> values = CSRValueArray(&csr);
      Array<int> column = CSRColumnArray(&csr);
      Array<int> row    = CSRRowArray(&csr);

      int yOffset = b.y * block->blockSize;
      int xOffset = b.x * block->blockSize;
      int start = 0;
      for(int i = 0; i < csr.rowsAmount; i++){
        float val = 0;
        for(int ii = start; ii < row[i]; ii++){
          val += values[ii] * vector[xOffset + column[ii]];
        }
        res[yOffset + i] = val;
        start = row[i];
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

void PrintCSR(MatrixBlock* block){
   Array<BlockCSR> blocks = MatrixBlockCSRArray(block);

   printf("Blocks: %d\n",blocks.size);

   for(BlockCSR& b : blocks){
      printf("%d %d ",b.y,b.x);
      Print(&b.csr);
      printf("\n");
   }
}

Array<real> vec;
Array<real> mat;
FormatCSR csr;
FormatCOO coo;
MatrixBlock* block;

enum Type {COO,CSR,BLOCK,BLOCKCSR};

//set pointer to DDR base
#if (RUN_EXTMEM==0)  //running firmware from SRAM
    #define DATA_BASE_ADDR (EXTRA_BASE)
#else //running firmware from DDR
    #define DATA_BASE_ADDR ((1<<(FIRM_ADDR_W)))
#endif

void InitializeSMVM(Arena* arena,Type type){
   // Read from file
   int val = 0;

   Assert(sizeof(int) == sizeof(real));

   printf("here %d\n",val);
   if(val){
#ifdef PC
      String content = PushFile(arena,"../../../../PWTK");
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
      vec = PushArray<real>(arena,size);

      //printf("Vec: ");
      for(int i = 0; i < size; i++){
         vec[i] = RandomNumberBetween(1,size);

         //printf("%d ",vec[i]);
      }
      //printf("\n");

   } else {
      vec = PushArray<real>(arena,size);
      for(int i = 0; i < size; i++){
        vec[i] = (real) RandomNumberBetween(1,size);
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
         Print(block);
      }break;
      case Type::BLOCKCSR:{
         block = ConvertMatBlockCSR(mat,size,arena);
         PrintCSR(block);
      }break;
      }

      region(arena){
        Array<real> expected = Multiply(mat,size,vec,arena);
        PushExpected(expected);
      }

      printf("Finished SMVM init\n");
   }
}
