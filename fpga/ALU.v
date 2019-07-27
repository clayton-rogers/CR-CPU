`default_nettype none

module ALU (i_opcode, i_extra, i_data1, i_data2, i_const, o_data);
  input [3:0] i_opcode;
  input [1:0] i_extra;
  input [15:0] i_data1;
  input [15:0] i_data2;
  input [7:0] i_const;
  output reg [15:0] o_data;

  `include "opcodes.vh"

  // For add, sub, and, or
  wire operant2_switch = i_extra[0];
  wire [15:0] operant2 = (operant2_switch == 1'b0) ? i_data2 : {8'h00, i_const};

  wire shift_dir = i_extra[0];
  localparam SHIFT_RIGHT = 1'b0;
  localparam SHIFT_LEFT = 1'b1;
  wire shift_source = i_extra[1];
  localparam SHIFT_FROM_RB = 1'b0;
  localparam SHIFT_FROM_CONST = 1'b1;
  reg [15:0] shift_amount;
  always @ ( * ) begin
    shift_amount = 16'h0000;
    case (shift_source)
      SHIFT_FROM_RB    : shift_amount = i_data2;
      SHIFT_FROM_CONST : shift_amount = {8'h00 , i_const};
    endcase
  end

  always @ ( * ) begin
    case (i_opcode)
      ADD    : o_data = i_data1 + operant2;
      SUB    : o_data = i_data1 - operant2;
      AND    : o_data = i_data1 & operant2;
      OR     : o_data = i_data1 | operant2;
      SHIFT  : begin
        case (shift_dir)
          SHIFT_RIGHT : o_data = i_data1 >> shift_amount;
          SHIFT_LEFT  : o_data = i_data1 << shift_amount;
        endcase
        end
      LOAD   : o_data = 16'h0000; // ALU is unused
      STORE  : o_data = 16'h0000; // ALU is unused
      MOVE   : o_data = i_data1;
      JUMP   : o_data = 16'h0000; // ALU is unused
      LOADC  : o_data = 16'h0000; // ALU is unused
      UNDEF1 : o_data = 16'h0000; // ALU is unused
      UNDEF2 : o_data = 16'h0000; // ALU is unused
      UNDEF3 : o_data = 16'h0000; // ALU is unused
      UNDEF4 : o_data = 16'h0000; // ALU is unused
      UNDEF5 : o_data = 16'h0000; // ALU is unused
      UNDEF6 : o_data = 16'h0000; // ALU is unused
    endcase
  end

endmodule
