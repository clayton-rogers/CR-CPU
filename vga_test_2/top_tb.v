`default_nettype none
`define DUMPSTR(x) `"x.vcd`"
`timescale 1 ns / 100 ps


module top_tb ();

reg clk = 1'b0;
always #0.5 clk = ~clk;

wire v_sync;
wire h_sync;
wire on_screen;
sync_gen sync_gen_inst (         
          .CLK(clk),
          .v_sync(v_sync),
          .h_sync(h_sync),
          .on_screen(on_screen));

char_counter char_counter_inst (
  .CLK(clk));


initial begin
$dumpfile(`DUMPSTR(`VCD_OUTPUT));
$dumpvars(0, top_tb);

// This is to make the first clock tick at t + 1
// This makes it easier to test modules where you want to verify the iniitial state
#1000000

$display("end of simulation");
$finish;
end


endmodule // top_tb
