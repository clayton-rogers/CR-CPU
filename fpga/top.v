`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_13,
    output PIN_10
    );

    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;

    wire LSB0;
    assign PIN_13 = LSB0;
    wire LSB1;
    assign PIN_10 = LSB1;

    wire inc_inst;
    wire [15:0] inst;
    reg [7:0] in_pc_addr = 8'h00;
    reg load_pc = 1'b0;
    wire [7:0] pc_addr;
    program_counter #(.PROGRAM_FILENAME("program.hex")) pc
      (.i_clk(CLK), .i_inc(inc_inst), .i_load(load_pc), .i_addr(in_pc_addr), .o_addr(pc_addr), .o_instruction(inst));

    // Slow down the clock to one second.
    reg [31:0] count = 0;
    always @ ( posedge CLK ) begin
      if (count < 16_000_000) begin
        count <= count + 1;
      end else begin
        count <= 0;
      end
    end

    assign inc_inst = count == 0;

    assign LSB0 = inst[0];
    assign LSB1 = inst[4];
    assign LED  = inst[8];

endmodule
