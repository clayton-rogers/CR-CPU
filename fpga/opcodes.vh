
localparam ADD    = 4'h0;
localparam SUB    = 4'h1;
localparam AND    = 4'h2;
localparam OR     = 4'h3;
localparam XOR    = 4'h4;
localparam SHIFT  = 4'h5;
localparam LOAD   = 4'h6;
localparam STORE  = 4'h7;
localparam MOVE   = 4'h8;
localparam JUMP   = 4'h9;
localparam LOADI  = 4'hA;
localparam IN_OUT_PUSH_POP = 4'hB;
localparam CALL_RCALL_RET  = 4'hC;
localparam LOADA           = 4'hD;
localparam HALT            = 4'hE;
localparam NOP             = 4'hF;

// Sub commands for 0xB
localparam SUB_IN = 2'b00;
localparam SUB_OUT = 2'b01;
localparam SUB_PUSH = 2'b10;
localparam SUB_POP = 2'b11;

// Sub commands for 0xC
localparam SUB_CALL = 2'b00;
localparam SUB_RCALL = 2'b01;
localparam SUB_RET1 = 2'b10;
localparam SUB_RET2 = 2'b11;
