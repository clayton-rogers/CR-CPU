`default_nettype none
`define DUMPSTR(x) `"x.vcd`"
`timescale 100 ns / 10 ns

`include "register.test"

module top_tb ();

reg clk = 1'b0;
reg start = 1'b0;
always #0.5 clk = ~clk & start;

wire reg_result;
wire reg_done;
register_test register (.clk(clk), .result_out(reg_result), .done(reg_done));

wire done = reg_done;
wire result = reg_result;

initial begin
$dumpfile(`DUMPSTR(`VCD_OUTPUT));
$dumpvars(0, top_tb);

// This is to make the first clock tick at t + 1
// This makes it easier to test modules where you want to verify the iniitial state
#1
start = 1'b1;

while (!done)
  #1

$display("end of simulation");
$finish;
end


endmodule // top_tb
