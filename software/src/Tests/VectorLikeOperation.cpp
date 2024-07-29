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

// Matrix Allocate
#define MA(Y,X,TYPE) (TYPE*) PushBytes(sizeof(TYPE) * (X) * (Y))
// Matrix Index

#define MI(Y,X,ROWSIZE) ((Y) * (ROWSIZE) + (X)) 
#define MR(Y,ROWSIZE) ((Y) * (ROWSIZE))
//#define MI(Y,X,ROWSIZE) Y][X 

#define REAL_VERSAT
#define REAL_VERSAT2

#define ALIGN_4(VAL) (((VAL) + 3) & (~3))
#define ALIGN_16(VAL) (((VAL) + 15) & (~15))

// Need to align
#define ASYS ALIGN_16(SYS_N / 8) // 436 -> 448
#define SBYTE ALIGN_4(SYS_N / 8) // 436 -> 448
#define SINT (ASYS / 4) // 109 -> 112

static VectorLikeOperationConfig* vec;
static VectorLikeOperationAddr addr;

extern "C" void InitMcEliece(int versatBase){
    versat_init(versatBase);

    vec = (VectorLikeOperationConfig*) accelConfig;
    addr = ACCELERATOR_TOP_ADDR_INIT;
}

void PrintRow(uint8_t* view){
    for(int i = 0; i < 16; i++){
        printf("%02x ",view[i]);
    }
    printf("\n");
    for(int i = SBYTE - 16; i < SBYTE; i++){
        printf("%02x ",view[i]);
    }
    printf("\n\n");
}

void VersatLoadRow(uint32_t* mat){
    VersatMemoryCopy(addr.mat.addr,(int*) mat,SINT * sizeof(int));
}

void ReadRow(uint32_t* row){
    for (int i = 0; i < SINT; i++){
        row[i] = VersatUnitRead((iptr) addr.mat.addr,i);
    }
}

void VersatPrintRow(){
    uint32_t values[SINT];
    ReadRow(values);
    PrintRow((uint8_t*) values);
}

uint8_t* GetVersatRowForTests(){  
    static uint8_t buffer[ASYS];
    ReadRow((uint32_t*) buffer);
    return buffer;
}

void PrintFullMat(uint8_t* mat){
    printf("Printing full mat:\n");
    for(int i = 0; i < PK_NROWS; i++){
        PrintRow(&mat[MR(i,ASYS)]);
    }
}

void VersatMcElieceLoop1(uint8_t *row, uint8_t mask,bool first){
    static uint8_t savedMask = 0;
    uint32_t *row_int = (uint32_t*) row;

    if(first){
        ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);
        vec->mat.in0_wr = 0;
    } else {
        ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) row_int);

        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);
        vec->mask.constant = mask_int;
        vec->mat.in0_wr = 1;
    }

    // Allow reconfiguration while accelerator is running.

#if 1
    // Note: Because of pc-emul and sim-run differences, this might not actually work.
    //       If troubles start appearing on the board, change this.
    EndAccelerator();
    StartAccelerator();
#endif

    savedMask = mask;
}

// Can only be called if k != row. Care
void VersatMcElieceLoop2(unsigned char* mat,int timesCalled,int k,int row,uint8_t mask){
    static uint8_t savedMask = 0;

    int toRead =    k;
    int toCompute = ((toRead - 1    == row) ? toRead - 2    : toRead - 1);
    int toWrite =   ((toCompute - 1 == row) ? toCompute - 2 : toCompute - 1);

    if(toRead < PK_NROWS){
        uint32_t *toRead_int = (uint32_t*) &mat[MR(toRead,ASYS)];

        vec->mat.in0_wr = 0;
        ConfigureSimpleVReadShallow(&vec->row, SINT, (int*) toRead_int);        
    } else {
        vec->row.enableRead = 0;
        toRead = -9;
    }
    
    if(timesCalled >= 1 && toCompute >= 0 && toCompute < PK_NROWS){
        uint32_t mask_int = (savedMask) | (savedMask << 8) | (savedMask << 8*2) | (savedMask << 8*3);

        vec->mask.constant = mask_int;
    } else {
        ConfigureSimpleVWriteShallow(&vec->writer, SINT, (int*) nullptr);
        vec->writer.enableWrite = 0;
        toCompute = -9;
    }
    
    if(timesCalled >= 2 && toWrite >= 0){
        uint32_t *toWrite_int = (uint32_t*) &mat[MR(toWrite,ASYS)];

        ConfigureSimpleVWriteShallow(&vec->writer, SINT, (int*) toWrite_int);
    } else {
        toWrite = -9;
        vec->writer.enableWrite = 0;
    }

#if 1
    EndAccelerator();
    StartAccelerator();
#endif

    savedMask = mask;
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

/* input: secret key sk */
/* output: public key pk */
int pk_gen(unsigned char *pk, unsigned char *sk, const uint32_t *perm, int16_t *pi) {
    int i, j, k;
    int row, c;
  
    int mark = MarkArena();
  
    ConfigureSimpleVReadBare(&vec->row);
    ConfigureSimpleVWriteBare(&vec->writer);

    vec->mat.iterA = 1;
    vec->mat.incrA = 1;
    vec->mat.iterB = 1;
    vec->mat.incrB = 1;
    vec->mat.perA = SINT + 1;
    vec->mat.dutyA = SINT + 1;
    vec->mat.perB = SINT + 1;
    vec->mat.dutyB = SINT + 1;

    //uint64_t buf[ 1 << GFBITS ];
    uint64_t* buf = PushArray(1 << GFBITS,uint64_t);
  
    //unsigned char mat[ PK_NROWS ][ SBYTE ];
    unsigned char* mat = MA(PK_NROWS,ASYS,unsigned char);

    unsigned char mask;
    unsigned char b;

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


    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            // For each row
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

            //printf("%d\n",row);

            uint32_t *out_int = (uint32_t*) &(mat[MR(row,ASYS)]);

            VersatLoadRow(out_int);
            bool first = true;
            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[MI(row,i,ASYS)] ^ mat[MI( k , i ,ASYS)];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                VersatMcElieceLoop1(&(mat[MR(k,ASYS)]),mask,first);

                // We could fetch the value from Versat, but it's easier to calculate it CPU side.
                mat[MI(row,i,ASYS)] ^= mat[MI(k,i,ASYS)] & mask;
                first = false;
            }

            // Last run, use valid data to compute last operation
            VersatMcElieceLoop1(&(mat[MR(k,ASYS)]),0,false);  

            if ( uint64_is_zero_declassify((mat[MI(row,i,ASYS) ] >> j) & 1) ) { // return if not systematic
                PopArena(mark);
                return -1;
            }

#if 1
            EndAccelerator();
#endif

            ReadRow(out_int); 

            int index = 0;
            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[MI(k,i,ASYS)] >> j;
                    mask &= 1;
                    mask = -mask;

                    VersatMcElieceLoop2(mat,index,k,row,mask);
                    index += 1;
                }
            }

            VersatMcElieceLoop2(mat,index++,PK_NROWS,row,0);
            VersatMcElieceLoop2(mat,index++,PK_NROWS + 1,row,0);

            vec->writer.enableWrite = 0;
        }
    }

#if 1
    EndAccelerator();
#endif

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, &(mat[i*ASYS]) + PK_NROWS / 8, PK_ROW_BYTES);
    }

    PopArena(mark);
    return 0;
}
