`default_nettype none

module timer (
  input i_clk,

  input  wire [15:0] read_addr,
  output wire [15:0] read_data,

  input wire [15:0] write_addr,
  input wire [15:0] write_data,
  input wire        write_strobe
  );
parameter BASE_ADDR = 16'h8200;

reg [31:0] count = 32'h00000000;
always @ ( posedge i_clk ) begin
  count <= count + 1;
end

reg [15:0] out;
reg output_enable;
always @ ( posedge i_clk ) begin
  if (read_addr == BASE_ADDR) begin
    out <= count[15:0];
    output_enable <= 1'b1;
  end else begin
    if (read_addr == BASE_ADDR + 1) begin
      out <= count[31:16];
      output_enable <= 1'b1;
    end else begin
      out <= 16'h0000; // doesn't matter
      output_enable <= 1'b0;
    end
  end
end

assign read_data = (output_enable) ? out : 16'bzzzzzzzzzzzzzzzz;

/* verilator lint_off UNUSED */
wire [15:0] unused_write_addr   = write_addr;
wire [15:0] unused_write_data   = write_data;
wire        unused_write_strobe = write_strobe;
/* verilator lint_on UNUSED */

endmodule
