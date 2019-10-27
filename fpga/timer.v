`default_nettype none

module timer (
  input i_clk,

  input  wire [15:0] read_addr,
  output wire  [15:0] read_data,

  input wire [15:0] write_addr,
  input wire [15:0] write_data,
  input wire        write_strobe
  );
parameter OFFSET = 16'h8200;

reg [31:0] count = 16'h0000;
always @ ( posedge i_clk ) begin
  count <= count + 1;
end

reg [15:0] out;
reg output_enable;
always @ ( posedge i_clk ) begin
  if (read_addr == OFFSET) begin
    out <= count[15:0];
    output_enable <= 1'b1;
  end else begin
    if (read_addr == OFFSET + 1) begin
      out <= count[31:16];
      output_enable <= 1'b1;
    end else begin
      out <= 16'h0000; // doesn't matter
      output_enable <= 1'b0;
    end
  end
end

assign read_data = (output_enable) ? out : 16'bzzzzzzzzzzzzzzzz;

endmodule
