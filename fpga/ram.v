`default_nettype none

module ram (
  input i_clk,

  input  wire [15:0] read_addr,
  output wire [15:0] read_data,

  input wire [15:0] write_addr,
  input wire [15:0] write_data,
  input wire        write_strobe   //
  );

parameter FILENAME = "empty.hex";
parameter RAM_ADDR_WIDTH = 8; // unused for now

// This will infer a ram block
reg [15:0] mem [4096-1:0];
initial $readmemh(FILENAME, mem);

always @ ( posedge i_clk ) begin
  if (write_strobe && 4'h0 == write_addr[15:12])
    mem[write_addr[11:0]] <= write_data;
end

reg addr_good;
always @ ( posedge i_clk ) begin
  addr_good <= 4'h0 == read_addr[15:12];
end

reg [15:0] data_out;
always @ ( posedge i_clk ) begin
  data_out <= mem[read_addr[11:0]];
end

assign read_data = (addr_good) ? data_out : 16'bzzzzzzzzzzzzzzzz;

endmodule
