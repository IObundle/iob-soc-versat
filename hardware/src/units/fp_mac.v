`timescale 1ns / 1ps

//
// |<----------DATA_W----------->|
// |_____ _________ _____________|
// |__S__|___EXP___|______F______|
// |     |         |             |
// |<-1->|<-EXP_W->|<----F_W---->|
//

`define BIAS  {1'b0, {(EXP_W-1){1'b1}}}
`define F_W   (DATA_W - EXP_W - 1'b1)
`define MAN_W (`F_W + 1'b1)

`define EXP_MAX {{(EXP_W-1){1'b1}}, 1'b0}
`define EXP_MIN {{(EXP_W-1){1'b0}}, 1'b1}
`define EXP_INF {EXP_W{1'b1}}
`define EXP_NAN {EXP_W{1'b1}}
`define EXP_SUB {EXP_W{1'b0}}

`define EXTRA 3

// Canonical NAN
`define NAN {1'b0, `EXP_NAN, 1'b1, {(DATA_W-EXP_W-2){1'b0}}}

// Infinite
`define INF(SIGN) {SIGN, `EXP_INF, {(DATA_W-EXP_W-1){1'b0}}}

module fp_mac #(
                parameter DATA_W = 32,
                parameter EXP_W = 8,
                parameter GUARD_W = 10
                )
   (
    input                          clk,
    input                          rst,

    input                          start,
    output reg                     done,

    input                          op_a_sign,
    input [EXP_W-1:0]              op_a_exp,
    input [`MAN_W-1:0]             op_a_man,

    input                          op_b_sign,
    input [EXP_W-1:0]              op_b_exp,
    input [`MAN_W-1:0]             op_b_man,

    output reg                     res_sign,
    output reg [EXP_W+1:0]         res_exp,
    output reg [`MAN_W+`EXTRA-1:0] res_man,
    output                         res_inv
    );

   wire                            A_sign     = op_a_sign;
   wire [EXP_W-1:0]                A_Exponent = op_a_exp;
   wire [`MAN_W-1:0]               A_Mantissa = op_a_man;

   wire                            B_sign     = op_b_sign;
   wire [EXP_W-1:0]                B_Exponent = op_b_exp;
   wire [`MAN_W-1:0]               B_Mantissa = op_b_man;

   // pipeline stage 1
   reg                             A_sign_reg;
   reg [EXP_W-1:0]                 A_Exponent_reg;
   reg [`MAN_W-1:0]                A_Mantissa_reg;

   reg                             B_sign_reg;
   reg [EXP_W-1:0]                 B_Exponent_reg;
   reg [`MAN_W-1:0]                B_Mantissa_reg;

   reg                             done_int;
   always @(posedge clk) begin
      if (rst) begin
         A_sign_reg <= 1'b0;
         A_Exponent_reg <= {EXP_W{1'b0}};
         A_Mantissa_reg <= {`MAN_W{1'b0}};

         B_sign_reg <= 1'b0;
         B_Exponent_reg <= {EXP_W{1'b0}};
         B_Mantissa_reg <= {`MAN_W{1'b0}};

         done_int <= 1'b0;
      end else begin
         A_sign_reg <= A_sign;
         A_Exponent_reg <= A_Exponent;
         A_Mantissa_reg <= A_Mantissa;

         B_sign_reg <= B_sign;
         B_Exponent_reg <= B_Exponent;
         B_Mantissa_reg <= B_Mantissa;

         done_int <= start;
      end
   end

   // Multiplication
   wire                            mul_sign = A_sign_reg ^ B_sign_reg;
   wire [EXP_W+1:0]                mul_exp  = {2'd0, A_Exponent_reg} + {2'd0, B_Exponent_reg} - `BIAS;
   wire [2*`MAN_W-1:0]             mul_man  = A_Mantissa_reg * B_Mantissa_reg;

   // pipeline stage 2
   reg                             mul_sign_reg;
   reg [EXP_W+1:0]                 mul_exp_reg;
   reg [2*`MAN_W-1:0]              mul_man_reg;

   reg                             done_int2;
   always @(posedge clk) begin
      if (rst) begin
         mul_sign_reg <= 1'b0;
         mul_exp_reg  <= {(EXP_W+2){1'b0}};
         mul_man_reg  <= {(2*`MAN_W){1'b0}};

         done_int2 <= 1'b0;
      end else begin
         mul_sign_reg <= mul_sign;
         mul_exp_reg  <= mul_exp;
         mul_man_reg  <= mul_man;

         done_int2 <= done_int;
      end
   end

   localparam ACC_W = 2*`MAN_W+GUARD_W+1;
   localparam STICKY_BITS = ACC_W+3;

   // Accumulation in 2's Complement
   wire [2*`MAN_W:0]               mul_man_c_ = ({1'b0, mul_man_reg} ^ {(2*`MAN_W+1){mul_sign_reg}});
   wire [ACC_W:0]                  mul_man_c  = {{(GUARD_W+1){mul_man_c_[2*`MAN_W]}}, mul_man_c_};

   wire [EXP_W+1:0]                A_exp_int = done_int2? mul_exp_reg: acc_exp_reg2;
   wire [ACC_W:0]                  A_man_int = done_int2? mul_man_c: acc_man_reg2;

   wire [EXP_W+1:0]                B_exp_int = acc_exp_reg;
   wire [ACC_W:0]                  B_man_int = acc_man_reg;

   wire                            comp = (A_exp_int >= B_exp_int)? 1'b1 : 1'b0;

   wire [EXP_W+1:0]                A_exp = comp? A_exp_int: B_exp_int;
   wire [ACC_W-1:0]                A_man = comp? A_man_int: B_man_int;

   wire [EXP_W+1:0]                B_exp = comp? B_exp_int: A_exp_int;
   wire [ACC_W-1:0]                B_man = comp? B_man_int: A_man_int;

   // Align significants
   wire [EXP_W+1:0]                diff_exp = A_exp - B_exp;
   wire [EXP_W+1:0]                shift = (diff_exp > STICKY_BITS)? STICKY_BITS: diff_exp;
   wire [ACC_W+STICKY_BITS:0]      B_man_in = $signed({B_man, {STICKY_BITS{1'b0}}}) >>> shift;

   // Sticky
   wire                            sticky_bit = |B_man_in[STICKY_BITS:0];

   // pipeline stage 3
   reg [EXP_W+1:0]                 A_exp_reg;
   reg [ACC_W:0]                   A_man_reg;

   reg [ACC_W:0]                   B_man_reg;

   reg                             cin;

   reg                             done_int3;
   always @(posedge clk) begin
      if (rst) begin
         A_exp_reg <= {(EXP_W+2){1'b0}};
         A_man_reg <= {(ACC_W+1){1'b0}};

         B_man_reg <= {(ACC_W+1){1'b0}};

         cin <= 1'b0;

         done_int3 <= 1'b0;
      end else begin
         A_exp_reg <= A_exp;
         A_man_reg <= A_man;

         B_man_reg <= {B_man_in[STICKY_BITS+1 +: ACC_W], sticky_bit};

         cin <= mul_sign_reg;

         done_int3 <= done_int2;
      end
   end

   // Accumulation
   wire [ACC_W:0]                  acc_man = A_man_reg + B_man_reg + cin;

   // pipeline stage 4
   wire                            rst_acc  = (done_int & ~done_int2) | (done_int2 & ~done_int3);
   wire [EXP_W+1:0]                acc_exp_ = rst_acc? {{(EXP_W+1){1'b0}}, 1'b1}: A_exp_reg;
   wire [ACC_W:0]                  acc_man_ = rst_acc? {(ACC_W+1){1'b0}}: acc_man;

   reg [EXP_W+1:0]                 acc_exp_reg, acc_exp_reg2;
   reg [ACC_W:0]                   acc_man_reg, acc_man_reg2;

   reg                             done_int4;
   always @(posedge clk) begin
      if (rst) begin
         acc_exp_reg <= {(EXP_W+2){1'b0}};
         acc_man_reg <= {(ACC_W+1){1'b0}};

         acc_exp_reg2 <= {(EXP_W+2){1'b0}};
         acc_man_reg2 <= {(ACC_W+1){1'b0}};

         done_int4 <= 1'b0;
      end else begin
         acc_exp_reg <= acc_exp_;
         acc_man_reg <= acc_man_;

         acc_exp_reg2 <= acc_exp_reg;
         acc_man_reg2 <= acc_man_reg;

         done_int4 <= done_int3;
      end
   end

   // Convert final result to sign-magnitude
   wire                            res_sign_int = acc_man_reg[ACC_W];

   localparam EXP_MIN = 0;
   localparam RSHIFT_LIM = `MAN_W+2;

   // Normalize
   localparam LSC_W  = $clog2(ACC_W+2);
   localparam OFFSET = GUARD_W+4; // 3 -> 1 for the carry from the accumulator, 1 for the extra integer bit and 1 for the 2's Complement

   wire [LSC_W-1:0]                lsc;
   cls #(
         .DATA_W(ACC_W+1)
         )
   cls0
     (
      .data_in  (acc_man_reg),
      .symbol   (res_sign_int),
      .data_out (lsc)
      );

   wire [EXP_W+1:0]                exp_diff  = EXP_MIN - acc_exp_reg + lsc;
   wire [EXP_W+1:0]                exp_diff2 = EXP_MIN - acc_exp_reg;
   wire [EXP_W+1:0]                exp_diff2_1 = acc_exp_reg - EXP_MIN;

   wire [EXP_W+1:0]                lshift = exp_diff[EXP_W+1]? lsc: exp_diff2_1;
   wire [EXP_W+1:0]                lshift_inv = ~lshift;

   wire [EXP_W+1:0]                rshift = (exp_diff2 > RSHIFT_LIM)? RSHIFT_LIM: exp_diff2;
   wire [ACC_W+RSHIFT_LIM:0]       man_rsht = {acc_man_reg, {RSHIFT_LIM{1'b0}}} >> rshift;

   wire [EXP_W+1:0]                exp_norm = exp_diff2[EXP_W+1]? acc_exp_reg + lshift_inv + OFFSET:
                                                                  1'b0;
   wire [ACC_W:0]                  man_norm = exp_diff2[EXP_W+1]? acc_man_reg << lshift:
                                                                  {man_rsht[ACC_W+RSHIFT_LIM:RSHIFT_LIM+1], |man_rsht[RSHIFT_LIM:0]};

   wire [EXP_W+1:0]                exp_adjust = exp_norm;
   wire [`MAN_W+`EXTRA-1:0]        man_adjust = {man_norm[ACC_W -: `MAN_W+`EXTRA-1], |man_norm[ACC_W-(`MAN_W-`EXTRA-1):0]};

   // pipeline stage 5
   reg                             done_int5, done_int6;
   always @(posedge clk) begin
      if (rst) begin
         res_sign <= 1'b0;
         res_exp <= {(EXP_W+2){1'b0}};
         res_man <= {(`MAN_W+`EXTRA){1'b0}};

         done_int5 <= 1'b0;
         done_int6 <= 1'b0;
         done <= 1'b0;
      end else begin
         res_sign <= res_sign_int;
         res_exp <= exp_adjust;
         res_man <= man_adjust;

         done_int5 <= done_int4;
         done_int6 <= done_int5;
         done <= done_int6 & ~done_int5;
      end
   end

   assign res_inv = res_sign;

endmodule
