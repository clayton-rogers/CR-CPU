`default_nettype none

module cpu (
  input i_clk,
  output [7:0] cpu_output
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
 gpio #(.OFFSET(16'h8100))
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
  timer #(.OFFSET(16'h8200))
  timer(
    .i_clk(i_clk),
    .read_addr(read_addr),
    .read_data(read_data),
    .write_addr(write_addr),
    .write_data(write_data),
    .write_strobe(write_strobe)
    );

endmodule
