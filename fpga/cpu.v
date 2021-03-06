`default_nettype none

module cpu (
  input i_clk,
  output [7:0] cpu_output,
  input        uart_rx_unsafe,
  output       uart_tx,
  output wire vga_red,
  output wire vga_green,
  output wire vga_blue,
  output wire vga_v_sync,
  output wire vga_h_sync
  );
  parameter FILENAME = "top.hex";

  // main bus of cpu
  wire [15:0] read_addr;
  wire [15:0] read_data;
  wire [15:0] write_addr;
  wire [15:0] write_data;
  wire        write_strobe;

  // core logic
  core core(
    .i_clk(i_clk),
    .read_addr(read_addr),
    .read_data(read_data),
    .write_addr(write_addr),
    .write_data(write_data),
    .write_strobe(write_strobe)
    );

  // cpu ram
  ram #(.FILENAME(FILENAME))
  ram(
    .i_clk(i_clk),
    .read_addr(read_addr),
    .read_data(read_data),
    .write_addr(write_addr),
    .write_data(write_data),
    .write_strobe(write_strobe)
    );

 // IO pins
 gpio #(.BASE_ADDR(16'h8100))
 gpio(
   .i_clk(i_clk),
   .read_addr(read_addr),
   .read_data(read_data),
   .write_addr(write_addr),
   .write_data(write_data),
   .write_strobe(write_strobe),
   .output_pins(cpu_output)
   );

  // 32bit timer
  timer #(.BASE_ADDR(16'h8200))
  timer(
    .i_clk(i_clk),
    .read_addr(read_addr),
    .read_data(read_data),
    .write_addr(write_addr),
    .write_data(write_data),
    .write_strobe(write_strobe)
    );

  // Duplex uart peripheral
  uart_peripheral #(.BASE_ADDR(16'h8300))
  uart (
    .i_clk(i_clk),
    .read_addr(read_addr),
    .read_data(read_data),
    .write_addr(write_addr),
    .write_data(write_data),
    .write_strobe(write_strobe),
    .i_rx_unsafe(uart_rx_unsafe),
    .o_tx(uart_tx)
    );
  
  // character based VGA adaptor
  vga_peripheral #(.BASE_ADDR(16'h8400))
  vga (
    .i_clk(i_clk),
    .read_addr(read_addr),
    .read_data(read_data),
    .write_addr(write_addr),
    .write_data(write_data),
    .write_strobe(write_strobe),
    .red(vga_red),
    .green(vga_green),
    .blue(vga_blue),
    .v_sync(vga_v_sync),
    .h_sync(vga_h_sync)
  );

endmodule
