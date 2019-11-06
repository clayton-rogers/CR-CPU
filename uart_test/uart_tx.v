`default_nettype none

module uart_tx (
    input i_clk,
    input [7:0] data,
    input strobe,
    output o_tx,
    output reg busy = 1'b0 //
);

parameter BAUD = 9_600;
parameter INPUT_CLOCK = 16_000_000;

localparam CLOCKS_PER_BAUD = INPUT_CLOCK / BAUD;

reg [9:0] internal_data = 10'b1111111111;
reg [12:0] counter = 0;
reg [3:0] state = 0;

always @ ( posedge i_clk ) begin

  if (busy)
    counter <= counter + 1;

  if (counter == CLOCKS_PER_BAUD) begin
    internal_data <= {1'b1, internal_data[9:1]};
    state <= state - 1;
    counter <= 1;
  end

  if (state == 0) begin
    state <= 0;
    counter <= 0;
    busy <= 1'b0;
  end

  if (strobe && !busy) begin
    busy <= 1'b1;
    counter <= 1;
    // stop bit, data (MSB .. LSB), start bit
    internal_data <= {1'b1, data, 1'b0};
    // technically we only need ~10 states to transfer the data,
    // but having more causes us to idle between characters which
    // could help syncing.
    state <= 15;
  end

end

assign o_tx = internal_data[0];

endmodule
