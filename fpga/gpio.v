`default_nettype none

module gpio (
  input i_clk,

  input  wire [15:0] read_addr,
  output wire  [15:0] read_data,

  input wire [15:0] write_addr,
  input wire [15:0] write_data,
  input wire        write_strobe,

  output reg [7:0] output_pins = 8'h00
  );
parameter BASE_ADDR = 16'h8100;

// GPIO module does not have any output right now.
// The following is a hack to get around not being able to constant assign
// an output as tri-state.
reg output_valid = 1'b0;
assign read_data = (output_valid) ? 16'hFFFF : 16'bzzzzzzzzzzzzzzzz;

/* verilator lint_off UNUSED */
wire [15:0] unused_read_addr = read_addr;
wire [15:0] unused_read_data = read_data;
wire [7:0] unused_write_data = write_data[15:8];
/* verilator lint_on UNUSED */

always @ ( posedge i_clk ) begin
  if (write_addr == BASE_ADDR + 1 && write_strobe == 1)
    output_pins <= write_data[7:0];
  else
    output_pins <= output_pins;
end

endmodule
