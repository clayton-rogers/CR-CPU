`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_1,
    output PIN_3,
    output PIN_5,
    output PIN_7,
    output PIN_10,
    output PIN_11,
    output PIN_12,
    output PIN_13
    );

    localparam INPUT_CLOCK     = 16_000_000;
    localparam OUTPUT_CLOCK    = 100;

    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;

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

    /* verilator lint_off UNUSED */
    wire [15:0] core_output; // some bits are unused
    /* verilator lint_on UNUSED */
    core core (.i_clk(real_clock), .o_out(core_output));

    assign {PIN_13, PIN_12, PIN_11, PIN_10, PIN_1, PIN_3, PIN_5, PIN_7} = core_output[7:0];
    assign LED = 1'b1;
endmodule
