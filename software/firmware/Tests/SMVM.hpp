#include "testbench.h"

#include "memory.hpp"
#include "utils.hpp"

static Array<int> RandomMatrix(Arena* out,int seed,int size,int nonZeros,int randomSeed){
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

      if(res[y * size + x] != 0){
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
   int size;
   Array<int> row;
   Array<int> column;
   Array<int> values;
};

FormatCOO ConvertCOO(Array<int> matrix,int size,Arena* arena){
   int nonZero = NonZero(matrix);

   FormatCOO res = {};
   res.row = PushArray<int>(arena,nonZero);
   res.column = PushArray<int>(arena,nonZero);
   res.values = PushArray<int>(arena,nonZero);
   res.size = size;
   
   int index = 0;
   for(int y = 0; y < size; y++){
      for(int x = 0; x < size; x++){
         int val = matrix[y * size + x];
         if(val){
            res.row[index] = y;
            res.column[index] = x;
            res.values[index] = val;
            index += 1;
         }
      }
   }
   Assert(index == nonZero);

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
   int size = coo.size;
   Array<int> res = PushArray<int>(arena,size);
   Memset(res,0);

   for(int i = 0; i < coo.values.size; i++){
      res[coo.row[i]] += coo.values[i] * vector[coo.column[i]];
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

   res.column = PushArray<int>(temp,amountOfNZ);
   res.row = PushArray<int>(temp,amountOfNZ);
   res.values = PushArray<int>(temp,amountOfNZ);

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
            res.row[index] = y;
            res.column[index] = x;
            res.values[index] = val;
            index += 1;
         }
      }
   }

   return res;
}

int size = 5;
int amountNZ = 20;
int vecData[] = {1,2,3,4,5};
Array<int> vec = C_ARRAY_TO_ARRAY(vecData);

