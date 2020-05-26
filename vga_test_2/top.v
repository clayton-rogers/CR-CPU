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


  assign PIN_22 = red;
  assign PIN_21 = green;
  assign PIN_20 = blue;
  assign PIN_24 = h_sync;
  assign PIN_23 = v_sync;

  // drive USB pull-up resistor to '0' to disable USB
  assign USBPU = 0;
  assign LED = 1'b1;


  wire CLK;
  vga_pll vga_pll_inst (.REFERENCECLK(PIN_CLK),
                        .PLLOUTGLOBAL(CLK));

  wire red;
  wire green;
  wire blue;
  wire v_sync;
  wire h_sync;

  vga_peripheral vga_peripheral_inst (
          .CLK(CLK),
          .red(red),
          .green(green),
          .blue(blue),
          .v_sync(v_sync),
          .h_sync(h_sync)
          );

endmodule
