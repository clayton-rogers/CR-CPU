
module top_pll(input REFERENCECLK,
               output PLLOUTCORE,
               output PLLOUTGLOBAL,
               input RESET);

//input REFERENCECLK;
//input RESET;
//output PLLOUTCORE;
//output PLLOUTGLOBAL;


// Fout = Fref (DIVF + 1) / ((2^DIVQ)*(DIVR+1))
SB_PLL40_CORE
#(

//.DIVR(4'b0000), //0
//.DIVF(7'b0110001), //49
//.DIVQ(3'b101), //5
// 25.0 Mhz
// Produces "640x480@59"

//.DIVR(4'b0110), //6
//.DIVF(7'b01010101), //85
//.DIVQ(3'b011), //3
//24.5714 Mhz
// Produces "640x500@58"
// No blurring

//.DIVR(4'b0100), //4
//.DIVF(7'b0111110), //62
//.DIVQ(3'b011), //3
//25.2 Mhz
// Produces "640x480@60"
// Slight blurring every ~44 pixels
// With auto adjustment seems to work

//.DIVR(4'b0110), //6
//.DIVF(7'b0001010), //10
//.DIVQ(3'b000), //0
//25.1429 Mhz
// Produces "640x480@60"
// Shimering over whole screen

.DIVR(4'b0000), //0
.DIVF(7'b0100111), //39
.DIVQ(3'b100), //4
//40.0 Mhz
// Produces "800x600@60"
// After autoadjustment looks perfect


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
top_pll_inst(.REFERENCECLK(REFERENCECLK),
                           .PLLOUTCORE(PLLOUTCORE),
                           .PLLOUTGLOBAL(PLLOUTGLOBAL),
                           .EXTFEEDBACK(),
                           .DYNAMICDELAY(),
                           .RESETB(RESET),
                           .BYPASS(1'b0),
                           .LATCHINPUTVALUE(),
                           .LOCK(),
                           .SDI(),
                           .SDO(),
                           .SCLK()
                           );



endmodule
