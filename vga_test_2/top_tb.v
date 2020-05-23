`default_nettype none
`define DUMPSTR(x) `"x.vcd`"
`timescale 1 ns / 100 ps


module top_tb ();

reg clk = 1'b0;
always #0.5 clk = ~clk;

//wire v_sync;
//wire h_sync;
//wire on_screen;
//sync_gen sync_gen_inst (
//          .CLK(clk),
//          .v_sync(v_sync),
//          .h_sync(h_sync),
//          .on_screen(on_screen));
//
//char_counter char_counter_inst (
//  .CLK(clk));
//
//
//wire vga_red;
//wire vga_green;
//wire vga_blue;
//wire vga_v_sync;
//wire vga_h_sync;
//vga vga_inst (
//        .CLK(clk),
//        .red(vga_red),
//        .green(vga_green),
//        .blue(vga_blue),
//        .v_sync(vga_v_sync),
//        .h_sync(vga_h_sync));

wire peripheral_red;
wire peripheral_green;
wire peripheral_blue;
wire peripheral_v_sync;
wire peripheral_h_sync;
vga_peripheral vga_peripheral_inst (
        .CLK(clk),
        .red(peripheral_red),
        .green(peripheral_green),
        .blue(peripheral_blue),
        .v_sync(peripheral_v_sync),
        .h_sync(peripheral_h_sync));


initial begin
$dumpfile(`DUMPSTR(`VCD_OUTPUT));
$dumpvars(0, top_tb);

// This is to make the first clock tick at t + 1
// This makes it easier to test modules where you want to verify the iniitial state
#100000

$display("end of simulation");
$finish;
end


endmodule // top_tb
