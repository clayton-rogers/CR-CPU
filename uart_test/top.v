`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_14,
    input PIN_15 //
);

localparam INPUT_CLOCK     = 16_000_000;
localparam OUTPUT_CLOCK    = 115200;
localparam CLOCK_PER_BAUD  = INPUT_CLOCK / OUTPUT_CLOCK;
// drive USB pull-up resistor to '0' to disable USB
assign USBPU = 1'b0;
// turn on LED
assign LED = 1'b1;

wire [7:0] uart_rx_data;
reg        uart_rx_enable = 1'b0;
reg  [7:0] uart_tx_data   = 8'h00;
reg        uart_tx_enable = 1'b0;
uart uart (
 .i_clk(CLK),
 .o_read_data(uart_rx_data),
 .i_read_enable(uart_rx_enable),
 .i_write_data(uart_tx_data),
 .i_write_enable(uart_tx_enable),
 .o_tx(PIN_14),
 .i_rx_unsafe(PIN_15)
 );

endmodule
