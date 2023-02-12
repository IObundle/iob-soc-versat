#if 0

#define FFT_SIZE 1024

#include "versatExtra.hpp"

#include <cstdio>

extern Versat* versat;

// Straded fft implementation using versat
void VersatStradedFFT(float real[FFT_SIZE], float img[FFT_SIZE], float real_twid[FFT_SIZE/2], float img_twid[FFT_SIZE/2]){
   static SimpleAccelerator simple = {};
   InitSimpleAccelerator(&simple,versat,"SimpleFFTCalc");
   float temp;

   int log = 0;
   // Loops for log(FFT_SIZE) times
   for(int span = FFT_SIZE >> 1; span; span >>= 1, log++){ // Span goes down powers of 2
      // Loops for FFT_SIZE / 2 times
      for(int odd = span; odd < FFT_SIZE; odd++){ // Odd goes from span until 1024, increases by one but due to the (odd |= span), it jumps a few regions
         odd |= span;
         int even = odd ^ span;

         // Memory access real using even and odd indexes.
         // Read: real[even],real[odd]
         // Write:real[even],real[odd]
         float* out = (float*) RunSimpleAccelerator(&simple,PackInt(real[even]),PackInt(real[odd]));
         float out0 = out[0];
         float out1 = out[1];

         #if 1
         real[odd] = out0;
         real[even] = out1;
         #endif

         #if 0
         temp = real[even] + real[odd];
         real[odd] = real[even] - real[odd];
         real[even] = temp;
         #endif

         // Memory access img using even and odd indexes.
         // Read: img[even],img[odd]
         // Write:img[even],img[odd]
         temp = img[even] + img[odd];
         img[odd] = img[even] - img[odd];
         img[even] = temp;

         int rootindex = (even<<log) & (FFT_SIZE - 1);

         // Memory access img and real using odd and rootIndex values
         // Read: real[odd],img[odd] and twiddle arrays
         // Write:real[odd],img[odd]
         if(rootindex){
            temp = real_twid[rootindex] * real[odd] - img_twid[rootindex]  * img[odd];
            img[odd] = real_twid[rootindex]*img[odd] + img_twid[rootindex]*real[odd];
            real[odd] = temp;
         }
      }
   }
}

/*

Span = 512. Odd goes 512..1023 | Even goes 0..511 | Root index goes 0..511 in increments of 1
Span = 256. Odd goes 256..511 , 768..1023 | Even goes 0..255 , 512..767 | Root index goes 0..510 in increments of 2 twice (it loops back)
Span = 128. Odd goes 128..255 , 384..511 , 640..767 , 896..1023 | Even goes 0..127 , 256..383 , 512..639 , 768..895 | Root index goes 0..508 in increments of 4 four times

Each iteration, we sweep a increasing multiple of 2 area, odd sweeps a portion and even sweeps the other.
The root index node only hits the lower portion and due to the increase in increments, reaches the point where it is only zero (one iteration after it only swaps between 0 and 256) [Meaning that the if is not taken in the very last iteration]
The log is the number of outer iterations. Goes 0..9.


*/


#endif





