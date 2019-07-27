`default_nettype none
`define DUMPSTR(x) `"x.vcd`"
`timescale 100 ns / 10 ns

`include "register.test"
`include "ram.test"
`include "program_counter.test"
`include "core.test"
`include "ALU.test"

module top_tb ();

reg clk = 1'b0;
reg start = 1'b0;
always #0.5 clk = ~clk & start;

wire reg_result;
wire reg_done;
register_test register (.clk(clk), .result_out(reg_result), .done(reg_done));

wire ram_result;
wire ram_done;
ram_test ram(.clk(clk), .result_out(ram_result), .done(ram_done));

wire pc_result;
wire pc_done;
program_counter_test program_counter(.clk(clk), .result_out(pc_result), .done(pc_done));

wire core_result;
wire core_done;
core_test core(.clk(clk), .result_out(core_result), .done(core_done));

wire alu_result;
wire alu_done;
alu_test alu(.clk(clk), .result_out(alu_result), .done(alu_done));

wire result = reg_result | ram_result | pc_result | core_result | alu_result;
wire done = reg_done & ram_done & pc_done & core_done & alu_done;

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
