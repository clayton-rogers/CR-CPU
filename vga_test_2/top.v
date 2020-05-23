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
                       
  wire red = 1'b0;
  wire green = 1'b0;
  wire blue = 1'b0;
  wire v_sync;
  wire h_sync;
  wire on_screen;
  
  sync_gen sync_gen_inst (         
          .CLK(CLK),
          .v_sync(v_sync),
          .h_sync(h_sync),
          .on_screen(on_screen));
  
endmodule
