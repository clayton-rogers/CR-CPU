`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU  // USB pull-up resistor
    );
    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;



    wire inc_inst;
    wire [15:0] inst;
    program_counter pc (.i_clk(CLK), .i_inc(inc_inst), .o_instruction(inst));

    reg [31:0] count = 0;
    always @ ( posedge CLK ) begin
      if (count < 16_000_000) begin
        count <= count + 1;
      end else begin
        count <= 0;
      end
    end

    assign inc_inst = count == 0;

    assign LED = (count < 16_000 * 20) || inst[0];

endmodule
