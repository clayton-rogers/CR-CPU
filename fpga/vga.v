`default_nettype none

// This is the top-level manager for vga
module vga (
  input wire CLK,

  output wire red,
  output wire green,
  output wire blue,
  output wire v_sync,
  output wire h_sync,

  input wire [7:0]  write_char,
  input wire [10:0] write_char_pos,
  input wire        write_char_strobe
  );

  assign red = on_screen && pixel && nb2;
  assign green = red;
  assign blue = red;

  wire on_screen;

  sync_gen sync_gen_inst (
          .CLK(CLK),
          .v_sync(v_sync),
          .h_sync(h_sync),
          .on_screen(on_screen));

  wire [10:0] character_pos;
  wire [3:0]  x;
  wire [4:0]  y;
  wire        n_blank;
  reg nb1 = 1'b0;
  reg nb2 = 1'b0;

  // delay the blanking the same amount as the data
  reg [2:0] x1 = 3'b000;
  reg [3:0] y1 = 4'b0000;
  always @ ( posedge CLK ) begin
    // Delay blanking two clocks same as data
    nb1 <= n_blank;
    nb2 <= nb1;

    // Delay x and y one clock to skip over char_ram
    x1 <= x[2:0];
    y1 <= y[3:0];
  end

  char_counter char_counter_inst (
          .CLK(CLK),
          .character_pos(character_pos),
          .x(x),
          .y(y),
          .n_blank(n_blank));

  wire [7:0] character;
  char_ram char_ram_inst (
          .CLK(CLK),
          .character_pos(character_pos),
          .character(character),

          .write_character_pos(write_char_pos),
          .write_character(write_char),
          .write_strobe(write_char_strobe));

  wire pixel;
  bitmap_ram bitmap_ram_inst (
          .CLK(CLK),
          .character(character[6:0]),
          .x(x1[2:0]),
          .y(y1[3:0]),
          .pixel(pixel),

          .write_data(8'h00),
          .write_addr(11'h000),
          .write_strobe(1'b0));

  // verilator lint_off UNUSED
  wire [2:0] unused = { x[3], y[4], character[7] };
  // verilator lint_on UNUSED

endmodule
