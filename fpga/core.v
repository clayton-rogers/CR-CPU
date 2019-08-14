`default_nettype none

module core (i_clk, o_out);
    input i_clk;
    output reg [15:0] o_out;

    parameter PROGRAM_FILENAME = "program.hex";

    localparam INST_ADDR_WIDTH = 8;
    localparam DATA_ADDR_WIDTH = 8;

    `include "opcodes.vh"

    // STATE
    localparam INITIAL             = 0;
    localparam LOAD_NEXT_INST      = 1;
    localparam EXECUTE             = 2;
    localparam WAIT_FOR_LOAD       = 3;
    localparam HALT_STATE          = 4;
    reg [7:0] state = EXECUTE;
    always @ ( posedge i_clk ) begin
      case (state)
        EXECUTE:
          // These instructions require a cycle to get new data from ram
          case (opcode)
            LOAD,
            JUMP: state <= WAIT_FOR_LOAD;
            HALT: state <= HALT_STATE;
            default: state <= EXECUTE;
          endcase
        WAIT_FOR_LOAD:
          state <= EXECUTE;
        HALT_STATE:
          state <= HALT_STATE;
        default:
          state <= LOAD_NEXT_INST;
      endcase
    end

    // PROGRAM COUNTER
    reg inc_inst;
    always @ ( * ) begin
      case (state)
        EXECUTE:
          case (opcode)
            LOAD, JUMP: // These instructions take two cycles
              inc_inst = 1'b0;
            default: // In general we inc the pc every clock
              inc_inst = 1'b1;
          endcase
        WAIT_FOR_LOAD:
          inc_inst = 1'b1; // This is the second clock for these op so inc
        HALT_STATE:
          inc_inst = 1'b0; // Should stop incrementing when halted
        default:
          inc_inst = 1'b0; // unknown state should halt the proc
      endcase
    end
    wire [15:0] inst;
    reg [(INST_ADDR_WIDTH-1):0] pc_addr_in;
    always @ ( * ) begin
      case (extra_low)
      2'b00: pc_addr_in = pc_addr + constant + reg_output[0][(INST_ADDR_WIDTH-1):0];
      2'b01: pc_addr_in = pc_addr + constant + reg_output[1][(INST_ADDR_WIDTH-1):0];
      2'b10: pc_addr_in = pc_addr + constant + reg_output[2][(INST_ADDR_WIDTH-1):0];
      2'b11: pc_addr_in = pc_addr + constant; // none + constant;
      endcase
    end
    reg load_pc;
    always @ ( * ) begin
      if (state == EXECUTE) begin
        if (opcode == JUMP) begin
          case (extra_high)
            2'b00: load_pc = 1'b1; // JMP
            2'b01: load_pc = (|reg_output[0] == 0); // jump if zero
            2'b10: load_pc = (|reg_output[0] != 0); // jump if not zero
            2'b11: load_pc = (reg_output[0][15] == 0); // jump if greater than zero
          endcase
        end else begin
          load_pc = 1'b0;
        end
      end else begin
        load_pc = 1'b0;
      end
    end

    wire [(INST_ADDR_WIDTH-1):0] pc_addr;
    program_counter #(.ADDR_WIDTH(INST_ADDR_WIDTH), .PROGRAM_FILENAME(PROGRAM_FILENAME)) pc
      (.i_clk(i_clk), .i_inc(inc_inst), .i_load(load_pc), .i_addr(pc_addr_in), .o_addr(pc_addr), .o_instruction(inst));

    // DATA RAM
    wire load_ram = (state == EXECUTE && opcode == STORE);
    reg [(DATA_ADDR_WIDTH-1):0] ram_addr; // TODO For now RAM address is always constant
    always @ ( * ) begin
      case (extra_low)
      2'b00: ram_addr = constant + reg_output[0][(DATA_ADDR_WIDTH-1):0];
      2'b01: ram_addr = constant + reg_output[1][(DATA_ADDR_WIDTH-1):0];
      2'b10: ram_addr = constant + reg_output[2][(DATA_ADDR_WIDTH-1):0];
      2'b11: ram_addr = constant; // none + constant;
      endcase
    end
    wire [15:0] ram_data_in = reg_output[extra_high];
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
        XOR,
        SHIFT,
        MOVE:
          reg_input = alu_out;
        LOAD:
          reg_input = ram_data_out;
        LOADC:
          case (extra_low[1])
            1'b0: reg_input = {8'h00, constant};
            1'b1: reg_input = {constant, reg_output[extra_high][7:0]};
          endcase
        default:
          reg_input = 16'h0000;
      endcase
    end
    wire [63:0] raw_reg_output;
    wire [15:0] reg_output [3:0];
    wire load_reg = (state == EXECUTE &&
      (opcode == ADD ||
        opcode == SUB ||
        opcode == AND ||
        opcode == OR ||
        opcode == XOR ||
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
    wire [3:0] load_reg_flags = (load_reg) ? reg_to_load : 4'h0;
    assign reg_output[3] = raw_reg_output[63:48];
    assign reg_output[2] = raw_reg_output[47:32];
    assign reg_output[1] = raw_reg_output[31:16];
    assign reg_output[0] = raw_reg_output[15:0 ];
    register register [3:0]
      (.i_clk(i_clk),
       .i_load(load_reg_flags),
       .i_data(reg_input),
       .o_data(raw_reg_output));

    // INPUT 1
    reg [15:0] input_1;
    always @ ( * ) begin
    case (opcode)
      ADD,
      SUB,
      AND,
      OR,
      XOR    : input_1 = reg_output[ (extra_low[1]) ? 2'b10 : 2'b00 ];
      SHIFT  : input_1 = reg_output[extra_high];
      LOAD   : input_1 = 16'h0000;
      STORE  : input_1 = 16'h0000;
      // MOVE goes through the alu
      MOVE   : input_1 = reg_output[extra_low];
      JUMP   : input_1 = 16'h0000;
      LOADC  : input_1 = 16'h0000;
      OUT    : input_1 = 16'h0000;
      UNDEF3 : input_1 = 16'h0000;
      UNDEF4 : input_1 = 16'h0000;
      HALT   : input_1 = 16'h0000;
      NOP    : input_1 = 16'h0000;
    endcase
    end

    // INPUT 2
    reg [15:0] input_2;
    always @ ( * ) begin
    case (opcode)
      ADD,
      SUB,
      AND,
      OR,
      XOR:
        case (extra_low[0])
          1'b0: input_2 = reg_output[2'b01];
          1'b1: input_2 = {8'h00, constant};
        endcase
      SHIFT  :
        case (extra_low[1])
          1'b0: input_2 = reg_output[2'b01];
          1'b1: input_2 = {8'h00, constant};
        endcase
      LOAD   : input_2 = 16'h0000;
      STORE  : input_2 = 16'h0000;
      MOVE   : input_2 = 16'h0000;
      JUMP   : input_2 = 16'h0000;
      LOADC  : input_2 = 16'h0000;
      OUT    : input_2 = 16'h0000;
      UNDEF3 : input_2 = 16'h0000;
      UNDEF4 : input_2 = 16'h0000;
      HALT   : input_2 = 16'h0000;
      NOP    : input_2 = 16'h0000;
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
    always @ ( posedge i_clk ) begin
      if (opcode == OUT) begin
        o_out <= reg_output[extra_high];
      end else begin
        o_out <= o_out;
      end
    end

endmodule
