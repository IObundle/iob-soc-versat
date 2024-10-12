`timescale 1ns / 1ps

module TestParameters #(
   parameter P1 = 32,
   parameter P2 = DATA_W + 1,
   parameter P3 = $clog2(P2 + P1)
   )
   (

   );

endmodule