`default_nettype none

// look in pins.pcf for all the pin names on the TinyFPGA BX board
module top (
    input CLK,    // 16MHz clock
    output LED,   // User/boot LED next to power LED
    output USBPU,  // USB pull-up resistor

    output PIN_13,
    output PIN_10
    );

    localparam INST_ADDR_WIDTH = 8;
    localparam DATA_ADDR_WIDTH = 8;
    localparam INPUT_CLOCK     = 16_000_000;
    localparam OUTPUT_CLOCK    = 1;

    // drive USB pull-up resistor to '0' to disable USB
    assign USBPU = 0;

    // Setup clock
    reg real_clock = 0;

    // Slow down the clock to one second.
    reg [31:0] count = 0;
    always @ ( posedge CLK ) begin
      if (count < (INPUT_CLOCK / 2 * OUTPUT_CLOCK)) begin
        count <= count + 1;
      end else begin
        count <= 0;
        real_clock <= ~real_clock;
      end
    end

    // LED IO only for now
    wire LSB0;
    assign PIN_13 = LSB0;
    wire LSB1;
    assign PIN_10 = LSB1;

    // PROGRAM COUNTER
    reg inc_inst = 1; // for now just always incremt
    wire [15:0] inst;
    reg [(INST_ADDR_WIDTH-1):0] in_pc_addr = 0;
    reg load_pc = 1'b0;
    wire [(INST_ADDR_WIDTH-1):0] pc_addr;
    program_counter #(.ADDR_WIDTH(INST_ADDR_WIDTH), .PROGRAM_FILENAME("program.hex")) pc
      (.i_clk(real_clock), .i_inc(inc_inst), .i_load(load_pc), .i_addr(in_pc_addr), .o_addr(pc_addr), .o_instruction(inst));

    // DATA RAM
    reg load_ram = 0;
    reg [(DATA_ADDR_WIDTH-1):0] ram_addr = 0;
    reg [15:0] ram_data_in = 0;
    wire [15:0] ram_data_out;
    ram #(.ADDR_WIDTH(DATA_ADDR_WIDTH), .FILENAME("empty.hex")) ram
      (.i_clk(real_clock), .i_load(load_ram), .i_addr(ram_addr), .i_data(ram_data_in), .o_data(ram_data_out));


    assign LSB0 = inst[0];
    assign LSB1 = inst[4];
    assign LED  = inst[8];

endmodule
