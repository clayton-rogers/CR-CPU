`default_nettype none

module register (i_clk, i_load, i_data, o_data);
  input i_clk;
  input i_load;
  input [15:0] i_data;
  output reg [15:0] o_data;

always @ (posedge i_clk) begin
  if (i_load) begin
    o_data <= i_data;
  end
end

endmodule
