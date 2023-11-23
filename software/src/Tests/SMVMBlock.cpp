#include "testbench.hpp"

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

typedef float real;

#include <cstdint>
typedef uint16_t u16;

// 0 - Generate random matrix. 1 - Get values from ethernet 

//#define TEST_TYPE TestType::TYPE_CSR
#define TEST_TYPE TestType::TYPE_GENERATE
//#define TEST_TYPE TestType::TYPE_ETHERNET
//#define TEST_TYPE TestType::TYPE_DIRECTLY

// For some reason static variables cannot have computations because firmware just inits them as zero, need to define them this way to make sure the actual values are properly initialized at beginning of program
#define SIZE 20 // 250
#define AMOUNT_NZ ((SIZE * SIZE) / 2)
#define BLOCK_SIZE (SIZE / 2)
//#define BLOCK_SIZE SIZE
//#define BLOCK_SIZE 16

//#define BLOCK_SIZE 4096

// Max memory size for current accelerator should be 4096 but 8192 seems to work in sim? Maybe because memories output just in time to start getting filled again

// Above 512 nz per block gives error on sim and probably on the board.
// Probably would be best to allocate output array at the last possible position. That way we make sure that no problem from caching should occur

// Default values for random example
static int size = SIZE;
static int amountNZ = AMOUNT_NZ;
static int blockSize = BLOCK_SIZE;

static bool randomMat = true;

#define ALIGN_UP(val,size) (((val) + (size - 1)) & ~(size - 1))

static Byte* PushPageAlign(Arena* arena){
  int toPush = ALIGN_UP(arena->used,4096) - arena->used;

  return (Byte*) PushBytes(arena,toPush);
}

static Byte* PushAlign(Arena* arena,int bytesAlignment){
  int aligned = ALIGN_UP(arena->used,bytesAlignment);
  int toPush = aligned - arena->used;

  return (Byte*) PushBytes(arena,toPush);
}

#ifdef SIM
// Because compilation with .cpp files does not work as the embedded system does not include the necessary files.
// Just copy the implementations for now.
void FlushStdout(){
  fflush(stdout);
}

struct Uint64As8{
  unsigned char array[8];
};

uint64 SwapEndianess(uint64 val){
  Uint64As8* viewIn = (Uint64As8*) &val;
  uint64 res;
  Uint64As8* viewOut = (Uint64As8*) &res;

  for(int i = 0; i < 8; i++){
    viewOut[i] = viewIn[7-i];
  }

  return res;
}

void PrintTime(Time time,const char* id){
  printf("[TimeIt] %15s: ",id);

  char buffer[128];
  {
    int digits = NumberDigitsRepresentation(time.seconds);
    char* secondsRepr = GetNumberRepr(time.seconds,buffer);

    for(int i = 0; i < digits; i++){
      printf("%c",secondsRepr[i]);
    }
  }
  printf(".");
  {
    int digits = NumberDigitsRepresentation(time.microSeconds);
    char* nanoRepr = GetNumberRepr(time.microSeconds,buffer);

    for(int i = 0; i < (6 - digits); i++){
      printf("0");
    }
    
    for(int i = 0; i < digits; i++){
      printf("%c",nanoRepr[i]);
    }
  }

#if 0  
  Time temp = time;
  temp.seconds = SwapEndianess(time.seconds);
  temp.microSeconds = SwapEndianess(time.microSeconds);
  char* hexVal = GetHexadecimal((const char*) &temp,sizeof(Time),buffer); // Helper function to display result
  printf(" ");
  printf(" ");
  printf("%s ",hexVal);
#endif
  
  printf("\n");
}

void TimeIt::Output(){
  // Cannot use floating point because of embedded
  Time end = GetTime();

  Time diff = end - start;
  Assert(end > start);

  PrintTime(diff,id);
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

Time operator-(const Time& s1,const Time& s2){
  Time res = {};

  if(s1.seconds < s2.seconds  || (s1.seconds == s2.seconds && s1.microSeconds < s2.microSeconds)){
    printf("Not allowed negative time\n");
  }
  
  res.seconds = s1.seconds - s2.seconds;

  int s1MicroSeconds = s1.microSeconds;
  if(s1.microSeconds < s2.microSeconds){
    res.seconds -= 1;
    s1MicroSeconds += 1000000;
  }

  res.microSeconds = s1MicroSeconds - s2.microSeconds;
   
  return res;
}

#endif

void PrintMemoryBlock(void* mem,int size);

template<typename T>
int GetMaxDigitSize(Array<T> array){
  int maxReprSize = 0;
  for(u16 val : array){
    maxReprSize = std::max(maxReprSize,NumberDigitsRepresentation((int) val));
  }

  return maxReprSize;
}

template<typename T>
void Print(Array<T> array,int digitSize = 0){
  if(digitSize == 0){
    digitSize = GetMaxDigitSize(array);
  }

  for(int val : array){
    printf("%*d ",digitSize,val);
  }
}

struct Matrix{
  Array<real> values;
  int xSize;
  int ySize;
};

static Matrix RandomMatrix(Arena* out,int size,int nonZeros,int randomSeed){
  SeedRandomNumber(randomSeed);

  int nElements = size * size;

  // Cannot meet the specifications. Return empty
  if(nonZeros > nElements || nonZeros < size){
    printf("Empty matrix\n");
    return (Matrix){};
  }

  Array<real> data = PushArray<real>(out,nElements);
  Memset(data,0.0f);

#ifdef PRINT
  printf("Random mat\n");
#endif
  // Make sure that every column and every row has at least a non zero
  for(int i = 0; i < size; i++){
    data[i * size + i] = RandomNumberBetween(1,10);
  }
  int zerosLeft = nonZeros - size;

  while(zerosLeft){
    int y = RandomNumberBetween(0,size);
    int x = RandomNumberBetween(0,size);

#if 0
    if(zerosLeft % 100 == 0) printf("%d\n",zerosLeft);
#endif

    if(data[y * size + x] == 0){
      data[y * size + x] = (real) RandomNumberBetween(1,10);
      zerosLeft -= 1;
    } else {
      int i = y * size + x;
      for(int ii = 0; ii < nElements; ii++){
        if(data[i] == 0){
          break;
        }
        i = (i + 1) % nElements;
      }
      data[i] = (real) RandomNumberBetween(1,10);
      zerosLeft -= 1;
    }
  }

  Matrix res = {};
  res.values = data;
  res.xSize = size;
  res.ySize = size;

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

static Matrix ExampleMatrix(Arena* arena){
  Array<real> data = PushArray<real>(arena,25);

  size = 5;
  amountNZ = 11;

  Memset(data,0.0f);
  data[0 * 5 + 0] = 1.0;
  data[0 * 5 + 1] = 2.0;
  data[0 * 5 + 3] = 11.0;
  data[1 * 5 + 1] = 3.0;
  data[1 * 5 + 2] = 4.0;
  data[2 * 5 + 2] = 6.0;
  data[2 * 5 + 3] = 7.0;
  data[3 * 5 + 1] = 5.0;
  data[3 * 5 + 3] = 8.0;
  data[4 * 5 + 3] = 9.0;
  data[4 * 5 + 4] = 10.0;

  Matrix res = {};
  res.values = data;
  res.xSize = 5;
  res.ySize = 5;

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

void Identity(Matrix mat){
  Memset(mat.values,0.0f);

  Assert(mat.xSize == mat.ySize);

  for(int i = 0; i < mat.ySize; i++){
    mat.values[i * mat.ySize + i] = 1;
  }
}

void PrintMatrix(Matrix mat){
  int digitSize = GetMaxDigitSize(mat.values);
  for(int y = 0; y < mat.ySize; y++){
    for(int x = 0; x < mat.xSize; x++){
      printf("%*f ",digitSize,mat.values[y * mat.ySize + x]);
    }
    printf("\n");
  }
}

int NonZeros(Array<real> values){
  int count = 0;
  for(real val : values){
    //printf("%f %f %f\n",val,std::abs(val),0.00001f);
    if(std::abs(val) >= 0.00001f) count++;
  }
  return count;
}

Matrix ExtractBlock(Matrix mat,int blockStartX,int blockStartY,int blockSize,Arena* arena){
  int maxY = std::min(blockStartY + blockSize,mat.ySize);
  int maxX = std::min(blockStartX + blockSize,mat.xSize);

  int ySize = maxY - blockStartY;
  int xSize = maxX - blockStartX;

  Array<real> data = PushArray<real>(arena,ySize * xSize);
  //printf("%p\n",data.data);
  Memset(data,0.0f);

  int index = 0;
  for(int y = blockStartY; y < maxY; y++){
    for(int x = blockStartX; x < maxX; x++){
      //printf("%f\n",mat.values[y * mat.xSize + x]);
      data[index++] = mat.values[y * mat.xSize + x];
      //printf("%f\n",data[index-1]);
    }
  }
  //printf("%d\n",index);

  Matrix res = {};
  //printf("%p %f\n",data.data,data[0]);
  res.values = data;
  res.xSize = xSize;
  res.ySize = ySize;

  return res;
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

#define OriginalCSRValue(CSR_PTR) ((real*) (CSR_PTR)->data)
#define OriginalCSRColumn(CSR_PTR) ((u32*) (OriginalCSRValue(CSR_PTR) + (CSR_PTR)->nonZeros))
#define OriginalCSRRow(CSR_PTR) (OriginalCSRColumn(CSR_PTR) + (CSR_PTR)->nonZeros)

struct OriginalCSR{
  int* data;
#ifndef PC
  unsigned int _align; // PC is 8 bytes per pointer. In sim it's 4.
#endif

  unsigned int size;
  unsigned int rowsAmount;
  unsigned int nonZeros;
  
  // Array<real> values;
  // Array<u32> column;
  // Array<u32> row;
};

typedef uint32_t u32;

struct FormatCSR{
  int* data;
#ifndef PC
  u32 _align; // PC is 8 bytes per pointer. In sim it's 4.
#endif
  u16 rowsAmount;
  u16 nonZeros;

#ifndef PC
  u32 _align2;
#endif // PC

  //u16 offsets;
  //u16 _align0;

  // Array<real> values;
  // Array<u16> column;
  // Array<u16> row;
};

#define CSRValue(CSR_PTR) ((real*) (CSR_PTR)->data)
#define CSRColumn(CSR_PTR) ((u16*) (CSRValue(CSR_PTR) + (CSR_PTR)->nonZeros))
#define CSRRow(CSR_PTR) (CSRColumn(CSR_PTR) + (CSR_PTR)->nonZeros)
//#define CSROffsets(CSR_PTR) ((Pair<u16,u16>*) (CSRRow(CSR_PTR) + (CSR_PTR)->rowsAmount))

#ifdef TEMPORARY_MARK
#define CSRValueArray(CSR_PTR) {CSRValue(CSR_PTR),0,(CSR_PTR)->nonZeros}
#define CSRColumnArray(CSR_PTR) {CSRColumn(CSR_PTR),0,(CSR_PTR)->nonZeros}
#define CSRRowArray(CSR_PTR) {CSRRow(CSR_PTR),0,(CSR_PTR)->rowsAmount}
#else
#define CSRValueArray(CSR_PTR) {CSRValue(CSR_PTR),(CSR_PTR)->nonZeros}
#define CSRColumnArray(CSR_PTR) {CSRColumn(CSR_PTR),(CSR_PTR)->nonZeros}
#define CSRRowArray(CSR_PTR) {CSRRow(CSR_PTR),(CSR_PTR)->rowsAmount}
#endif
//#define CSROffsetsArray(CSR_PTR) {CSROffsets(CSR_PTR),(CSR_PTR)->offsets}

int CSRSize(int nonZeros,int rows,int offsets){
  int size = sizeof(float) * nonZeros;
     size += sizeof(u16) * nonZeros;
     size += sizeof(u16) * rows;
     //size += sizeof(Pair<u16,u16>) * offsets;

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
  Array<u16> column = CSRColumnArray(csr);
  Array<u16> row    = CSRRowArray(csr);
  Array<real> values = CSRValueArray(csr);
  //Array<Pair<u16,u16>> offsets = CSROffsetsArray(csr);

#ifndef PC
  printf("%p %d %d\n",csr->data,csr->rowsAmount,csr->nonZeros);
#endif

  printf("CSR: %d %d %d\n",column.size,row.size,values.size);
  //printf("Col: "); Print(column); printf("\n");
  printf("Row: "); Print(row); printf("\n");
  //printf("Val: "); Print(values); printf("\n");

#if 0
  if(offsets.size){
    printf("Off: %d [ ",offsets.size);
    for(Pair<u16,u16>& p : offsets){
      printf("%d:%d ",p.first,p.second);
    }
    printf("]\n");
  }
#endif
}

FormatCOO ConvertCOO(Matrix mat,Arena* arena){
  int nonZero = NonZeros(mat.values);

  FormatCOO res = {};
  res.data = PushArray<int>(arena,nonZero * 3 + 1).data;
  res.nonZeros = nonZero;

  Array<int> column = COOColumnArray(&res);
  Array<int> row    = COORowArray(&res);
  Array<real> values = COOValueArray(&res);

  int index = 0;
  for(int y = 0; y < mat.ySize; y++){
    for(int x = 0; x < mat.xSize; x++){
      real val = mat.values[y * mat.xSize + x];
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

Array<int> Rows(Matrix mat,Arena* arena){
  Array<int> rowCount = PushArray<int>(arena,mat.ySize);
  Memset(rowCount,0);

  for(int y = 0; y < mat.ySize; y++){
    for(int x = 0; x < mat.xSize; x++){
      real val = mat.values[y * mat.xSize + x];

      if(val){
        rowCount[y] += 1;
      }
    }
  }

  return rowCount;
}

int RowsCount(Array<int> rows){
  int count = 0;
  for(int val : rows){
    if(val){
      count += 1;
    }
  }

  return count;
}



int RowsCount(Matrix mat,Arena* arena){
  BLOCK_REGION(arena);

  Array<bool> seenRow = PushArray<bool>(arena,mat.ySize);
  Memset(seenRow,false);

  int count = 0;
  for(int y = 0; y < mat.ySize; y++){
    for(int x = 0; x < mat.xSize; x++){
      real val = mat.values[y * mat.xSize + x];

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

int CalculateOffsets(Matrix mat,Arena* arena){
  BLOCK_REGION(arena);

  Array<int> rows = Rows(mat,arena);

  int index = 0;
  int count = 0;
  while(index < rows.size){
    while(index < rows.size && rows[index] != 0){
      index += 1;
    }

    int start = index;

    while(index < rows.size && rows[index] == 0){
      index += 1;
    }

    int end = index;

    if(index >= rows.size){
      break;
    }

    count += 1;
  }

  return count;
}



Array<Pair<u16,u16>> PushOffsetPair(Matrix mat,int offsets,Arena* arena){
  Assert(offsets > 0);
  Array<Pair<u16,u16>> pairs = PushArray<Pair<u16,u16>>(arena,offsets);

  BLOCK_REGION(arena);

  Array<int> rows = Rows(mat,arena);
  u16 index = 0;

  int count = 0;
  while(index < rows.size){
    while(index < rows.size && rows[index] != 0){
      index += 1;
    }

    int start = index;

    while(index < rows.size && rows[index] == 0){
      index += 1;
    }

    int end = index;

    if(index >= rows.size){
      break;
    }

    pairs[count].first = start;
    pairs[count].second = end;

    count += 1;
  }

  return pairs;
}

FormatCSR ConvertCSR(Matrix mat,Arena* arena,Arena* temp){
  BLOCK_REGION(temp);
  int nonZero = NonZeros(mat.values);
  int rows = RowsCount(mat,temp);
  int offsets = CalculateOffsets(mat,temp);

  PushAlign(arena,4);
  
  FormatCSR res = {};
  res.data = (int*) PushArray<Byte>(arena,CSRSize(nonZero,rows,offsets)).data;
  res.nonZeros = nonZero;
  res.rowsAmount = rows;

  Array<real> values = CSRValueArray(&res);
  Array<u16> column = CSRColumnArray(&res);
  Array<u16> row    = CSRRowArray(&res);

  int index = 0;
  int rowIndex = 0;
  for(int y = 0; y < mat.ySize; y++){
    bool seenFirst = true;
    for(int x = 0; x < mat.xSize; x++){
      real val = mat.values[y * mat.xSize + x];
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

Array<real> Multiply(Matrix mat,Array<real> vector,Arena* arena){
  Array<real> res = PushArray<real>(arena,mat.ySize);
  Memset(res,0.0f);

  for(int y = 0; y < mat.ySize; y++){
    for(int x = 0; x < mat.xSize; x++){
      res[y] += vector[x] * mat.values[y * mat.xSize + x];
      //printf("Got: %f\n",res[y]);
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

Array<real> MultiplyCSR(OriginalCSR csr,Array<real> vector,Arena* arena){
  int size = vector.size;
  Array<real> res = PushArray<real>(arena,size + 2);
  Memset(res,0.0f);

  u32*  column = OriginalCSRColumn(&csr);
  u32*  row    = OriginalCSRRow(&csr);
  real* values = OriginalCSRValue(&csr);

  printf("%x %x %x %d\n",column,row,values,size);

  //int count = 0;
  // MARK
  Time cpuStart = GetTime();
  for(int i = 0; i < size; i++){
    int val = 0;
    int nextRow = row[i+1];
    for(int ii = row[i]; ii < nextRow; ii++){
      //count += 1;
      val += values[ii] * vector[column[ii]];
    }
    res[i] = val;
  }
  Time cpuEnd = GetTime();
  Time diff = cpuEnd - cpuStart;

  //printf("CSR Count: %d\n",count);
  PrintTime(diff,"CPU_CSR");
  
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

#define CACHE_LINE_BITS 256
#define CACHE_LINE_BYTES (CACHE_LINE_BITS / 8)
#define CACHE_N_WAYS 2
#define CACHE_N_LINES 128

#define CACHE_BIT_SIZE (CACHE_N_WAYS * CACHE_N_LINES * CACHE_LINE_BITS * 2) // Multiple by 2 to make sure
#define CACHE_BYTE_SIZE (CACHE_BIT_SIZE / 8)

// Should not be needed. Check if there is no bug elsewhere
void ClearCache(void* validStart){
#if 1
#ifndef PC
  printf("Gonna clear cache\n");
  char* ptr = (char*) validStart;

  char count = 0;
  for(int i = 0; i < CACHE_BYTE_SIZE; i += CACHE_LINE_BYTES){
    count += ptr[i];
  }

  printf("Clear cache count: %d\n",(int) count);
#endif
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
  Array<Pair<u16,u16>> offsets;
  FormatCSR csr;
};

struct MatrixBlock{
  int size;
  int blockSize;
  int numberBlocks;

  // Followed by array of blocks
  //Array<Block> blocks;
  // Followed by array of offsets
};

#define MatrixBlock(MATRIX_PTR) ((Block*) (&(MATRIX_PTR)[1]))
#define MatrixBlockArray(MATRIX_PTR) {MatrixBlock(MATRIX_PTR),(MATRIX_PTR)->numberBlocks}

#define MatrixBlockCSR(MATRIX_PTR) ((BlockCSR*) (&(MATRIX_PTR)[1]))

#ifdef TEMPORARY_MARK
#define MatrixBlockCSRArray(MATRIX_PTR) {MatrixBlockCSR(MATRIX_PTR),0,(MATRIX_PTR)->numberBlocks,0}
#else
#define MatrixBlockCSRArray(MATRIX_PTR) {MatrixBlockCSR(MATRIX_PTR),(MATRIX_PTR)->numberBlocks}
#endif

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
  MatrixBlock* matrix = (MatrixBlock*) base;

  Array<BlockCSR> blocks = MatrixBlockCSRArray(matrix);
  blocks.size = matrix->numberBlocks;

  Pair<u16,u16>* startOfOffsetData = (Pair<u16,u16>*) &blocks.data[blocks.size];
  for(int i = 0; i < blocks.size; i++){
    if(blocks[i].offsets.size){
      blocks[i].offsets.data = startOfOffsetData;
      startOfOffsetData += blocks[i].offsets.size;
    }
  }

  char* startOfCSRData = (char*) startOfOffsetData;
  for(BlockCSR& block : blocks){
    startOfCSRData = (char*) ALIGN_UP(((iptr) startOfCSRData),4);

    block.csr.data = (int*) startOfCSRData;

    int size = CSRSize(block.csr.nonZeros,block.csr.rowsAmount,block.offsets.size);
    startOfCSRData += size;
  }

  return matrix;
}

static const int MAXIMUM_NZ_PER_BLOCK = 500;

MatrixBlock* ConvertMatBlock(Matrix mat,Arena* arena){
#if 0 // If needed, change code to extract block instead of current approach
  int numberOfBlocks = ((mat.xSize / blockSize) + 1) * ((mat.ySize / blockSize) + 1);

  MatrixBlock* matrix = PushStruct<MatrixBlock>(arena);
  matrix->blockSize = blockSize;

  Array<Block> blocks = PushArray<Block>(arena,numberOfBlocks);

  int blockIndex = 0;
  //#ifdef BLOCK_XY
  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
    for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      //#else
      //for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      //  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
      //#endif
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

#ifdef PRINT
      printf("Block %d: %d\n",blockIndex,nonZeros);
#endif

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
#endif
  return nullptr;
}

MatrixBlock* ConvertMatBlockCSR(Matrix mat,Arena* arena,Arena* temp){
  int numberOfBlocks = (((mat.xSize + blockSize - 1) / blockSize)) * (((mat.ySize + blockSize - 1)  / blockSize));

  MatrixBlock* matrix = PushStruct<MatrixBlock>(arena);
  matrix->size = mat.xSize;
  matrix->blockSize = blockSize;

  int nonZeroBlocksCount = 0;
  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
    for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      BLOCK_REGION(temp);

      Matrix blockMat = ExtractBlock(mat,xStart,yStart,blockSize,temp);
      int nonZeros = NonZeros(blockMat.values);

      if(nonZeros == 0){ // Do not store info for empty blocks.
        continue;
      }

      nonZeroBlocksCount += 1;
    }
  }
  
  Array<BlockCSR> blocks = PushArray<BlockCSR>(arena,nonZeroBlocksCount);

  int blockIndex = 0;
  //#ifdef BLOCK_XY
  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
    for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      //#else
      //for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      //  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
      //#endif

#ifdef PRINT
      printf("%d\n",blockIndex);
#endif

      BLOCK_REGION(temp);

      Matrix blockMat = ExtractBlock(mat,xStart,yStart,blockSize,temp);
      //printf("%f\n",blockMat.values[0]);
      int nonZeros = NonZeros(blockMat.values);

      //printf("%d\n",nonZeros);

      if(nonZeros == 0){ // Do not store info for empty blocks.
        continue;
      }

      Array<int> rowsCount = Rows(blockMat,temp);
      int rows = RowsCount(rowsCount);
      int offsets = CalculateOffsets(blockMat,temp);

      if(offsets){
        blocks[blockIndex].offsets = PushOffsetPair(blockMat,offsets,arena);
      } else {
        blocks[blockIndex].offsets = {0,0};
      }
      blockIndex += 1;
    }
  }

  blockIndex = 0;
  //#ifdef BLOCK_XY
  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
    for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      //#else
      //for(int yStart = 0; yStart < mat.ySize; yStart += blockSize){
      //  for(int xStart = 0; xStart < mat.xSize; xStart += blockSize){
      //#endif

#ifdef PRINT
      printf("%d\n",blockIndex);
#endif

      BLOCK_REGION(temp);

      Matrix blockMat = ExtractBlock(mat,xStart,yStart,blockSize,temp);
      //printf("%f\n",blockMat.values[0]);
      int nonZeros = NonZeros(blockMat.values);

      //printf("%d\n",nonZeros);

      if(nonZeros == 0){ // Do not store info for empty blocks.
        continue;
      }

#ifdef PRINT
      printf("Block %d: %d\n",blockIndex,nonZeros);
#endif

#if 0
      Array<int> rowsCount = Rows(blockMat,blockSize,temp);
      int rows = RowsCount(rowsCount);
      int offsets = CalculateOffsets(blockMat,blockSize,temp);
#endif

      FormatCSR csr = ConvertCSR(blockMat,arena,temp);

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

  //int count = 0;
  timeRegion("CPU multiply"){
  for(BlockCSR& b : blocks){
    FormatCSR csr = b.csr;

    Array<real> values = CSRValueArray(&csr);
    Array<u16> column = CSRColumnArray(&csr);
    Array<u16> row    = CSRRowArray(&csr);
    Array<Pair<u16,u16>> offsets = b.offsets;

    int yOffset = b.y * block->blockSize;
    int xOffset = b.x * block->blockSize;
    int offset = 0;
    int start = 0;
    int pairIndex = 0;
    for(int i = 0; i < csr.rowsAmount; i++){
      if(pairIndex < offsets.size){
        if(offsets[pairIndex].first == offset){
          offset = offsets[pairIndex].second;
          pairIndex += 1;
        }
      }

      float val = 0;
      for(int ii = start; ii < row[i]; ii++){
        //count += 1;
        float sum = values[ii] * vector[xOffset + column[ii]];
        val += sum;
#ifdef HEAVY_PRINT
        printf("%d %d %2f*%2f=%2f\n",yOffset + offset,xOffset + column[ii],values[ii],vector[xOffset + column[ii]],sum);
#endif
      }

      res[yOffset + offset] += val;
      start = row[i];
      offset += 1;
    }
  }
}

  //printf("Block count: %d\n",count);

  //uart_finish();
  //exit(0);
  
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

void PrintSimpleCSR(MatrixBlock* block){
  Array<BlockCSR> blocks = MatrixBlockCSRArray(block);

  printf("Size: %d BlockSize: %d NBlocks: %d\n",block->size,block->blockSize,block->numberBlocks);

  for(BlockCSR& b : blocks){
    printf("%d %d %d",b.y,b.x,b.csr.nonZeros);

    printf("\n");
  }
}

void PrintCSR(MatrixBlock* block){
  printf("%d %d %d\n",block->size,block->blockSize,block->numberBlocks);
  Array<BlockCSR> blocks = MatrixBlockCSRArray(block);

  printf("%d %d %d\n",block->size,block->blockSize,block->numberBlocks);
  printf("Blocks: %d\n",blocks.size);

  for(BlockCSR& b : blocks){
    printf("%d %d ",b.y,b.x);
    Print(&b.csr);
    if(b.offsets.size){
      printf("Off: %d [ ",b.offsets.size);
      for(Pair<u16,u16>& p : b.offsets){
        printf("%d:%d ",p.first,p.second);
      }
      printf("]\n");
    }

    printf("\n");
  }
}

Array<real> vec;
Matrix mat;
FormatCSR csr;
FormatCOO coo;
MatrixBlock* block;

enum Type {COO,CSR,BLOCK,BLOCKCSR};

void PrintMemoryBlock(void* mem,int size){
  unsigned char* view = (unsigned char*) mem;
  for(int i = 0; i < size; i++){
    unsigned int ch = (int) view[i];

    printf(",0x%02x",ch & 0x000000ff);
    if((i + 1) % 16 == 0){
      printf("\n");
    }
  }
  printf("\n");
}

enum TestType {
  TYPE_GENERATE = 0,
  TYPE_ETHERNET = 1,
  TYPE_DIRECTLY = 2,
  TYPE_CSR = 3
};

void InitializeSMVM(Arena* arena,Type type){
  TestType test = TEST_TYPE;

  Arena tempInst = SubArena(arena,Megabyte(1));
  Arena* temp = &tempInst;
 
  switch(test){
  case TestType::TYPE_CSR:{
    eth_init(ETHERNET_BASE);

    char* buffer = (char*) PushBytes(arena,Megabyte(256));
    buffer += Megabyte(1); // Make sure that we are in memory never touched upon;

    printf("Testing CSR\n");
    printf("Waiting for file receive\n");
    int fileSize = eth_rcv_variable_file(buffer);
    printf("Received: %d\n",fileSize);

    OriginalCSR* csr = (OriginalCSR*) buffer;
    csr->data = (int*) &csr[1];
    
    printf("%x %d %d %d\n",csr->data,csr->size,csr->rowsAmount,csr->nonZeros);

    size = csr->size;

    vec = PushArray<real>(arena,size);
    for(int i = 0; i < size; i++){
      vec[i] = RandomNumberBetween(1,size);
    }

    Array<real> expected = MultiplyCSR(*csr,vec,arena);
    uart_finish();
    exit(0);
  } break;
  case TestType::TYPE_GENERATE:{
#ifdef PRINT
    printf("Size: %d,NZ: %d,BlockSize:%d \n",size,amountNZ,blockSize);
#endif
    
    if(randomMat){
      mat = RandomMatrix(arena,size,amountNZ,1);
    } else {
      mat = ExampleMatrix(arena);
    }
    PushPageAlign(arena);

#ifdef PRINT
    printf("Generated mat\n");
#endif
    
    // The previous code, ExampleMatrix can change size. Take care
    vec = PushArray<real>(arena,size);
    for(int i = 0; i < size; i++){
      vec[i] = (real) RandomNumberBetween(1,size);
    }

    PushPageAlign(arena);
    //PushBytes(arena,Kilobyte(4) - 4);

#ifdef HEAVY_PRINT
    printf("Vec:\n");
    Print(vec);
    printf("\n");
#endif // HEAVY_PRINT

#ifdef HEAVY_PRINT
    PrintMatrix(mat);
#endif // HEAVY_PRINT

    //printf("%f\n",mat.values[mat.values.size-1]);

    PushPageAlign(arena);
    switch(type){
    case Type::COO:{
      coo = ConvertCOO(mat,arena);
    }break;
    case Type::CSR:{
      csr = ConvertCSR(mat,arena,temp);
    }break;
    case Type::BLOCK:{
      block = ConvertMatBlock(mat,arena);
#ifdef BLOCK_PRINT
      Print(block);
#endif
    }break;
    case Type::BLOCKCSR:{
      Byte* start = (Byte*) PushBytes(arena,0);

#ifdef PRINT
      printf("Start: %d %x\n",start,start);
#endif
      block = ConvertMatBlockCSR(mat,arena,temp);

      Byte* end = (Byte*) PushBytes(arena,0);

#ifdef PRINT
      printf("Total size: %d\n",end - start);

      printf("Start: %x\n",start);
      printf("End: %x\n",end);
      //PrintMemoryBlock(start,end-start);
#endif
      
#ifdef HEAVY_PRINT
      PrintMemoryBlock(start,end-start);
#endif
#ifdef BLOCK_PRINT
      PrintCSR(block);
#endif
#ifdef BLOCK_SIMPLE_PRINT
      PrintSimpleCSR(block);
#endif
    }break;
    }
    PushPageAlign(arena);

    region(arena){
      Array<real> expected = Multiply(mat,vec,arena);
      PushExpected(expected);
    }
  }break;
  case TestType::TYPE_ETHERNET:{
    eth_init(ETHERNET_BASE);

    char* buffer = (char*) PushBytes(arena,Megabyte(256));
    buffer += Megabyte(1); // Make sure that we are in memory never touched upon;

    printf("Waiting for file receive\n");
    int fileSize = eth_rcv_variable_file(buffer);
    printf("Received: %d\n",fileSize);

    block = UnpackMatrixBlockCSR(buffer);
    printf("B: %p\n",block);
    printf("%d %d %d\n",block->size,block->blockSize,block->numberBlocks);

    printf("Start: %x\n",buffer);
    printf("End: %x\n",&buffer[fileSize]);
    
#ifdef HEAVY_PRINT
    PrintMemoryBlock(buffer,fileSize);
#endif // HEAVY_PRINT

#ifdef BLOCK_PRINT
    printf("%d %d %d\n",block->size,block->blockSize,block->numberBlocks);
    PrintCSR(block);
#endif // HEAVY_PRINT

#ifdef BLOCK_SIMPLE_PRINT
    PrintSimpleCSR(block);
#endif
    
    size = block->size;
    blockSize = block->blockSize; 
    
    vec = PushArray<real>(arena,size);
    for(int i = 0; i < size; i++){
      vec[i] = RandomNumberBetween(1,size);
    }
    
    Array<real> expected = MultiplyBlockCSR(block,vec,arena);
    PushExpected(expected);
  }break;
  case TestType::TYPE_DIRECTLY:{
#if 0
    printf("Using data directly\n");
    
    InitializeGotFloats(arena,size);

    if(startData < (int) &arena->mem[arena->used]){
      printf("[ERROR] Arena is overallocated compared to expected\n");
    }
       
    arena->used = (startData - (int) arena->mem);
    
    int dataSize = ARRAY_SIZE(testData);
    Byte* memory = PushBytes(arena,dataSize);

    printf("Start: 0x%p\n",memory);
    printf("End: 0x%p 0x%p\n",(int) &memory[dataSize],endData);

    int* intView = (int*) memory;
    int* testDataView = (int*) testData;
    for(int i = 0; i < (dataSize / sizeof(int)) + 1; i++){
      if(i % Megabyte(1) == 0){
        printf("%d\n",i);
      }
      intView[i] = testDataView[i];
    }

    block = (MatrixBlock*) memory;
#endif    
    // Do not know if should do a memory align ???
  }break;
  }

#ifdef PRINT
  printf("Finished SMVM init\n");
#endif
}

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
