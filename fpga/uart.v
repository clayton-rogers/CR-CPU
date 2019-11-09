`default_nettype none

module uart (
    input i_clk,    // 16MHz clock

    output [7:0] o_read_data,
    input        i_read_enable,
    input  [7:0] i_write_data,
    input        i_write_enable,

    output reg [7:0] rx_buffer_size = 8'h00,
    output reg [7:0] tx_buffer_size = 8'h00,

    output o_tx,
    input i_rx_unsafe ///
);

localparam INPUT_CLOCK     = 16_000_000;
localparam OUTPUT_CLOCK    = 115_200;
localparam CLOCK_PER_BAUD  = INPUT_CLOCK / OUTPUT_CLOCK;

wire rx_line;
DFF input_dff (
  .i_clk(i_clk),
  .i_in(i_rx_unsafe),
  .o_out(rx_line)
  );

wire [7:0] tx_data;
wire       tx_enable;
wire       tx_busy;
uart_tx #(
  .BAUD(OUTPUT_CLOCK),
  .INPUT_CLOCK(INPUT_CLOCK)
  ) tx (
  .i_clk(i_clk),
  .data(tx_data),
  .strobe(tx_enable),
  .o_tx(o_tx),
  .busy(tx_busy)
  );

wire [7:0] rx_data;
wire       rx_new_data;
wire       rx_ack;
uart_rx #(
  .BAUD(OUTPUT_CLOCK),
  .INPUT_CLOCK(INPUT_CLOCK)
  ) rx (
  .i_clk(i_clk),
  .i_rx(rx_line),
  .data(rx_data),
  .new_data(rx_new_data),
  .ack_data(rx_ack)
  );

wire [8:0] ram_read_addr;
wire [7:0] ram_read_data;
wire [8:0] ram_write_addr;
wire [7:0] ram_write_data;
wire       ram_write_enable;
uart_ram ram (
  .i_clk(i_clk),
  .read_addr(ram_read_addr),
  .read_data(ram_read_data),
  .write_addr(ram_write_addr),
  .write_data(ram_write_data),
  .write_strobe(ram_write_enable)
);

wire unused_read_ack;
wire unused_write_ack;
uart_ram_multiplexer multiplexer (
  .i_clk(i_clk),

  // RAM
  .o_ram_r_addr(ram_read_addr),
  .i_ram_r_data(ram_read_data),
  .o_ram_w_addr(ram_write_addr),
  .o_ram_w_data(ram_write_data),
  .o_ram_we(ram_write_enable),

  // Device A "user"
  .i_a_r_addr(user_read_addr),
  .o_a_r_data(o_read_data),
  .i_a_re(i_read_enable),
  .o_a_ack_r(unused_read_ack),
  .i_a_w_addr(user_write_addr),
  .i_a_w_data(i_write_data),
  .i_a_we(i_write_enable),
  .o_a_ack_w(unused_write_ack),

  // Device B "uart"
  .i_b_r_addr(uart_read_addr),
  .o_b_r_data(tx_data),
  .i_b_re(tx_we),
  .o_b_ack_r(tx_enable),
  .i_b_w_addr(uart_write_addr),
  .i_b_w_data(rx_data),
  .i_b_we(rx_new_data),
  .o_b_ack_w(rx_ack)
);

reg tx_we;
always @ ( * ) begin
  if (tx_buffer_size != 0 && !tx_busy && !tx_enable) begin
    tx_we = 1'b1;
  end else begin
    tx_we = 1'b0;
  end
end

reg [7:0] uart_read_addr = 8'h00;  // uart tx read here
reg [7:0] uart_write_addr = 8'h00; // uart rx write here
reg [7:0] user_read_addr = 8'h00;  // user read for rx
reg [7:0] user_write_addr = 8'h00; // user write for tx

always @ ( posedge i_clk ) begin
  // If we have sent a new character
  if (tx_we) begin
    uart_read_addr <= uart_read_addr + 1;
    tx_buffer_size <= tx_buffer_size - 1;
  end

  // If we have received a new character an successfully stored it
  if (rx_ack) begin
    uart_write_addr <= uart_write_addr + 1;
    rx_buffer_size <= rx_buffer_size + 1;
  end

  if (i_read_enable) begin
    user_read_addr <= user_read_addr + 1;
    rx_buffer_size <= rx_buffer_size - 1;
  end

  if (i_write_enable) begin
    user_write_addr <= user_write_addr + 1;
    tx_buffer_size <= tx_buffer_size + 1;
  end
end

endmodule
