
#if 0
#include "testbench.hpp"
#include "unitConfiguration.hpp"

void SingleTest(Arena* arena){
    VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;
    VectorLikeOperationConfig* vec = (VectorLikeOperationConfig*) accelConfig;

    int testRow[8] = {0x19,0x2a,0x3b,0x4c,0x5d,0x6e,0x7f,0x80};
    int testMem[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};

    ConfigureSimpleVRead(&vec->row,8,testRow);

    int size = 8;

    vec->mat.iterA = 1;
    vec->mat.incrA = 1;
    vec->mat.iterB = 1;
    vec->mat.incrB = 1;
    vec->mat.perA = size;
    vec->mat.dutyA = size;
    vec->mat.perB = size;
    vec->mat.dutyB = size;

    vec->mask.constant = 0x0f;

    //ConfigureMemoryReceive(&vec->mat, 8);
    VersatMemoryCopy(addr.mat.addr,testMem,8 * sizeof(int));

    //ConfigureMemoryReceive(&vec->output,8);

    int result[8] = {};
    vec->mat.in0_wr = 0;
    RunAccelerator(1);
    vec->mat.in0_wr = 1;
    RunAccelerator(1);

    for(int i = 0; i < 8; i++){
        result[i] = VersatUnitRead((iptr) addr.mat.addr,i);
        Assert_Eq(0x08,result[i]);
    }
}

#endif

#if 1

#include "versat_accel.h"
#include "unitConfiguration.hpp"


extern "C"{
#include <string.h>
#include <stdlib.h>

#include "controlbits.h"
#include "benes.h"
#include "crypto_declassify.h"
#include "crypto_uint64.h"
#include "params.h"
#include "pk_gen.h"
#include "root.h"
#include "uint64_sort.h"
#include "util.h"

#include "iob-uart.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "crypto_declassify.h"
#include "crypto_uint64.h"
#include "uint64_sort.h"

#include "printf.h"
#include "arena.h"
}

#if 0
module VectorLikeOperation(){
   Mem mat; // mat[row]
   VRead row; // mat[k]
   Mem output; 
   Const mask;
#
   a = row & mask;
   b = mat ^ a;

   b -> output;
}
#endif

#define REAL_VERSAT
//#define SIMULATED_VERSAT

//#define REAL_VERSAT2
#define SIMULATED_VERSAT2

//#define PRINT
//#define LIMIT

#define ALIGN_4(VAL) (((VAL) + 3) & (~3))
#define ALIGN_16(VAL) (((VAL) + 15) & (~15))

#define ASYS ALIGN_16(SYS_N / 8) // 436 -> 448
#define SBYTE ALIGN_4(SYS_N / 8) // 436 -> 448
//#define SBYTE ALIGN_16(SYS_N / 8) // 436 -> 448
#define SINT (ASYS / 4) // 109 -> 112

extern "C" void InitMcEliece(int versatBase){
   versat_init(versatBase);
   ConfigCreateVCD(true);

   VectorLikeOperationConfig* vec = (VectorLikeOperationConfig*) accelConfig;
   ConfigureSimpleVReadBare(&vec->row);

   vec->mat.iterA = 1;
   vec->mat.incrA = 1;
   vec->mat.iterB = 1;
   vec->mat.incrB = 1;
   vec->mat.perA = SINT + 1;
   vec->mat.dutyA = SINT + 1;
   vec->mat.perB = SINT + 1;
   vec->mat.dutyB = SINT + 1;

   //ConfigureSimpleVReadBare(&vec->mat);
}

void PrintRow(uint8_t* view){
    for(int i = 0; i < SBYTE; i++){
        if(i != 0 && i % 16 == 0){
            printf("\n");
        } else {
            printf("%x ",view[i]);
        }
    }
    printf("\n\n");
}

void VersatPrintRow(){
    uint32_t values[SINT];
    VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;

    int size = SINT;
    for(int i = 0; i < size; i++){
        values[i] = VersatUnitRead((iptr) addr.mat.addr,i);
    }
    PrintRow((uint8_t*) values);
}

void VersatLoadRow(uint32_t* mat){
    VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;

    VersatMemoryCopy(addr.mat.addr,(int*) mat,SINT * sizeof(int));
}

void ReadRow(uint32_t* row){
    VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;
    for (int i = 0; i < SINT; i++){
        row[i] = VersatUnitRead((iptr) addr.mat.addr,i);
    }
}

uint8_t* GetVersatRowForTests(){  
    static uint8_t buffer[ASYS];
    ReadRow((uint32_t*) buffer);
    return buffer;
}

extern "C" void VersatOnly(uint8_t *row, uint8_t mask,bool first){
    static uint8_t savedMask = 0;
    uint32_t *row_int = (uint32_t*) row;

    static int times = 0;
    if(times % 10000 == 0) printf("V:%d\n",times);
    times++;

#ifdef LIMIT
   if(times >= 20000){
    uart_finish();
    exit(1);
   }
#endif

    int start = GetAcceleratorCyclesElapsed();

    VectorLikeOperationConfig* vec = (VectorLikeOperationConfig*) accelConfig;
    VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;

    if(first){
        ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);
        vec->mat.in0_wr = 0;
        RunAccelerator(1); // Load VRead
    } else {
        ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);

        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);
        vec->mask.constant = mask_int;
        vec->mat.in0_wr = 1;
        RunAccelerator(1); // Run with correct data.
    }

    savedMask = mask;
    int end = GetAcceleratorCyclesElapsed();
}

static crypto_uint64 uint64_is_equal_declassify(uint64_t t, uint64_t u) {
    crypto_uint64 mask = crypto_uint64_equal_mask(t, u);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

static crypto_uint64 uint64_is_zero_declassify(uint64_t t) {
    crypto_uint64 mask = crypto_uint64_zero_mask(t);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

void VersatPart2(unsigned char* mat,int index,int k,int row,uint8_t mask);

// Matrix Allocate
#define MA(Y,X,TYPE) (TYPE*) PushBytes(sizeof(TYPE) * (X) * (Y))
// Matrix Index

#define MI(Y,X,ROWSIZE) ((Y) * (ROWSIZE) + (X)) 
#define MR(Y,ROWSIZE) ((Y) * (ROWSIZE))
//#define MI(Y,X,ROWSIZE) Y][X 

extern "C" {
    extern int secondTest;
}

/* input: secret key sk */
/* output: public key pk */
int pk_gen(unsigned char *pk, unsigned char *sk, const uint32_t *perm, int16_t *pi) {
    int i, j, k;
    int row, c;

#if 0  
    static int times = 0;
    printf("%d\n",times++);
#endif
  
    int mark = MarkArena();
  
    //uint64_t buf[ 1 << GFBITS ];
    uint64_t* buf = PushArray(1 << GFBITS,uint64_t);
  
    //unsigned char mat[ PK_NROWS ][ SBYTE ];
    unsigned char* mat = MA(PK_NROWS,ASYS,unsigned char);

    unsigned char mask;
    unsigned char b;

#if  0
    gf g[ SYS_T + 1 ]; // Goppa polynomial
    gf L[ SYS_N ]; // support
    gf inv[ SYS_N ];
#endif

    gf* g = PushArray(SYS_T + 1,gf);
    gf* L = PushArray(SYS_N,gf); // support
    gf* inv = PushArray(SYS_N,gf);
  
    //

    g[ SYS_T ] = 1;

    for (i = 0; i < SYS_T; i++) {
        g[i] = load_gf(sk);
        sk += 2;
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        buf[i] = perm[i];
        buf[i] <<= 31;
        buf[i] |= i;
    }

    uint64_sort(buf, 1 << GFBITS);

    for (i = 1; i < (1 << GFBITS); i++) {
        if (uint64_is_equal_declassify(buf[i - 1] >> 31, buf[i] >> 31)) {
          PopArena(mark);
           return -1;
        }
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        pi[i] = buf[i] & GFMASK;
    }
    for (i = 0; i < SYS_N;         i++) {
        L[i] = bitrev(pi[i]);
    }

    // filling the matrix

    root(inv, g, L);

    for (i = 0; i < SYS_N; i++) {
        inv[i] = gf_inv(inv[i]);
    }

    for (i = 0; i < PK_NROWS; i++) {
        for (j = 0; j < SBYTE; j++) {
          mat[MI(i , j ,ASYS)] = 0;
        }
    }

    for (i = 0; i < SYS_T; i++) {
        for (j = 0; j < SYS_N; j += 8) {
            for (k = 0; k < GFBITS;  k++) {
                b  = (inv[j + 7] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 6] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 5] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 4] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 3] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 2] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 1] >> k) & 1;
                b <<= 1;
                b |= (inv[j + 0] >> k) & 1;

                mat[MI( i * GFBITS + k, j / 8,ASYS)] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = gf_mul(inv[j], L[j]);
        }

    }

    //printf("PK_NROWS:%d  SYS_N:%d\n",PK_NROWS,SYS_N);
    //PK_NROWS:768  SYS_N:3488 (SBYTE):436 ((SBYTE) / 4):109

    if(secondTest){
        printf("Second test active\n");
    }

#if 1
    // This entire thing needs to be speedup.
    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            // For each row
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

            VectorLikeOperationConfig* vec = (VectorLikeOperationConfig*) accelConfig;
            VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;
            uint32_t *out_int = (uint32_t*) &(mat[MR(row,ASYS)]);

#ifdef REAL_VERSAT
            VersatLoadRow((uint32_t*) &mat[MR(row,ASYS)]);
#endif

            bool first = true;
            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[MI(row,i,ASYS)] ^ mat[MI( k , i ,ASYS)];
                mask >>= j;
                mask &= 1;
                mask = -mask;

#ifdef SIMULATED_VERSAT
                uint32_t* mat_int = (uint32_t*) &mat[MR(row,ASYS)];
                uint32_t* out_int = (uint32_t*) &mat[MR(row,ASYS)];
                uint32_t* row_int = (uint32_t*) &mat[MR(k,ASYS)];

                uint32_t mask_int = (mask) | (mask << 8) | (mask << 8*2) | (mask << 8*3);
                for(int i = 0; i < SINT; i++){
                    uint32_t a = row_int[i] & mask_int;
                    uint32_t b = mat_int[i] ^ a;
                    out_int[i] = b;
                }

                if(secondTest) {
                    printf("Mask: %d\n",mask ? 1 : 0);
                    printf("Software after\n");
                    PrintRow(&mat[MR(row,ASYS)]);
                }
#endif

#ifdef REAL_VERSAT
                VersatOnly(&(mat[MR(k,ASYS)]),mask,first);
                mat[MI(row,i,ASYS)] ^= mat[MI(k,i,ASYS)] & mask;
#endif

                first = false;
            }

            // Last run, use 
            VersatOnly(&(mat[MR(k,ASYS)]),0,false);  

#ifdef REAL_VERSAT
            // TODO: We do not need to do this because we only need MI(row,i,ASYS) (which we already have)
            //       and because the next loop needs row which we already have 
            ReadRow(out_int);
#endif

            if ( uint64_is_zero_declassify((mat[MI(row,i,ASYS) ] >> j) & 1) ) { // return if not systematic
                PopArena(mark);
                return -1;
            }

            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[MI(k,i,ASYS)] >> j;
                    mask &= 1;
                    mask = -mask;

                    for (c = 0; c < SYS_N / 8; c++) {
                        mat[MI(k,c,ASYS)] ^= mat[MI(row,c,ASYS)] & mask;
                    }
                }
            }

/*
#if defined(SIMULATED_VERSAT) && defined(REAL_VERSAT2) 
            VersatLoadRow((uint32_t*) &mat[MR(row,ASYS)]);
#endif


            int index = 0;
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[MI(k,i,ASYS)] >> j;
                    mask &= 1;
                    mask = -mask;

                    // Mat k changes per loop.
                    // The constant is mat row.
                    // So we need a VRead that reads k + 2;
                    // while we process k + 1. MI Row is stored inside Versat at this point
                    // and VWrite that writes k;

#ifdef SIMULATED_VERSAT2
                    for (c = 0; c < SBYTE; c++) {
                        mat[MI(k,c,ASYS)] ^= mat[MI(row,c,ASYS)] & mask;
                    }
#endif
#ifdef REAL_VERSAT2
                    VersatPart2(mat,index,k,row,mask);
                    index += 1;
#endif

                }
            }

#ifdef REAL_VERSAT2
            VersatPart2(mat,index++,PK_NROWS,row,0);
            VersatPart2(mat,index++,PK_NROWS + 1,row,0);

            //vec->writer.enableWrite = 0;
#endif

#if 0
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    printf("Rows %d %d %d %d %d\n",k,row,j,i,mask);
                    PrintRow(&mat[MR(k,ASYS)]);
                }
            }
#endif

#if 0
            uart_finish();
            exit(0);
#endif

            */
        }
    }
#endif

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, &(mat[i*ASYS]) + PK_NROWS / 8, PK_ROW_BYTES);
    }

    PopArena(mark);
    return 0;
}

#endif

/*
void VersatPart2(unsigned char* mat,int timesCalled,int k,int row,uint8_t mask){
    static uint32_t savedMask = 0;

    VectorLikeOperationConfig* vec = (VectorLikeOperationConfig*) accelConfig;
    VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;

    uint32_t mask_int = (mask) | (mask << 8) | (mask << 8*2) | (mask << 8*3);

    static int times = 0;
    times += 1;
#ifdef LIMIT
   if(times >= 10000){
    uart_finish();
    exit(1);
   }
#endif

    if(k == row){
        printf("This function is not supposed to be called when K == Row\n");
        uart_finish();
        exit(0);
    }

    // PK_NROWS - 768

    int toRead =    k;
    int toCompute = ((toRead - 1    == row) ? toRead - 2    : toRead - 1);
    int toWrite =   ((toCompute - 1 == row) ? toCompute - 2 : toCompute - 1);

    if(toRead < PK_NROWS){
        uint32_t *toRead_int = (uint32_t*) &mat[MR(toRead,ASYS)];

        vec->mat.in0_wr = 0;
        ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) toRead_int);        
    } else {
        //vec->row.enableRead = 0;
        toRead = -9;
    }
    if(timesCalled >= 1 && toCompute >= 0 && toCompute < PK_NROWS){
        vec->mask.constant = savedMask;
    } else {
        toCompute = -9;
        // Disable compute
    }
    if(timesCalled >= 2 && toWrite >= 0){
        uint32_t *toWrite_int = (uint32_t*) &mat[MR(toWrite,ASYS)];

        ConfigureSimpleVWrite(&vec->writer, SINT, (int*) toWrite_int);
    } else {
        toWrite = -9;
        // Disable write
        //vec->writer.enableWrite = 0;
    }

    printf("K %d,Row %d,toRead %d,toCompute %d,toWrite %d\n",k,row,toRead,toCompute,toWrite);

    RunAccelerator(1);

    savedMask = mask_int;
}

*/
