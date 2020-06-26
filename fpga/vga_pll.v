
`default_nettype none

module vga_pll(input REFERENCECLK,
               output PLLOUTGLOBAL);


// Fout = Fref (DIVF + 1) / ((2^DIVQ)*(DIVR+1))
SB_PLL40_CORE
#(


.DIVR(4'b0000), //0
.DIVF(7'b0100111), //39
.DIVQ(3'b100), //4
//40.0 Mhz
// Produces "800x600@60"

.FILTER_RANGE(3'b001),
.FEEDBACK_PATH("SIMPLE"),
.DELAY_ADJUSTMENT_MODE_FEEDBACK("FIXED"),
.FDA_FEEDBACK(4'b0000),
.DELAY_ADJUSTMENT_MODE_RELATIVE("FIXED"),
.FDA_RELATIVE(4'b0000),
.SHIFTREG_DIV_MODE(2'b00),
.PLLOUT_SELECT("GENCLK"),
.ENABLE_ICEGATE(1'b0),
)
// verilator lint_off PINCONNECTEMPTY
top_pll_inst(.REFERENCECLK(REFERENCECLK),
                           .PLLOUTCORE(),
                           .PLLOUTGLOBAL(PLLOUTGLOBAL),
                           .EXTFEEDBACK(),
                           .DYNAMICDELAY(),
                           .RESETB(1'b1),
                           .BYPASS(1'b0),
                           .LATCHINPUTVALUE(),
                           .LOCK(),
                           .SDI(),
                           .SDO(),
                           .SCLK()
                           );
// verilator lint_on PINCONNECTEMPTY

endmodule
