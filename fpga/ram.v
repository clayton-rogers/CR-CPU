`default_nettype none

module ram (i_clk, i_load, i_addr, i_data, o_data);
parameter ADDR_WIDTH = 8;
parameter FILENAME = "empty.hex";
  input i_clk;
  input i_load;
  input [15:0] i_addr;
  input [15:0] i_data;
  output reg [15:0] o_data;

  /* verilator lint_off UNUSED */
  wire [(16-ADDR_WIDTH-1):0] unused_addr = i_addr[15:(16-ADDR_WIDTH)]; // some bits are unused
  /* verilator lint_on UNUSED */

  reg [15:0] memory [(1<<ADDR_WIDTH)-1:0];
  initial $readmemh(FILENAME, memory);

always @ (posedge i_clk) begin
  if (i_load) begin
    memory[i_addr[(ADDR_WIDTH-1):0]] <= i_data;
  end
  o_data <= memory[i_addr[(ADDR_WIDTH-1):0]];
end

endmodule
