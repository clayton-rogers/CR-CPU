`default_nettype none

// ============================================================================
// ram multiplexer
//   Allows two devices to connect to the same ram. Device A will take priority
//   in cases where both devices want to use the ram at the same time. Note
//   that because device A has priority, it will always receive an ack on the
//   next clock. Thus it may be designed to assume that it has full control
//   over the ram.
//
//   The ram is shared between the two devices such that device a is reading
//   from the low half and writing to the high half. Whereas device b is the
//   opposite. This allows the two devices to queue full dupex data to
//   communicate with each other.
//
//   When a device wants to read, it sets the read address and the RE (read
//   enable). If it receives an ack on the next clock, then the read data is
//   for it. If it does not receive an ack then, the read data is for the other
//   device. In this case it should keep RE high until it receives an ack.
//
//   When a device wants to write, it sets the write address and data, and the
//   WE (write enable). If it recieves an ack on the next clock, then its write
//   was accepted and to may release the write enable. Otherwise, the device
//   should keep holding WE until it receives an ack.
// ============================================================================
module uart_ram_multiplexer (
  input i_clk,

  /// RAM connection
  output reg [8:0] o_ram_r_addr,
  input      [7:0] i_ram_r_data,
  output reg [8:0] o_ram_w_addr,
  output reg [7:0] o_ram_w_data,
  output reg       o_ram_we,

  /// Device A connection
  input      [7:0] i_a_r_addr,
  output     [7:0] o_a_r_data,
  input            i_a_re,
  output reg       o_a_ack_r = 1'b0,
  input      [7:0] i_a_w_addr,
  input      [7:0] i_a_w_data,
  input            i_a_we,
  output reg       o_a_ack_w,

  /// Device B connection
  input      [7:0] i_b_r_addr,
  output     [7:0] o_b_r_data,
  input            i_b_re,
  output reg       o_b_ack_r = 1'b0,
  input      [7:0] i_b_w_addr,
  input      [7:0] i_b_w_data,
  input            i_b_we,
  output reg       o_b_ack_w //
);

localparam SEL_A = 1'b0;
localparam SEL_B = 1'b1;
// ============================================================================
// READ
// ============================================================================
reg r_sel;
// Determine which device to service
always @ ( * ) begin
  if (i_a_re) begin
    r_sel = SEL_A;
  end else begin
    r_sel = SEL_B;
  end
end

// Ack is given next clock
always @ ( posedge i_clk ) begin
  o_a_ack_r <= i_a_re && r_sel == SEL_A;
  o_b_ack_r <= i_b_re && r_sel == SEL_B;
end

// Set actual address to ram
always @ ( * ) begin
  case (r_sel)
    SEL_A: begin
      o_ram_r_addr = {1'b0, i_a_r_addr};
    end
    SEL_B: begin
      o_ram_r_addr = {1'b1, i_b_r_addr};
    end
  endcase
end

// Both devices receive the same data, the ack tells them whether it is for them.
assign o_a_r_data = i_ram_r_data;
assign o_b_r_data = i_ram_r_data;

// ============================================================================
// WRITE
// ============================================================================
reg w_sel;
// Determine which device to service
always @ ( * ) begin
  if (i_a_we) begin
    w_sel = SEL_A;
  end else begin
    w_sel = SEL_B;
  end
end

// Ack is given this clock
always @ ( * ) begin
  o_a_ack_w = i_a_we && w_sel == SEL_A;
  o_b_ack_w = i_b_we && w_sel == SEL_B;
end

always @ ( * ) begin
  case (w_sel)
    SEL_A: begin
      o_ram_w_addr = {1'b1, i_a_w_addr};
      o_ram_w_data = i_a_w_data;
      o_ram_we     = i_a_we;
    end
    SEL_B: begin
      o_ram_w_addr = {1'b0, i_b_w_addr};
      o_ram_w_data = i_b_w_data;
      o_ram_we     = i_b_we;
    end
  endcase
end

endmodule
