`default_nettype none

module program_counter (i_clk, i_inc, i_load, i_addr, o_addr, o_instruction);
parameter ADDR_WIDTH = 8;
parameter PROGRAM_FILENAME = "program.hex";
  input i_clk;
  input i_inc;
  input i_load;
  input [(ADDR_WIDTH-1):0] i_addr;
  output [(ADDR_WIDTH-1):0] o_addr;
  output [15:0] o_instruction;

  assign o_addr = instruction_address;

reg [(ADDR_WIDTH-1):0] instruction_address = 0;
ram #(.ADDR_WIDTH(ADDR_WIDTH), .FILENAME(PROGRAM_FILENAME)) ram (
  .i_clk(i_clk),
  .i_load(1'b0),
  .i_addr(instruction_address),
  .i_data(16'h0000),
  .o_data(o_instruction));

always @ ( posedge i_clk ) begin
  if (i_load) begin
    instruction_address <= i_addr;
  end else if (i_inc) begin
    instruction_address <= instruction_address + 1;
  end
end

endmodule
