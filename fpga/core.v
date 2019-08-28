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
    localparam EXECUTE             = 2;
    localparam WAIT_FOR_LOAD       = 3; // For jump and load instructions
    localparam HALT_STATE          = 4;
    localparam RET_WAIT_FOR_LOAD   = 5; // Second clock of getting ret address from stack (first is EXECUTE)
    localparam RET_LOAD_PC         = 6; // First clock of jump
    localparam RET_WAIT_PC         = 7; // Second clock of waiting for jump
    reg [7:0] state = EXECUTE;
    always @ ( posedge i_clk ) begin
      case (state)
        EXECUTE:
          // These instructions require a cycle to get new data from ram
          case (opcode)
            LOAD,
            JUMP: state <= WAIT_FOR_LOAD;
            IN_OUT_PUSH_POP:
              case (extra_low)
                SUB_IN:   state <= EXECUTE;
                SUB_OUT:  state <= EXECUTE;
                SUB_PUSH: state <= EXECUTE;
                SUB_POP:  state <= WAIT_FOR_LOAD;
              endcase
            CALL_RCALL_RET:
              case (extra_low)
                SUB_CALL:  state <= WAIT_FOR_LOAD;
                SUB_RCALL: state <= WAIT_FOR_LOAD;
                SUB_RET1:  state <= RET_WAIT_FOR_LOAD;
                SUB_RET2:  state <= RET_WAIT_FOR_LOAD;
              endcase
            HALT: state <= HALT_STATE;
            default: state <= EXECUTE;
          endcase
        WAIT_FOR_LOAD: // Waits for a single cycle for a load (from mem or from pc)
          state <= EXECUTE;
        RET_WAIT_FOR_LOAD:
          state <= RET_LOAD_PC;
        RET_LOAD_PC:
          state <= RET_WAIT_PC;
        RET_WAIT_PC:
          state <= EXECUTE;
        HALT_STATE:
          state <= HALT_STATE;

        default:
          state <= EXECUTE;
      endcase
    end

    // PROGRAM COUNTER
    reg inc_inst;
    always @ ( * ) begin
      case (state)
        EXECUTE:
          case (opcode)
            LOAD, JUMP, CALL_RCALL_RET: // These instructions take two (or four) cycles
              inc_inst = 1'b0;
            IN_OUT_PUSH_POP: // POP takes two, otherwise 1
              case (extra_low)
                SUB_IN:   inc_inst = 1'b1;
                SUB_OUT:  inc_inst = 1'b1;
                SUB_PUSH: inc_inst = 1'b1;
                SUB_POP:  inc_inst = 1'b0;
              endcase
            default: // In general we inc the pc every clock
              inc_inst = 1'b1;
          endcase
        WAIT_FOR_LOAD:
          inc_inst = 1'b1; // This is the second clock for these op so inc
        RET_WAIT_FOR_LOAD:
          inc_inst = 1'b0;
        RET_LOAD_PC:
          inc_inst = 1'b0;
        RET_WAIT_PC:
          inc_inst = 1'b1; // Last state of ret, so inc
        HALT_STATE:
          inc_inst = 1'b0; // Should stop incrementing when halted
        default:
          inc_inst = 1'b0; // unknown state should halt the proc
      endcase
    end
    wire [15:0] inst;
    wire [15:0] pc_relative_offset = pc_addr_out + {{8{constant[7]}},constant}; // constant is treated as signed so sign extend
    wire [15:0] pc_absolute_offset = {addr, constant};
    reg [15:0] pc_addr_in;
    always @ ( * ) begin  // TODO Call
      case (opcode)
        JUMP:
          case (extra_low[1])
            // Relative jump
            1'b0: pc_addr_in = pc_relative_offset;
            // Absolute jump
            1'b1: pc_addr_in = pc_absolute_offset;
          endcase
        CALL_RCALL_RET:
          case (extra_low)
            SUB_CALL: pc_addr_in = pc_absolute_offset;
            SUB_RCALL: pc_addr_in = pc_relative_offset;
            SUB_RET1: pc_addr_in = ram_data_out;
            SUB_RET2: pc_addr_in = ram_data_out;
          endcase
        default:
        pc_addr_in = 16'h0000;
      endcase
    end
    reg load_pc;
    always @ ( * ) begin
      case (state)
        EXECUTE:
          case (opcode)
            JUMP:
              case (extra_high)
                2'b00: load_pc = 1'b1; // JMP
                2'b01: load_pc = (|reg_output[0] == 0); // jump if zero
                2'b10: load_pc = (|reg_output[0] != 0); // jump if not zero
                2'b11: load_pc = (reg_output[0][15] == 0); // jump if greater than zero
              endcase
            CALL_RCALL_RET:
              case (extra_low)
                SUB_CALL: load_pc = 1'b1;
                SUB_RCALL: load_pc = 1'b1;
                SUB_RET1: load_pc = 1'b0; // ret will be loaded in RET_LOAD_PC state
                SUB_RET2: load_pc = 1'b0;
              endcase
            default:
              load_pc = 1'b0;
          endcase
        RET_LOAD_PC:
          load_pc = 1'b1;
        default:
          load_pc = 1'b0;
      endcase
    end

    wire [15:0] pc_addr_out;
    program_counter #(.ADDR_WIDTH(INST_ADDR_WIDTH), .PROGRAM_FILENAME(PROGRAM_FILENAME)) pc
      (.i_clk(i_clk), .i_inc(inc_inst), .i_load(load_pc), .i_addr(pc_addr_in), .o_addr(pc_addr_out), .o_instruction(inst));

    // DATA RAM
    wire load_ram = (state == EXECUTE && opcode == STORE);
    reg [15:0] ram_addr; // TODO For now RAM address is always constant
    always @ ( * ) begin
      case (opcode)
        IN_OUT_PUSH_POP,
        CALL_RCALL_RET:
          ram_addr = reg_output[3]; // CALL pushes to ram at SP
        default:
          case (extra_low)
          // BP + constant
          2'b00: ram_addr = {{8{constant[7]}},constant} + reg_output[2];
          // SP + constant
          2'b01: ram_addr = {{8{constant[7]}},constant} + reg_output[3];
          // Absolute address
          2'b10: ram_addr = {addr,constant};
          // Just a constant
          2'b11: ram_addr = {8'h00, constant}; // none + constant;
          endcase
      endcase
    end
    reg [15:0] ram_data_in;
    always @ ( * ) begin
      case (opcode)
        CALL_RCALL_RET:
          ram_data_in = pc_addr_out + 1; // CALL pushes return address onto stack
        default:
          ram_data_in = reg_output[extra_high];
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

    // ADDRESS register
    reg [7:0] addr = 8'h00;
    always @ ( posedge i_clk ) begin
      case (opcode)
        LOADA: addr <= constant;
        default: addr <= addr; // Does this need to exist?
      endcase
    end

    // REGISTERS (ra, rb, bp, sp)
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
        LOADI:
          case (extra_low[1])
            1'b0: reg_input = {8'h00, constant};
            1'b1: reg_input = {constant, reg_output[extra_high][7:0]};
          endcase
        IN_OUT_PUSH_POP: // needed for pop and in
          // For push - value is written to ram and SP decrement simultaneously
          // For pop  - first cycle SP is incremented, second cycle value is returned from ram
          // TODO wil changing the SP while getting ram cause a problem??
          case (state)
            EXECUTE:
              case (extra_low)
                SUB_PUSH: reg_input = reg_output[3] - 1;
                SUB_POP:  reg_input = reg_output[3] + 1;
                default : reg_input = 16'h0000; // TODO input should be here
              endcase
            WAIT_FOR_LOAD:
              reg_input = ram_data_out;
            default:
              reg_input = 16'h0000;
          endcase
        CALL_RCALL_RET:
          case (extra_low)
            SUB_CALL, SUB_RCALL: reg_input = reg_output[3] - 1;
            SUB_RET1, SUB_RET2:  reg_input = reg_output[3] + 1;
          endcase
        default:
          reg_input = 16'h0000;
      endcase
    end

    reg load_reg;
    always @ ( * ) begin
      case (state)
        EXECUTE:
          case (opcode)
            ADD, SUB, AND, OR, XOR, SHIFT, MOVE, LOADI:
              load_reg = 1'b1;
            default:
              load_reg = 1'b0;
          endcase
        WAIT_FOR_LOAD:
          case (opcode)
            LOAD: load_reg = 1'b1;
            IN_OUT_PUSH_POP:
              case (extra_low)
                SUB_IN: load_reg = 1'b0; // TODO in the future inputing a value should load it into a reg
                SUB_OUT: load_reg = 1'b0;
                SUB_PUSH: load_reg = 1'b1;
                SUB_POP: load_reg = 1'b1;
              endcase
            default:
              load_reg = 1'b0; // Should never happen (?)
          endcase
        default: // in general don't load any registers
          load_reg = 1'b0;
      endcase
    end

    reg [3:0] reg_to_load;
    always @ ( * ) begin
      case (opcode)
        IN_OUT_PUSH_POP:
          case (extra_low)
            SUB_IN:
              case (extra_high)
                2'b00: reg_to_load = 4'b0001;
                2'b01: reg_to_load = 4'b0010;
                2'b10: reg_to_load = 4'b0100;
                2'b11: reg_to_load = 4'b1000;
              endcase
            SUB_OUT: reg_to_load = 4'b0001; // Doesn't matter, not loading
            SUB_PUSH,
            SUB_POP: reg_to_load = 4'b1000; // push, pop, call, rcall, ret all affect SP, i.e. reg 4
          endcase
        CALL_RCALL_RET:
          reg_to_load = 4'b1000; // push, pop, call, rcall, ret all affect SP, i.e. reg 4
        default:
          case (extra_high)
            2'b00: reg_to_load = 4'b0001;
            2'b01: reg_to_load = 4'b0010;
            2'b10: reg_to_load = 4'b0100;
            2'b11: reg_to_load = 4'b1000;
          endcase
      endcase
    end

    wire [3:0] load_reg_flags = (load_reg) ? reg_to_load : 4'h0;
    assign reg_output[3] = raw_reg_output[63:48];
    assign reg_output[2] = raw_reg_output[47:32];
    assign reg_output[1] = raw_reg_output[31:16];
    assign reg_output[0] = raw_reg_output[15:0 ];
    wire [63:0] raw_reg_output;
    wire [15:0] reg_output [3:0];
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
      XOR    : input_1 = reg_output[extra_high];
      SHIFT  : input_1 = reg_output[extra_high];
      LOAD   : input_1 = 16'h0000;
      STORE  : input_1 = 16'h0000;
      // MOVE goes through the alu
      MOVE   : input_1 = reg_output[extra_low];
      JUMP   : input_1 = 16'h0000;
      LOADI  : input_1 = 16'h0000;
      IN_OUT_PUSH_POP : input_1 = 16'h0000;
      CALL_RCALL_RET  : input_1 = 16'h0000;
      LOADA  : input_1 = 16'h0000;
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
        case (extra_low)
          2'b11: input_2 = {8'h00, constant};
          default: input_2 = reg_output[extra_low];
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
      LOADI  : input_2 = 16'h0000;
      IN_OUT_PUSH_POP : input_2 = 16'h0000;
      CALL_RCALL_RET  : input_2 = 16'h0000;
      LOADA  : input_2 = 16'h0000;
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
      if (opcode == IN_OUT_PUSH_POP) begin
        o_out <= reg_output[extra_high];
      end else begin
        o_out <= o_out;
      end
    end

endmodule
