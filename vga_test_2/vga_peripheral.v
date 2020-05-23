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

  localparam MAX_COUNT = 5; // TODO put back to 40000000
  reg [26:0] sub_count = 0;
  reg [10:0] seconds = 0;
  always @ ( posedge CLK ) begin
    sub_count <= sub_count + 1;
    if (seconds == 1920-1) begin
      seconds <= 0;
    end
    if (sub_count == MAX_COUNT-1) begin
      sub_count <= 0;
      seconds <= seconds + 1;
    end
  end


  // 0 = blank
  // 1 = M
  // 2 = solid
  // 3 = box
  // 4 = 9
  always @ ( posedge CLK ) begin
    write_strobe <= 1'b1;
    char_addr <= seconds;
    if (seconds == 1920-1) begin
      write_char <= write_char + 1;
      if (write_char == 4) begin
        write_char <= 0;
      end
    end
  end

endmodule
