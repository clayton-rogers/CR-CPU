`default_nettype none

// This is the ramblock that stores the pixel maps for each character
// It takes 1 clock to get the data out
module bitmap_ram (
  input  wire       CLK,
  input  wire [6:0] character, // only lower 128 ascii is supported
  input  wire [2:0] x,
  input  wire [3:0] y,
  output wire       pixel,

  input wire  [7:0] write_data,
  input wire [10:0] write_addr,
  input wire        write_strobe
);
  //parameter BITMAP_FILENAME = "";

  reg [7:0] mem [2048-1:0];
  //initial $readmemh(BITMAP_FILENAME, mem);

  // 0 = blank
  // 1 = M
  // 2 = solid
  // 3 = box
  // 4 = 9

  initial begin
   mem[0]  = 8'b00000000;
   mem[1]  = 8'b00000000;
   mem[2]  = 8'b00000000;
   mem[3]  = 8'b00000000;
   mem[4]  = 8'b00000000;
   mem[5]  = 8'b00000000;
   mem[6]  = 8'b00000000;
   mem[7]  = 8'b00000000;
   mem[8]  = 8'b00000000;
   mem[9]  = 8'b00000000;
   mem[10] = 8'b00000000;
   mem[11] = 8'b00000000;
   mem[12] = 8'b00000000;
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

   mem[32] = 8'b11111111;
   mem[33] = 8'b11111111;
   mem[34] = 8'b11111111;
   mem[35] = 8'b11111111;
   mem[36] = 8'b11111111;
   mem[37] = 8'b11111111;
   mem[38] = 8'b11111111;
   mem[39] = 8'b11111111;
   mem[40] = 8'b11111111;
   mem[41] = 8'b11111111;
   mem[42] = 8'b11111111;
   mem[43] = 8'b11111111;
   mem[44] = 8'b11111111;
   mem[45] = 8'b11111111;
   mem[46] = 8'b11111111;
   mem[47] = 8'b11111111;

   mem[48] = 8'b11111111;
   mem[49] = 8'b10000001;
   mem[50] = 8'b10000001;
   mem[51] = 8'b10000001;
   mem[52] = 8'b10000001;
   mem[53] = 8'b10000001;
   mem[54] = 8'b10000001;
   mem[55] = 8'b10000001;
   mem[56] = 8'b10000001;
   mem[57] = 8'b10000001;
   mem[58] = 8'b10000001;
   mem[59] = 8'b10000001;
   mem[60] = 8'b10000001;
   mem[61] = 8'b10000001;
   mem[62] = 8'b10000001;
   mem[63] = 8'b11111111;

   mem[64] = 8'b00000000;
   mem[65] = 8'b00000000;
   mem[66] = 8'b00111100;
   mem[67] = 8'b01111110;
   mem[68] = 8'b11100111;
   mem[69] = 8'b11000011;
   mem[70] = 8'b11100011;
   mem[71] = 8'b11111111;
   mem[72] = 8'b11011110;
   mem[73] = 8'b11100000;
   mem[74] = 8'b01100010;
   mem[75] = 8'b01111111;
   mem[76] = 8'b00111110;
   mem[77] = 8'b00000000;
   mem[78] = 8'b00000000;
   mem[79] = 8'b00000000;
  end

  // Write
  always @ ( posedge CLK ) begin
    if (write_strobe) begin
      mem[write_addr] = write_data;
    end
  end

  // Read
  reg pixel_out = 1'b0;
  wire [10:0] read_addr = { character, y };
  always @ ( posedge CLK ) begin
    pixel_out <= mem[read_addr][x];
  end

  assign pixel = pixel_out;

endmodule