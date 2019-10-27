`default_nettype none

module ALU (i_opcode, i_shift_dir, i_data1, i_data2, o_data);
  input [3:0] i_opcode;
  input i_shift_dir;
  input [15:0] i_data1;
  input [15:0] i_data2;
  output reg [15:0] o_data;

  localparam SHIFT_RIGHT = 1'b0;
  localparam SHIFT_LEFT = 1'b1;

  `include "opcodes.vh"
  always @ ( * ) begin
    case (i_opcode)
      ADD    : o_data = i_data1 + i_data2;
      SUB    : o_data = i_data1 - i_data2;
      AND    : o_data = i_data1 & i_data2;
      OR     : o_data = i_data1 | i_data2;
      XOR    : o_data = i_data1 ^ i_data2;
      SHIFT  : begin
        case (i_shift_dir)
          SHIFT_RIGHT : o_data = i_data1 >> i_data2;
          SHIFT_LEFT  : o_data = i_data1 << i_data2;
        endcase
        end
      LOAD   : o_data = 16'h0000; // ALU is unused
      STORE  : o_data = 16'h0000; // ALU is unused
      MOV    : o_data = i_data2;
      JUMP   : o_data = 16'h0000; // ALU is unused
      LOADI  : o_data = 16'h0000; // ALU is unused
      PUSH_POP : o_data = 16'h0000; // ALU is unused
      CALL_RET : o_data = 16'h0000; // ALU is unused
      LOADA  : o_data = 16'h0000; // ALU is unused
      HALT   : o_data = 16'h0000; // ALU is unused
      NOP    : o_data = 16'h0000; // ALU is unused
    endcase
  end

endmodule
