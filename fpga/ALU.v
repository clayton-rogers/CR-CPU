`default_nettype none

module ALU (i_opcode, i_extra, i_data1, i_data2, o_data);
  input [3:0] i_opcode;
  input [1:0] i_extra;
  input [15:0] i_data1;
  input [15:0] i_data2;
  output reg [15:0] o_data;

  wire shift_dir = i_extra[0];
  localparam SHIFT_RIGHT = 1'b0;
  localparam SHIFT_LEFT = 1'b1;

  `include "opcodes.vh"
  always @ ( * ) begin
    case (i_opcode)
      ADD    : o_data = i_data1 + i_data2;
      SUB    : o_data = i_data1 - i_data2;
      AND    : o_data = i_data1 & i_data2;
      OR     : o_data = i_data1 | i_data2;
      SHIFT  : begin
        case (shift_dir)
          SHIFT_RIGHT : o_data = i_data1 >> i_data2;
          SHIFT_LEFT  : o_data = i_data1 << i_data2;
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
