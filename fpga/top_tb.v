`default_nettype none
`define DUMPSTR(x) `"x.vcd`"
`timescale 100 ns / 10 ns

`include "ALU.test"
`include "cpu.test"

module top_tb ();

reg clk = 1'b0;
reg start = 1'b0;
always #0.5 clk = ~clk & start;

wire alu_result;
wire alu_done;
alu_test alu_test(.clk(clk), .result_out(alu_result), .done(alu_done));

wire cpu_result;
wire cpu_done;
cpu_test cpu_test(.clk(clk), .result_out(cpu_result), .done(cpu_done));

wire result = alu_result | cpu_result;
wire done =  alu_done & cpu_done;

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
