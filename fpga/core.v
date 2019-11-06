`default_nettype none

module core (
  input i_clk,

  output reg  [15:0] read_addr,
  input  wire [15:0] read_data,

  output reg [15:0] write_addr,
  output reg [15:0] write_data,
  output reg        write_strobe
  );

  // Named constants for all the opcodes and sub codes (shared with ALU)
  `include "opcodes.vh"

  reg state = 1'b1;

  reg [15:0] pc = 16'hFFFF;
  reg [7:0] address = 8'h00;

  reg [15:0] ra = 16'h0000;
  reg [15:0] rb = 16'h0000;
  reg [15:0] rp = 16'h0000;
  reg [15:0] sp = 16'h0000;

  // For instructions which take two cycles, need to cache the instruction
  // for next cycle
  reg [15:0] cached_instruction = 16'hF000; // default nop
  always @ ( posedge i_clk ) begin
    case (state)
    1'b0: cached_instruction <= read_data;
    1'b1: cached_instruction <= cached_instruction;
    endcase
  end

  // In general the current instruction is the read data, except for on the
  // second cycle, then we use the cached instruction
  reg [15:0] instruction;
  always @ ( * ) begin
    case (state)
    1'b0: instruction = read_data;
    1'b1: instruction = cached_instruction;
    endcase
  end

  // Decode
  wire [3:0] opcode     = instruction[15:12];
  wire [1:0] extra_high = instruction[11:10];
  wire [1:0] extra_low  = instruction[9:8];
  wire [7:0] constant   = instruction[7:0];

  // sign extended immediate
  wire [15:0] signed_constant = {{8{constant[7]}},constant};

  // full address - used by long jump/call and direct load/store
  wire [15:0] full_addr = {address, constant};

  // ==========================================================================
  // This section determines the jump location in the case of call/jump/ret
  // ==========================================================================
  // whether a call or jump is using a relative or absolute address
  wire is_relative = !extra_low[1];

  // In the case a call/jump is using a relative address, this is it
  wire [15:0] rel_cj_addr = pc + signed_constant;

  // The call/jump address, regardless of relative flag
  wire [15:0] cj_addr = (is_relative) ? rel_cj_addr : full_addr;

  // The address popped off the stack in the case of ret
  wire [15:0] popped_addr = read_data;

  // The final calculated address for all call/jmp/ret
  // if the next address is anything other than PC or PC + 1, this is it
  reg [15:0] calculated_pc;
  always @ ( * ) begin
    if (opcode == CALL_RET && extra_low[0] == SUB_RET)
      calculated_pc = popped_addr;
    else
      calculated_pc = cj_addr;
  end

  // In general the program counter is incremented every clock
  wire [15:0] inc_pc = pc + 1;

  // ==========================================================================
  // This section calculates whether a conditional jump should be taken
  // ==========================================================================
  reg should_jump;
  always @ ( * ) begin
    case (extra_high)
    2'b00: should_jump = 1'b1;        // jmp
    2'b01: should_jump = ra == 0;     // jmp z
    2'b10: should_jump = ra != 0;     // jmp nz
    2'b11: should_jump = ra[15] == 0; // jmp gz (positive)
    endcase
  end

  // ==========================================================================
  // The load address is used any time the next fetch will not be getting the
  // next instruction. Ex. a load, pop, or ret instruction
  // ==========================================================================
  // In the case of ret, the ret address is used as the load address
  wire [15:0] ret_addr = sp;
  // TODO forget exatly what these are
  wire [15:0] base_addr = (extra_low[0] == 1'b1) ? sp : rp;
  wire [15:0] offset_addr = base_addr + signed_constant;
  wire [15:0] load_inst_addr = (extra_low[1] == 1'b1) ? full_addr : offset_addr;

  wire [15:0] load_addr = (opcode == CALL_RET ||
    (opcode == PUSH_POP && extra_low[0] == 1'b1)) ?
    ret_addr : load_inst_addr;

  // ==========================================================================
  // This section calculates what the next PC value should be
  // ==========================================================================
  reg [15:0] next_pc;
  reg        next_state;

  always @ ( * ) begin
    if ((opcode == JUMP && should_jump) ||   // jmp
        (opcode == CALL_RET && extra_low[0] == 0) || // call
        (opcode == CALL_RET && extra_low[0] == 1 && state == 1) // ret
    ) begin
      next_pc = calculated_pc;
      next_state = 0;
    end else if ((opcode == LOAD && state == 0) ||  // first cycle load
                 (opcode == CALL_RET && extra_low[0] == 1 && state == 0) || // first cycle ret
                 (opcode == PUSH_POP && extra_low[0] == 1 && state == 0) || // first cycle pop
                 (opcode == HALT)  // HALT
    ) begin
      next_pc = pc;
      next_state = 1;
    end else begin
      next_pc = inc_pc;
      next_state = 0;
    end
  end

  // Finally, assign the next pc for next clock
  always @ ( posedge i_clk ) begin
    pc <= next_pc;
  end
  // Also assign the next_state
  always @ ( posedge i_clk ) begin
    state <= next_state;
  end

  // ==========================================================================
  // In general the next read location is the next instruction, except when
  // we're actually doing a load (or ret)
  // ==========================================================================
  always @ ( * ) begin
    if ((opcode == LOAD && state == 0) ||  // first cycle load
        (opcode == CALL_RET && extra_low[0] == 1 && state == 0) || // first cycle ret
        (opcode == PUSH_POP && extra_low[0] == 1 && state == 0)    // first cycle pop
    ) begin
      read_addr = load_addr;
    end else begin
      read_addr = next_pc;
    end
  end

  // ==========================================================================
  // All mathematical calculatons are performed by the ALU. First we must
  // figure out what to feed into it.
  // ==========================================================================
  wire shift_dir = extra_low[0];
  reg  [15:0] input_1;
  always @ ( * ) begin
    case (extra_high)
    2'b00: input_1 = ra;
    2'b01: input_1 = rb;
    2'b10: input_1 = rp;
    2'b11: input_1 = sp;
    endcase
  end
  reg [15:0] input_2;
  always @ ( * ) begin
    case (opcode)
      ADD, SUB, AND, OR, XOR, MOV:
        case (extra_low)
        2'b00: input_2 = ra;
        2'b01: input_2 = rb;
        2'b10: input_2 = rp;
        2'b11: input_2 = (opcode == MOV) ? sp : {8'h00, constant};
        endcase
      SHIFT:
        if (extra_low[1])
          input_2 = {8'h00, constant};
        else
          input_2 = rb;
    default: input_2 = ra; // doesn't matter
    endcase
  end
  wire [15:0] alu_output;

  ALU alu (
    .i_opcode(opcode),
    .i_shift_dir(shift_dir),
    .i_data1(input_1),
    .i_data2(input_2),
    .o_data(alu_output)
    );

  // ==========================================================================
  // This section calculates what (if anything) should be written.
  // ==========================================================================
  always @ ( * ) begin
    case (opcode)
      STORE:
      begin
        write_addr = load_addr;
        write_data = input_1;
        write_strobe = 1'b1;
      end
      PUSH_POP:
      if (extra_low[0]) begin
        write_addr = 16'h0000; // doesn't matter because no write stobe
        write_data = 16'h0000;
        write_strobe = 1'b0;
      end else begin
        write_addr = sp - 1;
        write_data = input_1;
        write_strobe = 1'b1;
      end
      CALL_RET:
      if (extra_low[0]) begin
        write_addr = 16'h0000; // doesn't matter because no write stobe
        write_data = 16'h0000;
        write_strobe = 1'b0;
      end else begin
        write_addr = sp - 1;
        write_data = inc_pc;
        write_strobe = 1'b1;
      end
      default:
      begin
        write_addr = 16'h0000; // doesn't matter because no write stobe
        write_data = 16'h0000;
        write_strobe = 1'b0;
      end
    endcase
  end

  // ==========================================================================
  // This section calculates if any registers need updating.
  // ==========================================================================
  always @ ( posedge i_clk ) begin
    case (opcode)
      ADD, SUB, AND, OR, XOR, MOV:
        case (extra_high)
        2'b00: ra <= alu_output;
        2'b01: rb <= alu_output;
        2'b10: rp <= alu_output;
        2'b11: sp <= alu_output;
        endcase
      LOAD:
        if (state == 1)
          case (extra_high)
          2'b00: ra <= read_data;
          2'b01: rb <= read_data;
          2'b10: rp <= read_data;
          2'b11: sp <= read_data;
          endcase
      LOADI:
        if (extra_low[1]) begin
          case (extra_high)
          2'b00: ra <= {constant, ra[7:0]};
          2'b01: rb <= {constant, rb[7:0]};
          2'b10: rp <= {constant, rp[7:0]};
          2'b11: sp <= {constant, sp[7:0]};
          endcase
        end else begin
          case (extra_high)
          2'b00: ra <= {8'h00, constant};
          2'b01: rb <= {8'h00, constant};
          2'b10: rp <= {8'h00, constant};
          2'b11: sp <= {8'h00, constant};
          endcase
        end
      PUSH_POP:
        if (extra_low[0]) begin  // pop
          if (state == 1'b1) begin
            case (extra_high)
            2'b00: ra <= read_data;
            2'b01: rb <= read_data;
            2'b10: rp <= read_data;
            2'b11: sp <= read_data;
            endcase
            sp <= sp + 1;
          end
        end else begin // push
          sp <= sp - 1;
        end
      CALL_RET:
        if (extra_low[0]) begin
          if (state == 1)
            sp <= sp + 1;
        end else begin
          sp <= sp - 1;
        end
      LOADA:
        address <= constant;
      default: ;
    endcase // case (opcode)
  end
endmodule
