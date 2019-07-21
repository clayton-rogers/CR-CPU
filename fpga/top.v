`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_13,
    output PIN_10
    );

    localparam INPUT_CLOCK     = 16_000_000;
    localparam OUTPUT_CLOCK    = 1;

    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;

    // Setup clock
    reg real_clock = 0;

    // Slow down the clock to one second.
    reg [31:0] count = 0;
    always @ ( posedge CLK ) begin
      if (count < (INPUT_CLOCK / 2 * OUTPUT_CLOCK)) begin
        count <= count + 1;
      end else begin
        count <= 0;
        real_clock <= ~real_clock;
      end
    end

    core core (.i_clk(real_clock), .o_led0(PIN_13), .o_led1(PIN_10), .o_led2(LED));

endmodule
