`default_nettype none

// This is the actual char array that the screen data is stored in.
// It takes 1 clock to get the data out.
module char_ram (
  input wire        CLK,
  input wire [10:0] character_pos,
  output reg  [7:0] character = 8'h00, // expected to be ascii

  input wire [10:0] write_character_pos,
  input wire  [7:0] write_character,
  input wire        write_strobe
);

  reg [7:0] mem [2048-1:0];

  // Write
  always @ ( posedge CLK ) begin
    if (write_strobe) begin
      mem[write_character_pos] <= write_character;
    end
  end

  // Read
  always @ ( posedge CLK ) begin
    character <= mem[character_pos];
  end

endmodule
