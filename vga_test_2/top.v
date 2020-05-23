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

  wire red = nb2 && character2[0];
  wire green = nb2 && character2[1];
  wire blue = nb2 && character2[2];
  wire v_sync;
  wire h_sync;
  wire on_screen;

  sync_gen sync_gen_inst (
          .CLK(CLK),
          .v_sync(v_sync),
          .h_sync(h_sync),
          .on_screen(on_screen));

  wire [10:0] character;
  wire [3:0]  x;
  wire [4:0]  y;
  wire        n_blank;

  // temp delay
  reg nb1;
  reg nb2;
  wire [10:0] character1;
  wire [10:0] character2;
  always @ ( posedge CLK ) begin
    nb1 <= n_blank && x[0] && y[0];
    nb2 <= nb1;

    character1 <= character;
    character2 <= character1;
  end
  char_counter char_counter_inst (
          .CLK(CLK),
          .character(character),
          .x(x),
          .y(y),
          .n_blank(n_blank));

endmodule
