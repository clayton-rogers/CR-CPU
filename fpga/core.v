`default_nettype none

module core (i_clk, o_leds);
    input i_clk;
    output [2:0] o_leds;

    parameter PROGRAM_FILENAME = "program.hex";

    localparam INST_ADDR_WIDTH = 8;
    localparam DATA_ADDR_WIDTH = 8;

    `include "opcodes.vh"

    // STATE
    localparam INITIAL             = 0;
    localparam LOAD_NEXT_INST      = 1;
    localparam EXECUTE             = 2;
    localparam WAIT_FOR_LOAD       = 3;
    reg [7:0] state = EXECUTE;
    always @ ( posedge i_clk ) begin
      case (state)
        LOAD_NEXT_INST:
          state <= EXECUTE;
        EXECUTE:
          // These instructions require a cycle to get new data from ram
          if (opcode == LOAD || opcode == JUMP) begin
            state <= WAIT_FOR_LOAD;
          end else begin
            state <= EXECUTE;
          end
        WAIT_FOR_LOAD:
          state <= EXECUTE;
        default:
          state <= LOAD_NEXT_INST;
      endcase
    end

    // PROGRAM COUNTER
    wire inc_inst = !((opcode == LOAD || opcode == JUMP) && state == EXECUTE);
    wire [15:0] inst;
    wire [(INST_ADDR_WIDTH-1):0] in_pc_addr = reg_output[0][(DATA_ADDR_WIDTH-1):0]; // For now PC in is always ra
    wire load_pc = (state == EXECUTE && opcode == JUMP);
    wire [(INST_ADDR_WIDTH-1):0] pc_addr; // unused
    program_counter #(.ADDR_WIDTH(INST_ADDR_WIDTH), .PROGRAM_FILENAME(PROGRAM_FILENAME)) pc
      (.i_clk(i_clk), .i_inc(inc_inst), .i_load(load_pc), .i_addr(in_pc_addr), .o_addr(pc_addr), .o_instruction(inst));

    // DATA RAM
    wire load_ram = (state == EXECUTE && opcode == STORE);
    wire [(DATA_ADDR_WIDTH-1):0] ram_addr = constant; // For now RAM address is always constant
    reg [15:0] ram_data_in;
    always @ ( * ) begin
      case (extra_high)
        2'b00: ram_data_in = reg_output[0];
        2'b01: ram_data_in = reg_output[1];
        2'b10: ram_data_in = reg_output[2];
        2'b11: ram_data_in = reg_output[3];
      endcase
    end
    wire [15:0] ram_data_out;
    ram #(.ADDR_WIDTH(DATA_ADDR_WIDTH), .FILENAME("empty.hex")) ram
      (.i_clk(i_clk), .i_load(load_ram), .i_addr(ram_addr), .i_data(ram_data_in), .o_data(ram_data_out));

    // DECODER
    wire [3:0] opcode = inst[15:12];
    wire [1:0] extra_high = inst[11:10];
    wire [1:0] extra_low = inst[9:8];
    wire [7:0] constant = inst[7:0];

    // REGISTERS (ra, rb, rc, rd)
    reg [15:0] reg_input;
    always @ ( * ) begin
      case (opcode)
        ADD,
        SUB,
        AND,
        OR,
        SHIFT,
        MOVE:
          reg_input = alu_out;
        LOAD:
          reg_input = ram_data_out;
        LOADC:
          reg_input = {8'h00, constant}; // TODO for now load c just loads the lower byte
        default:
          reg_input = 16'h0000;
      endcase
    end
    wire [63:0] raw_reg_output;
    wire [15:0] reg_output [3:0];
    wire should_load_reg = (state == EXECUTE &&
      (opcode == ADD ||
        opcode == SUB ||
        opcode == AND ||
        opcode == OR ||
        opcode == SHIFT ||
        opcode == MOVE ||
        opcode == LOADC) ||
        (state == WAIT_FOR_LOAD && opcode == LOAD));
    reg [3:0] reg_to_load;
    always @ ( * ) begin
      case (extra_high)
        2'b00: reg_to_load = 4'b0001;
        2'b01: reg_to_load = 4'b0010;
        2'b10: reg_to_load = 4'b0100;
        2'b11: reg_to_load = 4'b1000;
      endcase
    end
    wire [3:0] load_reg = (should_load_reg) ? reg_to_load : 4'h0;
    assign reg_output[3] = raw_reg_output[63:48];
    assign reg_output[2] = raw_reg_output[47:32];
    assign reg_output[1] = raw_reg_output[31:16];
    assign reg_output[0] = raw_reg_output[15:0 ];
    register register [3:0]
      (.i_clk(i_clk),
       .i_load(load_reg),
       .i_data(reg_input),
       .o_data(raw_reg_output));

    // INPUT 1
    reg [15:0] input_1;
    always @ ( * ) begin
    case (opcode)
      ADD,
      SUB,
      AND,
      OR:
        case (extra_low[1])
          1'b0: input_1 = reg_output[0];
          1'b1: input_1 = reg_output[1];
        endcase
      SHIFT  :
        case (extra_high)
          2'b00: input_1 = reg_output[0];
          2'b01: input_1 = reg_output[1];
          2'b10: input_1 = reg_output[2];
          2'b11: input_1 = reg_output[3];
        endcase
      LOAD   : input_1 = 16'h0000;
      STORE  : input_1 = 16'h0000;
      MOVE   : // MOVE goes through the alu
        case (extra_high)
          2'b00: input_1 = reg_output[0];
          2'b01: input_1 = reg_output[1];
          2'b10: input_1 = reg_output[2];
          2'b11: input_1 = reg_output[3];
        endcase
      JUMP   : input_1 = 16'h0000;
      LOADC  : input_1 = 16'h0000;
      UNDEF1 : input_1 = 16'h0000;
      UNDEF2 : input_1 = 16'h0000;
      UNDEF3 : input_1 = 16'h0000;
      UNDEF4 : input_1 = 16'h0000;
      UNDEF5 : input_1 = 16'h0000;
      UNDEF6 : input_1 = 16'h0000;
    endcase
    end

    // INPUT 2
    reg [15:0] input_2;
    always @ ( * ) begin
    case (opcode)
      ADD,
      SUB,
      AND,
      OR:
        case (extra_low[0])
          1'b0: input_2 = reg_output[2];
          1'b1: input_2 = {8'h00, constant};
        endcase
      SHIFT  :
        case (extra_low[1])
          1'b0: input_2 = reg_output[3];
          1'b1: input_2 = {8'h00, constant};
        endcase
      LOAD   : input_2 = 16'h0000;
      STORE  : input_2 = 16'h0000;
      MOVE   : input_2 = 16'h0000;
      JUMP   : input_2 = 16'h0000;
      LOADC  : input_2 = 16'h0000;
      UNDEF1 : input_2 = 16'h0000;
      UNDEF2 : input_2 = 16'h0000;
      UNDEF3 : input_2 = 16'h0000;
      UNDEF4 : input_2 = 16'h0000;
      UNDEF5 : input_2 = 16'h0000;
      UNDEF6 : input_2 = 16'h0000;
    endcase
    end

    // ALU
    wire [15:0] alu_out;
    ALU ALU
      (.i_opcode(opcode),
       .i_shift_dir(extra_low[0]),
       .i_data1(input_1),
       .i_data2(input_2),
       .o_data(alu_out));

    // OUTPUT
    assign o_leds[0] = inst[0];
    assign o_leds[1] = inst[4];
    assign o_leds[2] = inst[8];

endmodule
