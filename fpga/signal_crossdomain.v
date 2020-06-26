`default_nettype none

module signal_crossdomain (
    input  wire i_clk_a,
    input  wire i_signal,
    input  wire i_clk_b,
    output wire o_signal
);

reg [1:0] intermediate;
always @ ( posedge i_clk_b ) begin
    intermediate[0] <= i_signal;
    intermediate[1] <= intermediate[0];
end

assign o_signal = intermediate[1];

endmodule
