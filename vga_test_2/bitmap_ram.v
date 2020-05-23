`default_nettype none

// This is the ramblock that stores the pixel maps for each character
// It takes 1 clock to get the data out
module bitmap_ram (
  input wire        CLK,
  input wire [6:0] character, // only lower 128 ascii is supported
  input wire [2:0] x,
  input wire [3:0] y,
  output reg       pixel = 1'b0,

  input wire  [7:0] write_data,
  input wire [10:0] write_addr,
  input wire        write_strobe
);
  //parameter BITMAP_FILENAME = "";

  reg [7:0] mem [2048-1:0];
  //initial $readmemh(BITMAP_FILENAME, mem);

  initial begin
    mem[0]  = 8'b00000000;
    mem[1]  = 8'b00000000;
    mem[2]  = 8'b00111100;
    mem[3]  = 8'b01111110;
    mem[4]  = 8'b11100111;
    mem[5]  = 8'b11000011;
    mem[6]  = 8'b11100011;
    mem[7]  = 8'b11111111;
    mem[8]  = 8'b11011110;
    mem[9]  = 8'b11100000;
    mem[10] = 8'b01100010;
    mem[11] = 8'b01111111;
    mem[12] = 8'b00111110;
    mem[13] = 8'b00000000;
    mem[14] = 8'b00000000;
    mem[15] = 8'b00000000;

    mem[16] = 8'b00000000;
    mem[17] = 8'b00000000;
    mem[18] = 8'b11100111;
    mem[19] = 8'b11100111;
    mem[20] = 8'b11100111;
    mem[21] = 8'b11101111;
    mem[22] = 8'b11111111;
    mem[23] = 8'b11111011;
    mem[24] = 8'b11011011;
    mem[25] = 8'b11011011;
    mem[26] = 8'b11000011;
    mem[27] = 8'b11000011;
    mem[28] = 8'b11000011;
    mem[29] = 8'b00000000;
    mem[30] = 8'b00000000;
    mem[31] = 8'b00000000;
  end

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