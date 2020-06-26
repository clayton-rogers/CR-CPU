`default_nettype none

module vga_peripheral (
  input i_clk, // 16 MHz

  input  wire [15:0] read_addr,
  output wire [15:0] read_data,

  input wire [15:0] write_addr,
  input wire [15:0] write_data,
  input wire        write_strobe,

  output wire red,
  output wire green,
  output wire blue,
  output wire v_sync,
  output wire h_sync
  );
parameter BASE_ADDR = 16'h8400;

// Address map
// +0: char to write (auto increments the cursor)
// +1: set cursor position

wire clk; // 40 MHz
vga_pll vga_pll_inst (.REFERENCECLK(i_clk),
                      .PLLOUTGLOBAL(clk));

reg [7:0]  char_to_write = 8'h00;
reg [10:0] cursor_pos = 11'h000;
reg        vga_write = 1'b0;

always @ ( posedge i_clk ) begin // 16 MHz

  // If we wrote on the last cycle,
  // clear the flag and increment the pointer
  if (vga_write) begin
    vga_write <= 0;
    cursor_pos <= cursor_pos + 1;
  end

  // If we are writting to this device
  if (write_strobe == 1 && write_addr[15:4] == BASE_ADDR[15:4]) begin
    if (write_addr[0] == 1'b0) begin  // BASEADDR + 0
      vga_write <= 1;
      char_to_write <= write_data[7:0];
    end else begin            // BASEADDR + 1
      cursor_pos <= write_data[10:0];
    end
  end
end


// TODO temp only
assign red = 1'b0;
assign green = 1'b0;
assign blue = 1'b0;
assign v_sync = 1'b0;
assign h_sync = 1'b0;



// ==========================================================================
// Extra junk required for the cpu bus
// ==========================================================================
// VGA module does not have any output right now.
// The following is a hack to get around not being able to constant assign
// an output as tri-state.
reg output_valid = 1'b0;
assign read_data = (output_valid) ? 16'hFFFF : 16'bzzzzzzzzzzzzzzzz;

/* verilator lint_off UNUSED */
wire [15:0] unused_read_addr = read_addr;
wire [2:0] unused_write_addr = write_addr[3:1];
wire [4:0] unused_write_data = write_data[15:11];
/* verilator lint_on UNUSED */



endmodule
