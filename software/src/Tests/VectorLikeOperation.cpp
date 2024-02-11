/*
  This file is for public-key generation
*/

#include "versat_accel.h"
#include "unitConfiguration.hpp"

extern "C"{
#include <string.h>

#include "benes.h"
#include "controlbits.h"
#include "gf.h"
#include "params.h"
#include "pk_gen.h"
#include "root.h"
#include "util.h"
#include "arena.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

//#include "crypto_declassify.h"
//#include "crypto_uint64.h"
//#include "uint64_sort.h"

#include "printf.h"
}

static int test = 0;
static int n_systematic = 0;

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

#define VERSAT
#define SIMULATED_VERSAT

extern "C" void InitMcEliece(int versatBase){
   versat_init(versatBase);
   ConfigCreateVCD(false);
}

void VersatLineXOR(uint8_t* out, uint8_t *mat, uint8_t *row, int n_cols, uint8_t mask) {
   static bool once = true;
   if(once){
      once = false;
#ifdef SIMULATED_VERSAT
       printf("Simulating versat\n");
#else
       printf("Using versat\n");
#endif
   }

   static int times = 0;
   if(times % 10000 == 0) printf("V:%d\n",times);
   times++;

   uint32_t mask_int = (mask) | (mask << 8) | (mask << 8*2) | (mask << 8*3);
   uint32_t *mat_int = (uint32_t*) mat;
   uint32_t *out_int = (uint32_t*) out;
   uint32_t *row_int = (uint32_t*) row;
   int n_cols_int = (n_cols >> 2);

#ifdef SIMULATED_VERSAT
   for(int i = 0; i < n_cols_int; i++){
        uint32_t a = row_int[i] & mask_int;
        uint32_t b = mat_int[i] ^ a;
        out_int[i] = b;
   }
#else
   VectorLikeOperationConfig* vec = (VectorLikeOperationConfig*) accelConfig;
   VectorLikeOperationAddr addr = ACCELERATOR_TOP_ADDR_INIT;

   ConfigureSimpleVRead(&vec->row, n_cols_int, (int*) row_int);
   ConfigureSimpleMemory(&vec->mat,n_cols_int,0,addr.mat,(int*) mat_int);

   vec->mask.constant = mask_int;
   ConfigureMemoryReceive(&vec->output, n_cols_int);

   RunAccelerator(2);

   for (int i = 0; i < n_cols_int; i++){
        out_int[i] = VersatUnitRead((iptr) addr.output.addr,i);
   }
#endif
}

#if 1
/* input: secret key sk */
/* output: public key pk */
int PQCLEAN_MCELIECE348864_CLEAN_pk_gen(uint8_t *pk, uint32_t *perm, const uint8_t *sk) {
    int i, j, k;
    int row, c;

    int mark = MarkArena();
    uint64_t *buf = (uint64_t*) PushBytes((1 << GFBITS)*sizeof(uint64_t));

    uint8_t *mat = (uint8_t*) PushBytes(( GFBITS * SYS_T )*( SYS_N / 8 )*sizeof(uint8_t));
    uint8_t mask;
    uint8_t b;

    gf *g = (gf*) PushBytes((SYS_T + 1)*sizeof(gf)); // Goppa polynomial
    gf *L = (gf*) PushBytes((SYS_N)*sizeof(gf)); // support
    gf *inv = (gf*) PushBytes((SYS_N)*sizeof(gf));

    //

    g[ SYS_T ] = 1;

    for (i = 0; i < SYS_T; i++) {
        g[i] = PQCLEAN_MCELIECE348864_CLEAN_load2(sk);
        g[i] &= GFMASK;
        sk += 2;
    }

    for (i = 0; i < (1 << GFBITS); i++) {
        buf[i] = perm[i];
        buf[i] <<= 31;
        buf[i] |= i;
    }

    PQCLEAN_MCELIECE348864_CLEAN_sort_63b(1 << GFBITS, buf);

    for (i = 0; i < (1 << GFBITS); i++) {
        perm[i] = buf[i] & GFMASK;
    }

    for (i = 0; i < SYS_N;         i++) {
        L[i] = PQCLEAN_MCELIECE348864_CLEAN_bitrev((gf)perm[i]);
    }

    // filling the matrix

    PQCLEAN_MCELIECE348864_CLEAN_root(inv, g, L);

    for (i = 0; i < SYS_N; i++) {
        inv[i] = PQCLEAN_MCELIECE348864_CLEAN_gf_inv(inv[i]);
    }

    for (i = 0; i < PK_NROWS; i++) {
        for (j = 0; j < SYS_N / 8; j++) {
            // mat[i][j] = 0;
            mat[i*(SYS_N/8)+j] = 0;
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

                mat[( i * GFBITS + k)*(SYS_N/8) + (j / 8) ] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = PQCLEAN_MCELIECE348864_CLEAN_gf_mul(inv[j], L[j]);
        }

    }

    // gaussian elimination
    for (i = 0; i < (GFBITS * SYS_T + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= GFBITS * SYS_T) {
                break;
            }

            for (k = row + 1; k < GFBITS * SYS_T; k++) {
                mask = mat[ row*(SYS_N/8) + i ] ^ mat[ k*(SYS_N/8) + i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

#ifdef VERSAT
                if (mask != 0){
                    VersatLineXOR(&(mat[row*(SYS_N/8)+0]), &(mat[row*(SYS_N/8)+0]), &(mat[k*(SYS_N/8)+0]), SYS_N / 8, mask);
                }
#else
                for (c = 0; c < SYS_N / 8; c++) {
                    mat[  row*(SYS_N/8) + c ] ^= mat[ k*(SYS_N/8) + c ] & mask;
                }
#endif
            }

            if ( ((mat[ row*(SYS_N/8) + i ] >> j) & 1) == 0 ) { // return if not systematic
                n_systematic++;
                printf("\ttest: %d | n_systematic: %d\n", test, n_systematic);
                PopArena(mark);
                return -1;
            }

            for (k = 0; k < GFBITS * SYS_T; k++) {
                if (k != row) {
                    mask = mat[ k*(SYS_N/8) + i ] >> j;
                    mask &= 1;
                    mask = -mask;

#ifdef VERSAT
                    if (mask != 0){
                        VersatLineXOR(&(mat[k*(SYS_N/8)+0]), &(mat[k*(SYS_N/8)+0]), &(mat[row*(SYS_N/8)+0]), SYS_N / 8, mask);
                    }
#else                    
                    for (c = 0; c < SYS_N / 8; c++) {
                       mat[ k*(SYS_N/8) + c ] ^= mat[ row*(SYS_N/8) + c ] & mask;
                    }
#endif
                }
            }
        }
    }

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, &(mat[i*(SYS_N/8)]) + PK_NROWS / 8, PK_ROW_BYTES);
    }

    test++; // success: move to next test
    n_systematic = 0; // success: reset counter

    PopArena(mark);
    return 0;
}
#endif

#if 0
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

    uint64_t buf[ 1 << GFBITS ];

    unsigned char mat[ PK_NROWS ][ SYS_N / 8 ];
    unsigned char mask;
    unsigned char b;

    gf g[ SYS_T + 1 ]; // Goppa polynomial
    gf L[ SYS_N ]; // support
    gf inv[ SYS_N ];

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
        for (j = 0; j < SYS_N / 8; j++) {
            mat[i][j] = 0;
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

                mat[ i * GFBITS + k ][ j / 8 ] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = gf_mul(inv[j], L[j]);
        }

    }

    // gaussian elimination

    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[ row ][ i ] ^ mat[ k ][ i ];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                for (c = 0; c < SYS_N / 8; c++) {
                    mat[ row ][ c ] ^= mat[ k ][ c ] & mask;
                }
            }

            if ( uint64_is_zero_declassify((mat[ row ][ i ] >> j) & 1) ) { // return if not systematic
                return -1;
            }

            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[ k ][ i ] >> j;
                    mask &= 1;
                    mask = -mask;

                    for (c = 0; c < SYS_N / 8; c++) {
                        mat[ k ][ c ] ^= mat[ row ][ c ] & mask;
                    }
                }
            }
        }
    }

    for (i = 0; i < PK_NROWS; i++) {
        memcpy(pk + i * PK_ROW_BYTES, mat[i] + PK_NROWS / 8, PK_ROW_BYTES);
    }

    return 0;
}
#endif
