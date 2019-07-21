`default_nettype none

module core (i_clk, o_leds);
    input i_clk;
    output [2:0] o_leds;

    localparam INST_ADDR_WIDTH = 8;
    localparam DATA_ADDR_WIDTH = 8;

    // PROGRAM COUNTER
    reg inc_inst = 1; // for now just always incremt
    wire [15:0] inst;
    reg [(INST_ADDR_WIDTH-1):0] in_pc_addr = 0;
    reg load_pc = 1'b0;
    wire [(INST_ADDR_WIDTH-1):0] pc_addr;
    program_counter #(.ADDR_WIDTH(INST_ADDR_WIDTH), .PROGRAM_FILENAME("program.hex")) pc
      (.i_clk(i_clk), .i_inc(inc_inst), .i_load(load_pc), .i_addr(in_pc_addr), .o_addr(pc_addr), .o_instruction(inst));

    // DATA RAM
    reg load_ram = 0;
    reg [(DATA_ADDR_WIDTH-1):0] ram_addr = 0;
    reg [15:0] ram_data_in = 0;
    wire [15:0] ram_data_out;
    ram #(.ADDR_WIDTH(DATA_ADDR_WIDTH), .FILENAME("empty.hex")) ram
      (.i_clk(i_clk), .i_load(load_ram), .i_addr(ram_addr), .i_data(ram_data_in), .o_data(ram_data_out));

    // OUTPUT
    assign o_leds[0] = inst[0];
    assign o_leds[1] = inst[4];
    assign o_leds[2] = inst[8];

endmodule
