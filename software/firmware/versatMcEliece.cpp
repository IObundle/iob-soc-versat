#if 0

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

extern "C"{
#include "crypto/mceliece/mceliece348864/controlbits.h"
#include "crypto/mceliece/mceliece348864/uint64_sort.h"
#include "crypto/mceliece/mceliece348864/pk_gen.h"
#include "crypto/mceliece/mceliece348864/params.h"
#include "crypto/mceliece/mceliece348864/benes.h"
#include "crypto/mceliece/mceliece348864/root.h"
#include "crypto/mceliece/mceliece348864/util.h"
#include "crypto/mceliece/mceliece348864/subroutines/crypto_declassify.h"
#include "crypto/mceliece/mceliece348864/subroutines/crypto_uint64.h"

#include "iob-timer.h"
}

#include "versat.hpp"

extern Versat* versat;

// GFBITS   = 12
// GFMASK   = Mask of (12) = 4095
// SYS_N    = 3488
// SYS_N/8  = 436
// SYS_T    = 64
// PK_NROWS = 768

void Print(unsigned char mat[][436]){
	for(int y = 0; y < 768; y++){
		printf("%d: ",y);
		for(int x = 0; x < 436; x++){
			printf("%x ",(int)mat[y][x]);
		}
		printf("\n");
	}
}

#include "unitConfiguration.hpp"
#include "debug.hpp"
#include "logger.hpp"

void VersatSim(unsigned char* matRow,unsigned char* kRow, unsigned char mask){
	static Accelerator* accel = nullptr;
	static int timesCalled = 0;

	// Running versat in pc-emul all the time is slow. Only some times
	bool runVersat = ((++timesCalled + COMPILE_TIME) % 10000 == 0);

	#if 0
	runVersat = false;
	#endif // 0

	if(runVersat && !accel){
		accel = CreateAccelerator(versat);
		FUDeclaration* type = GetTypeByName(versat,STRING("VectorLikeOperation"));
		CreateFUInstance(accel,type,STRING("Top"),false,false);
	}

	FUInstance* outputInst;
	unsigned char versatRow[436];
	if(runVersat){
		LogOnce(LogModule::TOP_SYS,LogLevel::DEBUG,"Versat McEliece run");

		union{
			int asInt;
			char byte[4];
		}repl;

		repl.byte[0] = mask;
		repl.byte[1] = mask;
		repl.byte[2] = mask;
		repl.byte[3] = mask;

		FUInstance* rowInst = GetInstanceByName(accel,"Top","row");
		ConfigureSimpleVRead(rowInst,436 / 4,(int*) kRow);
		FUInstance* maskInst = GetInstanceByName(accel,"Top","mask");
		maskInst->config[0] = repl.asInt;
		FUInstance* matInst = GetInstanceByName(accel,"Top","mat");
		ConfigureMemoryLinear(matInst,436 / 4);

		for (int c = 0; c < (SYS_N/8) / 4; c++){
			for (int i = 0; i < 4; i++){
				repl.byte[i] = matRow[c*4+i];
			}
			VersatUnitWrite(matInst,c,repl.asInt);
		}

		outputInst = GetInstanceByName(accel,"Top","output");
		ConfigureMemoryReceive(outputInst,436 / 4,1);

		AcceleratorRun(accel); // Fills vread with valid data
		AcceleratorRun(accel);

		for (int c = 0; c < SYS_N/8; c++){
			versatRow[c] = SelectByte(VersatUnitRead(outputInst,c / 4),c % 4);
		}
	}

	// The expected result
	for (int c = 0; c < SYS_N/8; c++){
		matRow[c] ^= kRow[c] & mask;
	}

	if(runVersat){
		if(memcmp(versatRow,matRow,436) != 0){
			printf("Versat:\n");
			OutputMemoryHex(versatRow,16);
			printf("Good:\n");
			OutputMemoryHex(matRow,16);
			fflush(stdout);
			assert(false);
		} else {
			LogOnce(LogModule::TOP_SYS,LogLevel::DEBUG,"Looking good so far");
		}
	}
}

int pk_gen(unsigned char * pk, unsigned char * sk, uint32_t * perm, int16_t * pi)
{
	int i, j, k;
	int row, c;

	uint64_t buf[ 1 << GFBITS ];

	unsigned char mat[ PK_NROWS ][ SYS_N/8 ]; // Matrix 768 * 436 = 334848
	unsigned char mask;
	unsigned char b;

	gf g[ SYS_T+1 ]; // Goppa polynomial
	gf L[ SYS_N ]; // support
	gf inv[ SYS_N ];

	//
	{
	TIME_IT("A");
	g[ SYS_T ] = 1;

	for (i = 0; i < SYS_T; i++) { g[i] = load_gf(sk); sk += 2; }

	for (i = 0; i < (1 << GFBITS); i++)
	{
		buf[i] = perm[i];
		buf[i] <<= 31;
		buf[i] |= i;
	}

	uint64_sort(buf, 1 << GFBITS);

	for (i = 1; i < (1 << GFBITS); i++)
		if (crypto_uint64_equal_mask(buf[i-1] >> 31,buf[i] >> 31))
			return -1;

	for (i = 0; i < (1 << GFBITS); i++) pi[i] = buf[i] & GFMASK;
	for (i = 0; i < SYS_N;         i++) L[i] = bitrev(pi[i]);
	} // TIME_IT('A');

	// filling the matrix
	{
	TIME_IT("B");
	root(inv, g, L);

	for (i = 0; i < SYS_N; i++)
		inv[i] = gf_inv(inv[i]);

	for (i = 0; i < PK_NROWS; i++)
	for (j = 0; j < SYS_N/8; j++)
		mat[i][j] = 0;

	for (i = 0; i < SYS_T; i++)
	{
		for (j = 0; j < SYS_N; j+=8)
		for (k = 0; k < GFBITS;  k++)
		{
			b  = (inv[j+7] >> k) & 1; b <<= 1;
			b |= (inv[j+6] >> k) & 1; b <<= 1;
			b |= (inv[j+5] >> k) & 1; b <<= 1;
			b |= (inv[j+4] >> k) & 1; b <<= 1;
			b |= (inv[j+3] >> k) & 1; b <<= 1;
			b |= (inv[j+2] >> k) & 1; b <<= 1;
			b |= (inv[j+1] >> k) & 1; b <<= 1;
			b |= (inv[j+0] >> k) & 1;

			mat[ i*GFBITS + k ][ j/8 ] = b;
		}

		for (j = 0; j < SYS_N; j++)
			inv[j] = gf_mul(inv[j], L[j]);

	}
	} // TIME_IT('B');

	// gaussian elimination
	{
	TIME_IT("C");

	#if 0
	#define DO_PRINT
	#endif

	#ifdef DO_PRINT
	Print(mat);
	#endif
	for (i = 0; i < (PK_NROWS + 7) / 8; i++){
		for (j = 0; j < 8; j++){
			row = i*8 + j;

			#if 0 // Does not appear to do anything
			if (row >= PK_NROWS)
				break;
			#endif

			for (k = row + 1; k < PK_NROWS; k++){ // For each row below the current row

				// This computation should be made in the cpu. It's performed 294528 times, but accelerating through versat is clubersome as it's basically two memory accesses
				mask = mat[ row ][ i ] ^ mat[ k ][ i ];
				mask >>= j;
				mask &= 1;
				mask = -mask;

				// Basically computes a element wise XOR with a mask for each row below the selected row
				// In other words, a vector like operation 436 times. Memory is continuous

				VersatSim(&mat[row][0],&mat[k][0],mask);
			}

			// This piece of code is here to terminate earlier. The check can be postponed to the end
			#if 1
			if ( crypto_uint64_zero_mask((mat[ row ][ i ] >> j) & 1) ){ // return if not systematic
				#ifdef DO_PRINT
				printf("\n\n\n NOT SYSTEMATIC AT %d: \n\n\n",row);
				Print(mat);
				#endif
				return -1;
			}
			#endif

			for (k = 0; k < PK_NROWS; k++){
				if (k != row){
					mask = mat[ k ][ i ] >> j;
					mask &= 1;
					mask = -mask;

					VersatSim(&mat[k][0],&mat[row][0],mask);
				}
			}
		}
	}

	// There is no point implementing this part in hardware, as we are basically just checking one value per row
	#if 0
	{
	TIME_IT("D");
	for (i = 0; i < (PK_NROWS + 7) / 8; i++){
		for (j = 0; j < 8; j++){
			row = i*8 + j;
			if ( crypto_uint64_zero_mask((mat[ row ][ i ] >> j) & 1) ){ // return if not systematic
				#ifdef DO_PRINT
				printf("\n\n\n NOT SYSTEMATIC AT %d: \n\n\n",row);
				Print(mat);
				#endif
				return -1;
			}
		}
	}
	} // TIME_IT('D');
	#endif

	} // TIME_IT('C');
	#ifdef DO_PRINT
	printf("\n\n\n========================\n\n\n");
	Print(mat);
	fflush(stdout);
	assert(false);
	#endif

	for (i = 0; i < PK_NROWS; i++)
		memcpy(pk + i*PK_ROW_BYTES, mat[i] + PK_NROWS/8, PK_ROW_BYTES);

	return 0;
}

#endif
