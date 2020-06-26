`default_nettype none

// This is the ramblock that stores the pixel maps for each character
// It takes 1 clock to get the data out
module bitmap_ram (
  input  wire       CLK,
  input  wire [6:0] character, // only lower 128 ascii is supported
  input  wire [2:0] x,
  input  wire [3:0] y,
  output reg        pixel,

  input wire  [7:0] write_data,
  input wire [10:0] write_addr,
  input wire        write_strobe
);
  parameter BITMAP_FILENAME = "font_bitmap.hex";

  reg [7:0] mem [2048-1:0];
  initial $readmemh(BITMAP_FILENAME, mem);

  // Write
  always @ ( posedge CLK ) begin
    if (write_strobe) begin
      mem[write_addr] = write_data;
    end
  end

  // Read
  wire [10:0] read_addr = { character, y };
  always @ ( posedge CLK ) begin
    pixel <= mem[read_addr][x];
  end

endmodule
