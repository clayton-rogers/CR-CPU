`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_3, // GPIO MSB
    output PIN_4,
    output PIN_5,
    output PIN_6,
    output PIN_7,
    output PIN_8,
    output PIN_9,
    output PIN_10, // GPIO LSB

    input  PIN_24, // uart rx
    output PIN_23, // uart tx

    output PIN_22, // h_sync
    output PIN_21, // v_sync
    output PIN_20, // red
    output PIN_19, // green
    output PIN_18  // blue

    );

    localparam INPUT_CLOCK     = 16_000_000;
    localparam OUTPUT_CLOCK    = 10;

    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;
    // turn off LED
    assign LED = 1'b0;

    // Setup clock
    reg real_clock = 0;

    // Slow down the clock to one second.
    reg [31:0] count = 0;
    always @ ( posedge CLK ) begin
      if (count < (INPUT_CLOCK / (2 * OUTPUT_CLOCK))) begin
        count <= count + 1;
      end else begin
        count <= 0;
        real_clock <= ~real_clock;
      end
    end

    // instantiate CPU
    wire [7:0] core_output;
    assign {PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9, PIN_10} = core_output;
    cpu cpu (
      .i_clk(CLK),
      .cpu_output(core_output),
      .uart_rx_unsafe(PIN_24),
      .uart_tx(PIN_23),
      .vga_red(PIN_20),
      .vga_green(PIN_19),
      .vga_blue(PIN_18),
      .vga_v_sync(PIN_21),
      .vga_h_sync(PIN_22)
    );

endmodule
