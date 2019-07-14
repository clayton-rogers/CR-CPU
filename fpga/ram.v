`default_nettype none

module ram (i_clk, i_load, i_addr, i_data, o_data);
parameter ADDR_WIDTH = 8;
  input i_clk;
  input i_load;
  input [ADDR_WIDTH-1:0] i_addr;
  input [15:0] i_data;
  output reg [15:0] o_data;

  reg [15:0] memory [(1<<ADDR_WIDTH)-1:0];

always @ (posedge i_clk) begin
  if (i_load) begin
    memory[i_addr] <= i_data;
  end
  o_data <= memory[i_addr];
end

endmodule
