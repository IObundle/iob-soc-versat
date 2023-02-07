#if 1

#include "logger.hpp"

#include "versatExtra.hpp"
#include "unitConfiguration.hpp"

extern "C"{
   #include "crypto/blake2/blake2.h"

static inline uint32_t rotr32( const uint32_t w, const unsigned c )
{
  return ( w >> c ) | ( w << ( 32 - c ) );
}

#undef G
#undef ROUND

#define G(m0,m1,a,b,c,d)                      \
  do {                                      \
    a = a + b + m0; \
    d = rotr32(d ^ a, 16);                  \
    c = c + d;                              \
    b = rotr32(b ^ c, 12);                  \
    a = a + b + m1; \
    d = rotr32(d ^ a, 8);                   \
    c = c + d;                              \
    b = rotr32(b ^ c, 7);                   \
  } while(0)

#define ROUND(r)                    \
  do {                              \
    G(m[blake2s_sigma[r][0]],m[blake2s_sigma[r][1]],v[ 0],v[ 4],v[ 8],v[12]); \
    G(m[blake2s_sigma[r][2]],m[blake2s_sigma[r][3]],v[ 1],v[ 5],v[ 9],v[13]); \
    G(m[blake2s_sigma[r][4]],m[blake2s_sigma[r][5]],v[ 2],v[ 6],v[10],v[14]); \
    G(m[blake2s_sigma[r][6]],m[blake2s_sigma[r][7]],v[ 3],v[ 7],v[11],v[15]); \
    G(m[blake2s_sigma[r][8]],m[blake2s_sigma[r][9]],v[ 0],v[ 5],v[10],v[15]); \
    G(m[blake2s_sigma[r][10]],m[blake2s_sigma[r][11]],v[ 1],v[ 6],v[11],v[12]); \
    G(m[blake2s_sigma[r][12]],m[blake2s_sigma[r][13]],v[ 2],v[ 7],v[ 8],v[13]); \
    G(m[blake2s_sigma[r][14]],m[blake2s_sigma[r][15]],v[ 3],v[ 4],v[ 9],v[14]); \
  } while(0)

static inline uint32_t load32( const void *src )
{
#if defined(NATIVE_LITTLE_ENDIAN)
  uint32_t w;
  memcpy(&w, src, sizeof w);
  return w;
#else
  const uint8_t *p = ( const uint8_t * )src;
  return (( uint32_t )( p[0] ) <<  0) |
         (( uint32_t )( p[1] ) <<  8) |
         (( uint32_t )( p[2] ) << 16) |
         (( uint32_t )( p[3] ) << 24) ;
#endif
}

static const uint32_t blake2s_IV[8] =
{
  0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
  0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static const uint8_t blake2s_sigma[10][16] =
{
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } , // 0
  { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 } ,
  { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 } ,
  {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 } ,
  {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 } ,
  {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 } ,
  { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 } ,
  { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 } ,
  {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 } ,
  { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 } , // 9
};

extern Versat* versat;

void blake2s_compress( blake2s_state *S, const uint8_t in[BLAKE2S_BLOCKBYTES] ){
  static SimpleAccelerator blake2G;
  static SimpleAccelerator blake2Round;
  InitSimpleAccelerator(&blake2G,versat,"Blake2G");
  InitSimpleAccelerator(&blake2Round,versat,"Blake2Round");

  uint32_t m[16];
  uint32_t v[16];
  size_t i;

  LogOnce(LogModule::TOP_SYS,LogLevel::INFO,"Versat blake2");

  for( i = 0; i < 16; ++i ) {
    m[i] = load32( in + i * sizeof( m[i] ) );
  }

  for( i = 0; i < 8; ++i ) {
    v[i] = S->h[i];
  }

  v[ 8] = blake2s_IV[0];
  v[ 9] = blake2s_IV[1];
  v[10] = blake2s_IV[2];
  v[11] = blake2s_IV[3];
  v[12] = S->t[0] ^ blake2s_IV[4];
  v[13] = S->t[1] ^ blake2s_IV[5];
  v[14] = S->f[0] ^ blake2s_IV[6];
  v[15] = S->f[1] ^ blake2s_IV[7];

  // The M thing could be well implemented if we could use a VRead + Lookup table combo.
  // VRead reads the m array, a constant mem contains the lookup indices.
  #if 0
  int* out;
  #define VG(i,a,b,c,d) \
    do{ \
    out =  RunSimpleAccelerator(&blake2G,a,b,c,d,m[blake2s_sigma[0][i]],m[blake2s_sigma[0][i+1]]); \
    a = out[0]; \
    b = out[1]; \
    c = out[2]; \
    d = out[3]; \
    } while(0)

  VG(0,v[0],v[4],v[8],v[12]);
  VG(2,v[ 1],v[ 5],v[ 9],v[13]);
  VG(4,v[ 2],v[ 6],v[10],v[14]);
  VG(6,v[ 3],v[ 7],v[11],v[15]);
  VG(8,v[ 0],v[ 5],v[10],v[15]);
  VG(10,v[ 1],v[ 6],v[11],v[12]);
  VG(12,v[ 2],v[ 7],v[ 8],v[13]);
  VG(14,v[ 3],v[ 4],v[ 9],v[14]);
  #else
  FUInstance* mInst = GetInstanceByName(blake2Round.accel,"Test","m");
  ConfigureMemoryLinear(mInst,16);
  for(int i = 0; i < 16; i++){
    VersatUnitWrite(mInst,i,m[blake2s_sigma[0][i]]);
  }
  int* out =  RunSimpleAccelerator(&blake2Round,v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],v[11],v[12],v[13],v[14],v[15]);

  for(int i = 0; i < 16; i++){
    v[i] = out[i];
  }
  #endif

  //ROUND( 0 );
  ROUND( 1 );
  ROUND( 2 );
  ROUND( 3 );
  ROUND( 4 );
  ROUND( 5 );
  ROUND( 6 );
  ROUND( 7 );
  ROUND( 8 );
  ROUND( 9 );

  for( i = 0; i < 8; ++i ) {
    S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];
  }
}

} // extern "C"

#undef G
#undef ROUND

#endif

