`default_nettype none

module top (
  input PIN_CLK, // 16MHz clock
  output LED,    // User/boot LED next to power LED
  output USBPU,  // USB pull-up resistor

  output PIN_20,
  output PIN_21,
  output PIN_22,
  output PIN_23,
  output PIN_24
  );


  assign PIN_22 = red && on_screen;
  assign PIN_21 = green && on_screen;
  assign PIN_20 = blue && on_screen;
  assign PIN_24 = h_sync;
  assign PIN_23 = v_sync;

  // drive USB pull-up resistor to '0' to disable USB
  assign USBPU = 0;
  assign LED = 1'b1;


  wire CLK;
  vga_pll vga_pll_inst (.REFERENCECLK(PIN_CLK),
                        .PLLOUTGLOBAL(CLK));

  wire red = pixel && nb2;
  wire green = red;
  wire blue = red;
  wire v_sync;
  wire h_sync;
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
  always @ ( posedge CLK ) begin
    nb1 <= n_blank;
    nb2 <= nb1;
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

          .write_character_pos(11'h000),
          .write_character(8'h00),
          .write_strobe(1'b0));

  wire pixel;
  bitmap_ram bitmap_ram_inst (
          .CLK(CLK),
          .character(character[6:0]),
          .x(x[2:0]),
          .y(y[3:0]),
          .pixel(pixel),

          .write_data(8'h00),
          .write_addr(11'h000),
          .write_strobe(1'b0));

endmodule
