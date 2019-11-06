`default_nettype none

module DFF (
    input i_clk,
    input i_in,
    output o_out //
);

reg FF = 1'b1;
reg FF2 = 1'b1;

assign o_out = FF2;

always @ ( posedge i_clk ) begin
  FF <= i_in;
  FF2 <= FF;
end

endmodule
