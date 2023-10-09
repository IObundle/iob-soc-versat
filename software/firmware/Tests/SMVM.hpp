#ifdef TEMPORARY_MARK
template<typename T>
class ArrayIterator{
public:
   T* ptr;

   inline bool operator!=(const ArrayIterator<T>& iter){return ptr != iter.ptr;};
   inline ArrayIterator<T>& operator++(){++ptr; return *this;};
   inline T& operator*(){return *ptr;};
};

template<typename T>
struct Array{
  T* data;
  int _align;
  int size;
  int _align2;

  inline T& operator[](int index) const {return data[index];}
  ArrayIterator<T> begin(){return ArrayIterator<T>{data};};
  ArrayIterator<T> end(){return ArrayIterator<T>{data + size};};
};
#endif

#include "testbench.h"

#include "memory.hpp"
#include "utils.hpp"

typedef float real;

#include <cstdint>
typedef uint16_t u16;

// 0 - Generate random matrix. 1 - Get values from ethernet 

//#define TEST_TYPE TestType::TYPE_CSR
#define TEST_TYPE TestType::TYPE_GENERATE
//#define TEST_TYPE TestType::TYPE_ETHERNET
//#define TEST_TYPE TestType::TYPE_DIRECTLY

// For some reason static variables cannot have computations because firmware just inits them as zero, need to define them this way to make sure the actual values are properly initialized at beginning of program
#define SIZE 4 // 250
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

static Byte* PushPageAlign(Arena* arena){
  int toPush = ALIGN_UP(arena->used,4096) - arena->used;

  return PushBytes(arena,toPush);
}

static Byte* PushAlign(Arena* arena,int bytesAlignment){
  int aligned = ALIGN_UP(arena->used,bytesAlignment);
  int toPush = aligned - arena->used;

  return PushBytes(arena,toPush);
}

#ifdef SIM
// Because compilation with .cpp files does not work as the embedded system does not include the necessary files.
// Just copy the implementations for now.
void FlushStdout(){
  fflush(stdout);
}

static char* GetNumberRepr(uint64 number,char* buffer){
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

char GetHexadecimalChar(int value){
  if(value < 10){
    return '0' + value;
  } else{
    return 'a' + (value - 10);
  }
}

char* GetHexadecimal(const char* text, int str_size,char* buffer){
  int i;

  Assert(str_size * 2 < 64);
  for(i = 0; i < str_size; i++){
    int ch = (int) ((unsigned char) text[i]);

    buffer[64 - 3 - i*2] = GetHexadecimalChar(ch / 16);
    buffer[64 - 3 - i*2+1] = GetHexadecimalChar(ch % 16);
  }

  buffer[64 - 1] = '\0';

  return &buffer[64 - str_size * 2 - 1];
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

int GetMaxDigitSize(Array<u16> array){
  int maxReprSize = 0;
  for(u16 val : array){
    maxReprSize = std::max(maxReprSize,NumberDigitsRepresentation((int) val));
  }

  return maxReprSize;
}

void Print(Array<u16> array,int digitSize = 0){
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

  printf("Random mat\n");
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

static real* expectedFloats;
static real* gotFloats;
static bool errorFloats = false;

void PushExpected(Array<real> vec){
  if(vec.size != size){
    errorFloats = true;
  }

  for(int i = 0; i < vec.size; i++){
    expectedFloats[i] = vec[i];
  }
}

void PushGot(Array<real> vec){
  if(vec.size != size){
    errorFloats = true;
  }
  printf("%p\n",gotFloats);
  for(int i = 0; i < vec.size; i++){
    gotFloats[i] = vec[i];
  }
}

#if 0
void PushExpected(Array<int> vec){
  for(int val : vec){
    PushExpectedI(val);
  }
}
#endif

#if 0
void PushGot(Array<int> vec){
  for(int val : vec){
    PushGotI(val);
  }
}
#endif

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
  unsigned char* view = (Byte*) mem;
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

void InitializeGotFloats(Arena* arena,int size){
  PushPageAlign(arena);
  expectedFloats = PushArray<real>(arena,size+1).data;
  PushPageAlign(arena);
  gotFloats = PushArray<real>(arena,size+1).data;
  PushPageAlign(arena);
}

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
    printf("Size: %d,NZ: %d,BlockSize:%d \n",size,amountNZ,blockSize);

    InitializeGotFloats(arena,size);
    
    if(randomMat){
      mat = RandomMatrix(arena,size,amountNZ,1);
    } else {
      mat = ExampleMatrix(arena);
    }
    PushPageAlign(arena);

    printf("Generated mat\n");

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
      Byte* start = PushBytes(arena,0);

      printf("Start: %d %x\n",start,start);
      block = ConvertMatBlockCSR(mat,arena,temp);

      Byte* end = PushBytes(arena,0);

      printf("Total size: %d\n",end - start);

      printf("Start: %x\n",start);
      printf("End: %x\n",end);
      //PrintMemoryBlock(start,end-start);

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
    
    InitializeGotFloats(arena,block->size);
    
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

  printf("Finished SMVM init\n");
}
