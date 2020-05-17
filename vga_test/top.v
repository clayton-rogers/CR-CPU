`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input PIN_CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_20,
    output PIN_21,
    output PIN_22,
    output PIN_23,
    output PIN_24
    );

    assign PIN_22 = red && on_screen;
    assign PIN_21 = green && on_screen;
    assign PIN_20 = blue && on_screen;
    assign PIN_24 = h_sync;
    assign PIN_23 = v_sync;

    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;
    assign LED = 1'b1;

    // To change output mode, only these parameters and the PLL need to be changed.
    // PLL output should be set to the pixel clock
    // See also http://tinyvga.com/vga-timing
    localparam SCREEN_WIDTH = 800;
    localparam LINE_LENGTH = 1056;
    localparam H_SYNC_START = SCREEN_WIDTH + 40;
    localparam H_SYNC_SIZE = 128;
    localparam H_SYNC_POLARITY = 1'b1;

    localparam SCREEN_HEIGHT= 600;
    localparam NUMBER_LINES = 628;
    localparam V_SYNC_START = SCREEN_HEIGHT + 1;
    localparam V_SYNC_SIZE  = 4;
    localparam V_SYNC_POLARITY = 1'b1;

    wire unused_clk;
    //wire CLK = PIN_CLK;
    wire CLK;
    top_pll top_pll_inst(.REFERENCECLK(PIN_CLK),
                         .PLLOUTCORE(unused_clk),
                         .PLLOUTGLOBAL(CLK),
                         .RESET(1'b1));

    wire on_screen = (pixel_counter < SCREEN_WIDTH) && (line_counter < SCREEN_HEIGHT);

    reg red;
    reg green;
    reg blue;
    reg h_sync = ~H_SYNC_POLARITY;
    reg v_sync = ~V_SYNC_POLARITY;

    reg [15:0] pixel_counter = 0;
    reg [15:0] next_pixel_counter = 1;

    always @ ( posedge CLK ) begin
      next_pixel_counter <= next_pixel_counter + 1;
      if (next_pixel_counter == LINE_LENGTH-1) begin
        next_pixel_counter <= 0;
      end

      pixel_counter <= next_pixel_counter;
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

    reg [7:0] data [15:0][7:0];
    initial begin
      data[0] [0] = 8'b00000000;
      data[1] [0] = 8'b00000000;
      data[2] [0] = 8'b00111100;
      data[3] [0] = 8'b01111110;
      data[4] [0] = 8'b11100111;
      data[5] [0] = 8'b11000011;
      data[6] [0] = 8'b11100011;
      data[7] [0] = 8'b11111111;
      data[8] [0] = 8'b11011110;
      data[9] [0] = 8'b11100000;
      data[10][0] = 8'b01100010;
      data[11][0] = 8'b01111111;
      data[12][0] = 8'b00111110;
      data[13][0] = 8'b00000000;
      data[14][0] = 8'b00000000;
      data[15][0] = 8'b00000000;

      data[0] [1] = 8'b00000000;
      data[1] [1] = 8'b00000000;
      data[2] [1] = 8'b11100111;
      data[3] [1] = 8'b11100111;
      data[4] [1] = 8'b11100111;
      data[5] [1] = 8'b11101111;
      data[6] [1] = 8'b11111111;
      data[7] [1] = 8'b11111011;
      data[8] [1] = 8'b11011011;
      data[9] [1] = 8'b11011011;
      data[10][1] = 8'b11000011;
      data[11][1] = 8'b11000011;
      data[12][1] = 8'b11000011;
      data[13][1] = 8'b00000000;
      data[14][1] = 8'b00000000;
      data[15][1] = 8'b00000000;

      data[0] [2] = 8'b00000000;
      data[1] [2] = 8'b00000000;
      data[2] [2] = 8'b11100111;
      data[3] [2] = 8'b11100111;
      data[4] [2] = 8'b11100111;
      data[5] [2] = 8'b11101111;
      data[6] [2] = 8'b11111111;
      data[7] [2] = 8'b11111011;
      data[8] [2] = 8'b11011011;
      data[9] [2] = 8'b11011011;
      data[10][2] = 8'b11000011;
      data[11][2] = 8'b11000011;
      data[12][2] = 8'b11000011;
      data[13][2] = 8'b00000000;
      data[14][2] = 8'b00000000;
      data[15][2] = 8'b00000000;

      data[0] [3] = 8'b00000000;
      data[1] [3] = 8'b00000000;
      data[2] [3] = 8'b11100111;
      data[3] [3] = 8'b11100111;
      data[4] [3] = 8'b11100111;
      data[5] [3] = 8'b11101111;
      data[6] [3] = 8'b11111111;
      data[7] [3] = 8'b11111011;
      data[8] [3] = 8'b11011011;
      data[9] [3] = 8'b11011011;
      data[10][3] = 8'b11000011;
      data[11][3] = 8'b11000011;
      data[12][3] = 8'b11000011;
      data[13][3] = 8'b00000000;
      data[14][3] = 8'b00000000;
      data[15][3] = 8'b00000000;

      data[0] [4] = 8'b00000000;
      data[1] [4] = 8'b00000000;
      data[2] [4] = 8'b11100111;
      data[3] [4] = 8'b11100111;
      data[4] [4] = 8'b11100111;
      data[5] [4] = 8'b11101111;
      data[6] [4] = 8'b11111111;
      data[7] [4] = 8'b11111011;
      data[8] [4] = 8'b11011011;
      data[9] [4] = 8'b11011011;
      data[10][4] = 8'b11000011;
      data[11][4] = 8'b11000011;
      data[12][4] = 8'b11000011;
      data[13][4] = 8'b00000000;
      data[14][4] = 8'b00000000;
      data[15][4] = 8'b00000000;

      data[0] [5] = 8'b00000000;
      data[1] [5] = 8'b00000000;
      data[2] [5] = 8'b11100111;
      data[3] [5] = 8'b11100111;
      data[4] [5] = 8'b11100111;
      data[5] [5] = 8'b11101111;
      data[6] [5] = 8'b11111111;
      data[7] [5] = 8'b11111011;
      data[8] [5] = 8'b11011011;
      data[9] [5] = 8'b11011011;
      data[10][5] = 8'b11000011;
      data[11][5] = 8'b11000011;
      data[12][5] = 8'b11000011;
      data[13][5] = 8'b00000000;
      data[14][5] = 8'b00000000;
      data[15][5] = 8'b00000000;

      data[0] [6] = 8'b00000000;
      data[1] [6] = 8'b00000000;
      data[2] [6] = 8'b11100111;
      data[3] [6] = 8'b11100111;
      data[4] [6] = 8'b11100111;
      data[5] [6] = 8'b11101111;
      data[6] [6] = 8'b11111111;
      data[7] [6] = 8'b11111011;
      data[8] [6] = 8'b11011011;
      data[9] [6] = 8'b11011011;
      data[10][6] = 8'b11000011;
      data[11][6] = 8'b11000011;
      data[12][6] = 8'b11000011;
      data[13][6] = 8'b00000000;
      data[14][6] = 8'b00000000;
      data[15][6] = 8'b00000000;

      data[0] [7] = 8'b00000000;
      data[1] [7] = 8'b00000000;
      data[2] [7] = 8'b11100111;
      data[3] [7] = 8'b11100111;
      data[4] [7] = 8'b11100111;
      data[5] [7] = 8'b11101111;
      data[6] [7] = 8'b11111111;
      data[7] [7] = 8'b11111011;
      data[8] [7] = 8'b11011011;
      data[9] [7] = 8'b11011011;
      data[10][7] = 8'b11000011;
      data[11][7] = 8'b11000011;
      data[12][7] = 8'b11000011;
      data[13][7] = 8'b00000000;
      data[14][7] = 8'b00000000;
      data[15][7] = 8'b00000000;

    end

    reg [15:0] char_counter = 0;
    reg [3:0] sub_char_counter = 0;
    localparam CHAR_WIDTH = 10;
    always @ ( posedge CLK ) begin
      sub_char_counter <= sub_char_counter + 1;

      if (sub_char_counter == (CHAR_WIDTH - 1)) begin
        char_counter <= char_counter + 1;
        sub_char_counter <= 0;
      end

      if ( next_pixel_counter == 0 ) begin
        char_counter <= 0;
        sub_char_counter <= 0;
      end
    end

    reg [7:0] char_line_counter = 0;
    reg [7:0] sub_char_line_counter = 0;
    localparam CHAR_HEIGHT = 25;
    always @ ( posedge CLK ) begin
      if (next_pixel_counter == 0 ) begin
        sub_char_line_counter <= sub_char_line_counter + 1;
        if (sub_char_line_counter == (CHAR_HEIGHT-1)) begin
          char_line_counter <= char_line_counter + 1;
          sub_char_line_counter <= 0;
        end
        if (line_counter == (NUMBER_LINES-1)) begin
          char_line_counter <= 0;
          sub_char_line_counter <= 0;
         end
      end
    end

    always @ ( * ) begin
      //red = pixel_counter == 16'h031d | pixel_counter == 16'h0000;
      //green = pixel_counter == 16'h031e | pixel_counter == 16'h0001;
      //blue = pixel_counter == 16'h031f | pixel_counter == 16'h0002;
      case (sub_char_line_counter)
      8'd0, 8'd1, 8'd2, 8'd3, 8'd4, 8'd5, 8'd6: red = 1'b0; // Upper bound
      8'd23, 8'd24: red = 1'b0; // Lower bound
      default: begin
        case (sub_char_counter)
        4'h0, 4'h9:
          red = 1'b0;
        default:
          red = data[sub_char_line_counter-7][char_counter[2:0]][sub_char_counter-1]; // Actual data
        endcase
      end
      endcase
      //red = data[line_counter[3:0]][sub_char_counter];
      green = red;
      blue = red;
      //red = line_counter[0] == 1'b0 && pixel_counter[2] == 1'b0;
      //green = line_counter[1] == 1'b0 && pixel_counter[3] == 1'b0;
      //blue = line_counter[2] == 1'b0 && pixel_counter[4] == 1'b0;
      //red = data[pixel_counter[2:0]];
      //green = not_data[pixel_counter[2:0]];
      //blue = 1'b0;
    end

endmodule
