`default_nettype none

module uart_rx (
    input i_clk,
    input i_rx,
    output reg [7:0] data = 8'h00,
    output reg new_data = 1'b0,
    input ack_data //
);

parameter BAUD = 9_600;
parameter INPUT_CLOCK = 16_000_000;

localparam CLOCKS_PER_BAUD = INPUT_CLOCK / BAUD;


reg [12:0] counter = 0;
reg [3:0] state = 0;
localparam WAIT_FOR_START_BIT = 0;
localparam WAIT_FOR_BIT_0 = 1;
localparam WAIT_FOR_STOP_BIT = 9;


always @ ( posedge i_clk ) begin
  case (state)
  WAIT_FOR_START_BIT: begin
    if (i_rx == 1'b0) begin // if low
      counter <= counter + 1;
    end else begin
      counter <= 0;
    end

    // if it's been low for at lease half a cycle then
    // this is a start bit
    if (counter == CLOCKS_PER_BAUD/2) begin
      counter <= 0;
      state <= WAIT_FOR_BIT_0;
    end
  end
  default: begin
    counter <= counter + 1;

    // Check if it's time to sample again
    if (counter == CLOCKS_PER_BAUD) begin
      state <= state + 1;
      counter <= 0;
      if (state == WAIT_FOR_STOP_BIT) begin
        state <= WAIT_FOR_START_BIT;
        new_data <= 1'b1;
      end else begin
        data <= {i_rx, data[7:1]}; // shift in new data
      end
    end
  end
  endcase

  if (new_data && ack_data)
    new_data <= 1'b0;
end


endmodule
