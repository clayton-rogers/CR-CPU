`default_nettype none

module vga_peripheral (
  input wire CLK,
  output wire red,
  output wire green,
  output wire blue,
  output wire v_sync,
  output wire h_sync
  );

  reg [7:0]  write_char = 8'h00;
  reg [10:0] char_addr = 11'h000;
  reg        write_strobe = 1'b0;

  vga vga_inst (
        .CLK(CLK),
        .red(red),
        .green(green),
        .blue(blue),
        .v_sync(v_sync),
        .h_sync(h_sync),
        .write_char(write_char),
        .write_char_pos(char_addr),
        .write_char_strobe(write_strobe)
        );

  localparam CLK_PER_HALF_HZ = 20000000;
  reg [25:0] counter = 26'h0000000;
  always @ (posedge CLK) begin
    counter <= counter + 1;
    if (counter == CLK_PER_HALF_HZ-1) begin
      counter <= 0;
    end
  end

  wire draw_char = counter == 1;

  always @ (posedge CLK) begin
    write_strobe <= 1'b0;
    if (draw_char) begin
      write_strobe <= 1'b1;
      write_char <= write_char + 1;
      char_addr <= char_addr + 1;
    end
  end

endmodule
