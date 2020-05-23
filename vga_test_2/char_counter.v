`default_nettype none

// Given the pixel clock, calculates which character is currently being draw
// and which x and y coordinate of that character. Zero is in the top left.

// Also provides a blanking signal since the character is assumed to be smaller
// than the space it will be draw.

// Also provides an ajustable anti-delay since it is assumed it will take
// several clocks to get the character and pixel maps
module char_counter (
    input  wire       CLK,
    output reg [10:0] character = 0, // 0 .. 1920 (80*24)
    output reg [3:0]  x = ADVANCE,   // 0 .. 7    font is 8x16
    output reg [4:0]  y = 0,         // 0 .. 15
    output wire       n_blank    // high when character should be displayed
    );

  parameter ADVANCE = 2; // give the rest of the code 2 clocks to get the pixel

  parameter SCREEN_WIDTH = 800;
  parameter LINE_LENGTH = 1056;

  parameter SCREEN_HEIGHT= 600;
  parameter NUMBER_LINES = 628;

  reg [15:0] pixel_counter = ADVANCE;
  always @ ( posedge CLK ) begin
    pixel_counter <= pixel_counter + 1;
    if (pixel_counter == LINE_LENGTH-1) begin
      pixel_counter <= 0;
    end
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

  // The font is 8x16, but the space for each character is 10x25
  // This give 2 pixels after each character, and 8 pixels under each line.
  parameter FONT_X = 8;
  parameter FONT_Y = 16;
  parameter HOLE_X = 10;
  parameter HOLE_Y = 25;

  always @ ( posedge CLK ) begin
    x <= x + 1;
    if (x == HOLE_X-1) begin
      x <= 0;
    end
    if (pixel_counter == LINE_LENGTH-1) begin
      x <= 0;
    end
  end
  always @ ( posedge CLK ) begin
    if (pixel_counter == LINE_LENGTH-1) begin
      y <= y + 1;
      if (y == HOLE_Y-1) begin
        y <= 0;
      end
      if (line_counter == NUMBER_LINES-1) begin
        y <= 0;
      end
    end
  end

  // If inside the font, then display
  // Note that whether the signal is on screen is decided at a higher layer
  assign n_blank = x < FONT_X
      && y < FONT_Y
      && (pixel_counter < SCREEN_WIDTH)
      && (line_counter < SCREEN_HEIGHT);

  // Increment the character when we get to the next on.
  // Sub a lines worth of chars when we get to a line,
  // except when it's the end of a character line, then add a line.
  // Reset to zero if we're at then end of the screen.
  reg last_n_blank = 1'b0;
  always @ ( posedge CLK ) begin
    last_n_blank <= n_blank;
  end

  localparam CHAR_PER_LINE = SCREEN_WIDTH / HOLE_X; // should be 80

  always @ ( posedge CLK ) begin
    // On the falling edge of n_blank
    if (last_n_blank && !n_blank) begin
      character <= character + 1;
    end
    if (pixel_counter == SCREEN_WIDTH && y < FONT_Y) begin
      character <= character - CHAR_PER_LINE;
    end
    if (pixel_counter == SCREEN_WIDTH && y == FONT_Y) begin
      character <= character + CHAR_PER_LINE;
    end
    if (line_counter == NUMBER_LINES-1) begin
      character <= 0;
    end
  end

endmodule
