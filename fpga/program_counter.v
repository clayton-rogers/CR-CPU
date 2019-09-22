`default_nettype none

module program_counter (i_clk, i_inc, i_load, i_addr, o_addr, o_instruction);
parameter ADDR_WIDTH = 8;
parameter PROGRAM_FILENAME = "program.hex";
  input i_clk;
  input i_inc;
  input i_load;
  input [15:0] i_addr;
  output [15:0] o_addr;
  output [15:0] o_instruction;

  reg last_was_load = 1'b1;

  // If the PC was loaded on the last cycle, then it takes an additional
  // cycle to load the next instruction from ram, so mask the output with
  // a copy of the last instruction
  assign o_instruction =  (last_was_load) ? last_instruction : ram_out;
  assign o_addr = instruction_address - 1;

reg [15:0] instruction_address = 0;
wire [15:0] ram_out;
ram #(.ADDR_WIDTH(ADDR_WIDTH), .FILENAME(PROGRAM_FILENAME)) ram (
  .i_clk(i_clk),
  .i_load(1'b0),
  .i_addr(instruction_address),
  .i_data(16'h0000),
  .o_data(ram_out));

always @ ( posedge i_clk ) begin
  if (i_load) begin
    instruction_address <= i_addr;
  end else if (i_inc) begin
    instruction_address <= instruction_address + 1;
  end
end

always @ ( posedge i_clk ) begin
  case (last_was_load)
  1'b0: last_was_load <= i_load || !i_inc;
  1'b1: last_was_load <= 1'b0;
  endcase
end

reg [15:0] last_instruction = 16'hFFFF;
always @ ( posedge i_clk ) begin
  last_instruction <= o_instruction;
end

endmodule
