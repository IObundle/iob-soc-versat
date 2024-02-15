/*
  This file is for public-key generation
*/

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "controlbits.h"
#include "benes.h"
#include "crypto_declassify.h"
#include "crypto_uint64.h"
#include "params.h"
#include "pk_gen.h"
#include "root.h"
#include "uint64_sort.h"
#include "util.h"

#include "printf.h"
#include "arena.h"

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

// Matrix Allocate
#define MA(Y,X,TYPE) PushBytes(sizeof(TYPE) * (X) * (Y))
// Matrix Index

#define MI(Y,X,ROWSIZE) ((Y) * (ROWSIZE) + (X)) 
//#define MI(Y,X,ROWSIZE) Y][X 

void VersatLineXOR(uint8_t* out, uint8_t *mat, uint8_t *row, int n_cols, uint8_t mask);

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
  
    //unsigned char mat[ PK_NROWS ][ SYS_N / 8 ];
    unsigned char* mat = MA(PK_NROWS,SYS_N / 8,unsigned char);

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
        for (j = 0; j < SYS_N / 8; j++) {
          mat[MI(i , j ,SYS_N / 8)] = 0;
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

                mat[MI( i * GFBITS + k, j / 8,SYS_N / 8)] = b;
            }
        }

        for (j = 0; j < SYS_N; j++) {
            inv[j] = gf_mul(inv[j], L[j]);
        }

    }

#if 1
    // This entiry thing needs to be speedup.
    for (i = 0; i < (PK_NROWS + 7) / 8; i++) {
        for (j = 0; j < 8; j++) {
            row = i * 8 + j;

            if (row >= PK_NROWS) {
                break;
            }

            for (k = row + 1; k < PK_NROWS; k++) {
                mask = mat[MI( row , i,SYS_N / 8) ] ^ mat[MI( k , i ,SYS_N / 8)];
                mask >>= j;
                mask &= 1;
                mask = -mask;

                // Each loop, k changes, row remains constant inside this loop.
                // So out and mat remain constant inside each loop.
                // Basically, load mat[row], use vread to load 

                

                VersatLineXOR(&(mat[row*(SYS_N/8)+0]), &(mat[row*(SYS_N/8)+0]), &(mat[k*(SYS_N/8)+0]), SYS_N / 8, mask);
#if 0             
                for (c = 0; c < SYS_N / 8; c++) {
                  
                  //mat[MI( row , c ,SYS_N / 8)] ^= mat[MI( k , c ,SYS_N / 8) ] & mask;
                }
#endif             
            }

            if ( uint64_is_zero_declassify((mat[ MI( row , i ,SYS_N / 8) ] >> j) & 1) ) { // return if not systematic
                PopArena(mark);
                return -1;
            }

            for (k = 0; k < PK_NROWS; k++) {
                if (k != row) {
                    mask = mat[MI( k , i ,SYS_N / 8)] >> j;
                    mask &= 1;
                    mask = -mask;

                    for (c = 0; c < SYS_N / 8; c++) {
                        mat[MI( k , c ,SYS_N / 8)] ^= mat[MI( row , c ,SYS_N / 8)] & mask;
                    }
                }
            }
        }
    }
#endif

    for (i = 0; i < PK_NROWS; i++) {
      memcpy(pk + i * PK_ROW_BYTES, &(mat[i*(SYS_N/8)]) + PK_NROWS / 8, PK_ROW_BYTES);
    }

    PopArena(mark);
    return 0;
}
#endif