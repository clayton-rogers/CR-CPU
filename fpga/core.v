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

    // DECODER
    wire [3:0] opcode = inst[15:12];
    wire [1:0] extra_high = inst[11:10];
    wire [1:0] extra_low = inst[9:8];
    wire [7:0] constant = inst[7:0];

    // ALU
    reg [15:0] data1 = 0;
    reg [15:0] data2 = 0;
    wire [15:0] alu_out;
    ALU ALU
      (.i_opcode(opcode),
       .i_extra(extra_low),
       .i_data1(data1),
       .i_data2(data2),
       .i_const (constant),
       .o_data(alu_out));

    // REGISTERS
    reg [3:0] load_reg = 4'b0000;
    reg [15:0] reg_input = 16'h0000;
    wire [63:0] raw_reg_output;
    wire [15:0] reg_output [3:0];
    assign reg_output[0] = raw_reg_output[63:48];
    assign reg_output[1] = raw_reg_output[47:32];
    assign reg_output[2] = raw_reg_output[31:16];
    assign reg_output[3] = raw_reg_output[15:0 ];
    register register [0:3]
      (.i_clk(i_clk),
       .i_load(load_reg),
       .i_data(reg_input),
       .o_data(raw_reg_output));

    // *** CORE **** //
    `include "opcodes.vh"
    // INPUT 1
    reg [15:0] input_1;
    // always @ ( * ) begin
    // case (opcode)
    //   ADD    : input_1
    //   begin
    //     case (extra_low[1])
    //       1'b0 :
    //     endcase
    //   end
    //   SUB    :
    //   AND    :
    //   OR     :
    //   SHIFT  :
    //   LOAD   :
    //   STORE  :
    //   MOVE   :
    //   JUMP   :
    //   LOADC  :
    //   UNDEF1 :
    //   UNDEF2 :
    //   UNDEF3 :
    //   UNDEF4 :
    //   UNDEF5 :
    //   UNDEF6 :
    // endcase
    // end

    // INPUT 2
    reg [15:0] input_2 = 16'h0000;

    // STATE
    localparam INITIAL = 0;
    localparam LOAD_NEXT_INST = 1;
    reg [7:0] state = LOAD_NEXT_INST;

    // OUTPUT
    assign o_leds[0] = inst[0];
    assign o_leds[1] = inst[4];
    assign o_leds[2] = inst[8];

endmodule
