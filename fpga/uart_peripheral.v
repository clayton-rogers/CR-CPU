`default_nettype none

module uart_peripheral (
  input i_clk,

  input  wire [15:0] read_addr,
  output wire [15:0] read_data,

  input wire [15:0] write_addr,
  input wire [15:0] write_data,
  input wire        write_strobe,

  input i_rx_unsafe,
  output o_tx //
);
parameter BASE_ADDR = 16'h8300;

// Memory map for this peripheral
localparam TX_BUFFER_SIZE_OFFSET = 0;
localparam RX_BUFFER_SIZE_OFFSET = 1;
localparam TX_WRITE_OFFSET       = 2;
localparam RX_READ_OFFSET        = 3;

wire should_read_data = (read_addr == BASE_ADDR + RX_READ_OFFSET);
wire should_read_TX_BUFF = (read_addr == BASE_ADDR + TX_BUFFER_SIZE_OFFSET);
wire should_read_RX_BUFF = (read_addr == BASE_ADDR + RX_BUFFER_SIZE_OFFSET);
wire should_write = (write_strobe && write_addr == BASE_ADDR + TX_WRITE_OFFSET);
wire [7:0] uart_rx_data;

reg read_uart_last_clock = 1'b0;
reg read_tx_buff_last_clock = 1'b0;
reg read_rx_buff_last_clock = 1'b0;
always @ ( posedge i_clk ) begin
  read_uart_last_clock <= should_read_data;
  read_tx_buff_last_clock <= should_read_TX_BUFF;
  read_rx_buff_last_clock <= should_read_RX_BUFF;
end

reg [7:0] maybe_output_data;
always @ ( * ) begin
  if (read_uart_last_clock) begin
    maybe_output_data = uart_rx_data;
  end else if (read_tx_buff_last_clock) begin
    maybe_output_data = tx_buffer_size;
  end else if (read_rx_buff_last_clock) begin
    maybe_output_data = rx_buffer_size;
  end else begin
    maybe_output_data = 8'h00; // doesn't matter not used
  end
end

assign read_data = (read_uart_last_clock || read_tx_buff_last_clock || read_rx_buff_last_clock) ?
  {8'h00, maybe_output_data} : 16'bzzzzzzzzzzzzzzzz;

wire [7:0] rx_buffer_size;
wire [7:0] tx_buffer_size;

uart uart (
    .i_clk(i_clk),    // 16MHz clock

    .o_read_data(uart_rx_data),
    .i_read_enable(should_read_data),
    .i_write_data(write_data[7:0]),
    .i_write_enable(should_write),

    .rx_buffer_size(rx_buffer_size),
    .tx_buffer_size(tx_buffer_size),

    .o_tx(o_tx),
    .i_rx_unsafe(i_rx_unsafe)
);

/* verilator lint_off UNUSED */
wire [7:0] unused_write_data_high = write_data[15:8];
/* verilator lint_on UNUSED */

endmodule
