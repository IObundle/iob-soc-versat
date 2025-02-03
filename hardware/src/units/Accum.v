`timescale 1ns / 1ps

module Accum(
   input                   clk,
   input                   rst,

   input                   running,
   input                   run,

   input [31:0]            in0,      
   output [31:0]           out0,      

   input [6:0]             duty,
   input [6:0]             delay0
   );

reg [6:0] delay;
reg [31:0] accum;
reg working;

assign out0 = accum;

always @(posedge clk, posedge rst) begin
   if (rst) begin
      delay <= 0;
      working <= 0;
   end else if (run) begin
      delay <= delay0;
      working <= 0;
   end else if (|delay) begin
      delay <= delay - 1;
   end else begin
      delay <= duty;
      working <= 1'b1;
   end
end

wire resetAccum = (delay == 0);

always @(posedge clk, posedge rst) begin
   if(rst) begin
      accum <= 0;
   end else if(resetAccum) begin
      accum <= 0;
   end else if(working) begin
      accum <= accum + in0;
   end
end

endmodule 