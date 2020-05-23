`default_nettype none

// Given the pixel clock, calculates the vertical and horizontal sync pulses
// required for any vga signal. Also provides an on_screen signal which is true
// when the current pixel is within bounds of the screen.
module sync_gen (
          input  wire CLK,
          output reg  v_sync = 1'b0,
          output reg  h_sync = 1'b0,
          output wire on_screen);

  parameter SCREEN_WIDTH = 800;
  parameter LINE_LENGTH = 1056;
  parameter H_FRONTPORCH = 40;
  parameter H_SYNC_SIZE = 128;
  parameter H_SYNC_POLARITY = 1'b1;

  localparam H_SYNC_START = SCREEN_WIDTH + H_FRONTPORCH;

  parameter SCREEN_HEIGHT= 600;
  parameter NUMBER_LINES = 628;
  parameter V_FRONTPORCH = 1;
  parameter V_SYNC_SIZE  = 4;
  parameter V_SYNC_POLARITY = 1'b1;

  localparam V_SYNC_START = SCREEN_HEIGHT + V_FRONTPORCH;


  reg [15:0] pixel_counter = 0;

  always @ ( posedge CLK ) begin
    pixel_counter <= pixel_counter + 1;
    if (pixel_counter == LINE_LENGTH-1) begin
      pixel_counter <= 0;
    end
  end

  always @ ( posedge CLK ) begin
    if (pixel_counter == H_SYNC_START-1)
      h_sync <= H_SYNC_POLARITY;
    if (pixel_counter == (H_SYNC_START+H_SYNC_SIZE)-1)
      h_sync <= ~H_SYNC_POLARITY;
  end

  reg [15:0] line_counter = 0;

  always @ ( posedge CLK ) begin
    if (pixel_counter == LINE_LENGTH-1) begin
      line_counter <= line_counter + 1;
      if (line_counter == NUMBER_LINES-1) begin
        line_counter <= 0;
      end
    end
  end
  
  // Technically v_sync is firing 1 clock too late,
  // hopefully it shouldn't matter since the pulse size is
  // thousands of clocks
  always @ ( posedge CLK ) begin
    if (line_counter == (V_SYNC_START))
      v_sync <= V_SYNC_POLARITY;
    if (line_counter == (V_SYNC_START+V_SYNC_SIZE))
      v_sync <= ~V_SYNC_POLARITY;
  end
  
  assign on_screen = (pixel_counter < SCREEN_WIDTH)
      && (line_counter < SCREEN_HEIGHT);

endmodule
