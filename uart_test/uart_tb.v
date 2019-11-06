`default_nettype none
`define DUMPSTR(x) `"x.vcd`"
`timescale 100 ns / 10 ns

module uart_tb ();

reg clk = 1'b0;
always #0.5 clk = ~clk;

wire uart_out;
reg uart_in = 1'b1;

wire [7:0] uart_rx_data;
reg        uart_rx_enable = 1'b0;
reg  [7:0] uart_tx_data = 8'h00;
reg        uart_tx_enable = 1'b0;
uart uart(
  .i_clk(clk),
  .o_read_data(uart_rx_data),
  .i_read_enable(uart_rx_enable),
  .i_write_data(uart_tx_data),
  .i_write_enable(uart_tx_enable),
  .o_tx(uart_out),
  .i_rx_unsafe(uart_in)
);

initial begin
$dumpfile(`DUMPSTR(`VCD_OUTPUT));
$dumpvars(0, uart_tb);

#123 // random delay
// 139 is the number of 16MHz clocks in 115200 baud
uart_in = 0;
#139 // start
// 00101010 0x54 'T'
uart_in = 0;
#139 // LSB
uart_in = 0;
#139
uart_in = 1;
#139
uart_in = 0;
#139
uart_in = 1;
#139
uart_in = 0;
#139
uart_in = 1;
#139
uart_in = 0;
#139 // MSB
uart_in = 1;
#139 // stop bit

#50 // wait a bit
uart_rx_enable = 1'b1;
#1
uart_rx_enable = 1'b0; // read the received data

// Send one char
#50 // wait a bit
uart_tx_data = 8'hA5;
uart_tx_enable = 1'b1;
#1
uart_tx_enable = 1'b0;
#1400 // wait for char to be sent

// Send two char at same time
uart_tx_data = 8'hFF;
uart_tx_enable = 1'b1;
#1
uart_tx_data = 8'h11;
#1
uart_tx_enable = 1'b0;
#5000 // wait for char to be sent

#100

$display("end of simulation");
$finish;
end


endmodule // top_tb
