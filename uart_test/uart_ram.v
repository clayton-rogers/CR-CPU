`default_nettype none

module uart_ram (
  input i_clk,

  input      [8:0] read_addr,
  output     [7:0] read_data,

  input     [8:0] write_addr,
  input     [7:0] write_data,
  input           write_strobe   //
);

// This will infer a ram block
reg [7:0] mem [511:0];
integer i;
initial begin
  for (i=0; i < 512; i=i+1)
    mem[i] = 8'b00;
end

always @ ( posedge i_clk ) begin
  if (write_strobe)
    mem[write_addr] <= write_data;
end

reg [7:0] data_out;
always @ ( posedge i_clk ) begin
  data_out <= mem[read_addr];
end

assign read_data = data_out;

endmodule
