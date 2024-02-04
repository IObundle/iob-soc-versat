`timescale 1ns / 1ps

`include "bsp.vh"
`include "iob_soc_versat_conf.vh"

//Peripherals _swreg_def.vh file includes.
`include "iob_uart_swreg_def.vh"
`include "iob_timer_swreg_def.vh"
`include "iob_versat_swreg_def.vh"

`ifndef IOB_UART_SWREG_ADDR_W
`define IOB_UART_SWREG_ADDR_W 16
`endif
`ifndef IOB_SOC_VERSAT_DATA_W
`define IOB_SOC_VERSAT_DATA_W 32
`endif
`ifndef IOB_ETH_SWREG_ADDR_W
`define IOB_ETH_SWREG_ADDR_W 12
`endif

module iob_soc_versat_sim_wrapper (
   input  [1-1:0] clk_i,
   input  [1-1:0] arst_i,
   output         trap_o,

`ifdef IOB_SOC_VERSAT_USE_ETHERNET
   // Ethernet for testbench
   input                               ethernet_valid_i,
   input  [ `IOB_ETH_SWREG_ADDR_W-1:0] ethernet_addr_i,
   input  [`IOB_SOC_VERSAT_DATA_W-1:0] ethernet_wdata_i,
   input  [                       3:0] ethernet_wstrb_i,
   output [`IOB_SOC_VERSAT_DATA_W-1:0] ethernet_rdata_o,
   output                              ethernet_ready_o,
   output                              ethernet_rvalid_o,
`endif

   // UART for testbench
   input                               uart_valid_i,
   input  [`IOB_UART_SWREG_ADDR_W-1:0] uart_addr_i,
   input  [`IOB_SOC_VERSAT_DATA_W-1:0] uart_wdata_i,
   input  [                       3:0] uart_wstrb_i,
   output [`IOB_SOC_VERSAT_DATA_W-1:0] uart_rdata_o,
   output                              uart_ready_o,
   output                              uart_rvalid_o
);

   localparam AXI_ID_W = 4;
   localparam AXI_LEN_W = 8;
   localparam AXI_ADDR_W = `DDR_ADDR_W;
   localparam AXI_DATA_W = `DDR_DATA_W;

   wire          uart_txd_o;
   wire          uart_rxd_i;
   wire          uart_cts_i;
   wire          uart_rts_o;
   wire [12-1:0] VERSAT0_ext_dp_addr_0_port_0_o;
   wire [32-1:0] VERSAT0_ext_dp_out_0_port_0_o;
   wire [32-1:0] VERSAT0_ext_dp_in_0_port_0_i;
   wire          VERSAT0_ext_dp_enable_0_port_0_o;
   wire          VERSAT0_ext_dp_write_0_port_0_o;
   wire [12-1:0] VERSAT0_ext_dp_addr_0_port_1_o;
   wire [32-1:0] VERSAT0_ext_dp_out_0_port_1_o;
   wire [32-1:0] VERSAT0_ext_dp_in_0_port_1_i;
   wire          VERSAT0_ext_dp_enable_0_port_1_o;
   wire          VERSAT0_ext_dp_write_0_port_1_o;
   wire [12-1:0] VERSAT0_ext_dp_addr_1_port_0_o;
   wire [32-1:0] VERSAT0_ext_dp_out_1_port_0_o;
   wire [32-1:0] VERSAT0_ext_dp_in_1_port_0_i;
   wire          VERSAT0_ext_dp_enable_1_port_0_o;
   wire          VERSAT0_ext_dp_write_1_port_0_o;
   wire [12-1:0] VERSAT0_ext_dp_addr_1_port_1_o;
   wire [32-1:0] VERSAT0_ext_dp_out_1_port_1_o;
   wire [32-1:0] VERSAT0_ext_dp_in_1_port_1_i;
   wire          VERSAT0_ext_dp_enable_1_port_1_o;
   wire          VERSAT0_ext_dp_write_1_port_1_o;

   //DDR AXI interface signals
`ifdef IOB_SOC_VERSAT_USE_EXTMEM
   // Wires for the system and its peripherals
   wire [AXI_ID_W-1:0] axi_awid;  //Address write channel ID.
   wire [AXI_ADDR_W-1:0] axi_awaddr;  //Address write channel address.
   wire [AXI_LEN_W-1:0] axi_awlen;  //Address write channel burst length.
   wire [3-1:0] axi_awsize; //Address write channel burst size. This signal indicates the size of each transfer in the burst.
   wire [2-1:0] axi_awburst;  //Address write channel burst type.
   wire [2-1:0] axi_awlock;  //Address write channel lock type.
   wire [4-1:0] axi_awcache; //Address write channel memory type. Set to 0000 if master output; ignored if slave input.
   wire [3-1:0] axi_awprot; //Address write channel protection type. Set to 000 if master output; ignored if slave input.
   wire [4-1:0] axi_awqos;  //Address write channel quality of service.
   wire [1-1:0] axi_awvalid;  //Address write channel valid.
   wire [1-1:0] axi_awready;  //Address write channel ready.
   wire [AXI_DATA_W-1:0] axi_wdata;  //Write channel data.
   wire [(AXI_DATA_W/8)-1:0] axi_wstrb;  //Write channel write strobe.
   wire [1-1:0] axi_wlast;  //Write channel last word flag.
   wire [1-1:0] axi_wvalid;  //Write channel valid.
   wire [1-1:0] axi_wready;  //Write channel ready.
   wire [AXI_ID_W-1:0] axi_bid;  //Write response channel ID.
   wire [2-1:0] axi_bresp;  //Write response channel response.
   wire [1-1:0] axi_bvalid;  //Write response channel valid.
   wire [1-1:0] axi_bready;  //Write response channel ready.
   wire [AXI_ID_W-1:0] axi_arid;  //Address read channel ID.
   wire [AXI_ADDR_W-1:0] axi_araddr;  //Address read channel address.
   wire [AXI_LEN_W-1:0] axi_arlen;  //Address read channel burst length.
   wire [3-1:0] axi_arsize; //Address read channel burst size. This signal indicates the size of each transfer in the burst.
   wire [2-1:0] axi_arburst;  //Address read channel burst type.
   wire [2-1:0] axi_arlock;  //Address read channel lock type.
   wire [4-1:0] axi_arcache; //Address read channel memory type. Set to 0000 if master output; ignored if slave input.
   wire [3-1:0] axi_arprot; //Address read channel protection type. Set to 000 if master output; ignored if slave input.
   wire [4-1:0] axi_arqos;  //Address read channel quality of service.
   wire [1-1:0] axi_arvalid;  //Address read channel valid.
   wire [1-1:0] axi_arready;  //Address read channel ready.
   wire [AXI_ID_W-1:0] axi_rid;  //Read channel ID.
   wire [AXI_DATA_W-1:0] axi_rdata;  //Read channel data.
   wire [2-1:0] axi_rresp;  //Read channel response.
   wire [1-1:0] axi_rlast;  //Read channel last word.
   wire [1-1:0] axi_rvalid;  //Read channel valid.
   wire [1-1:0] axi_rready;  //Read channel ready.
   // Wires to connect the interconnect with the memory
   wire [AXI_ID_W-1:0] memory_axi_awid;  //Address write channel ID.
   wire [AXI_ADDR_W-1:0] memory_axi_awaddr;  //Address write channel address.
   wire [AXI_LEN_W-1:0] memory_axi_awlen;  //Address write channel burst length.
   wire [3-1:0] memory_axi_awsize; //Address write channel burst size. This signal indicates the size of each transfer in the burst.
   wire [2-1:0] memory_axi_awburst;  //Address write channel burst type.
   wire [2-1:0] memory_axi_awlock;  //Address write channel lock type.
   wire [4-1:0] memory_axi_awcache; //Address write channel memory type. Set to 0000 if master output; ignored if slave input.
   wire [3-1:0] memory_axi_awprot; //Address write channel protection type. Set to 000 if master output; ignored if slave input.
   wire [4-1:0] memory_axi_awqos;  //Address write channel quality of service.
   wire [1-1:0] memory_axi_awvalid;  //Address write channel valid.
   wire [1-1:0] memory_axi_awready;  //Address write channel ready.
   wire [AXI_DATA_W-1:0] memory_axi_wdata;  //Write channel data.
   wire [(AXI_DATA_W/8)-1:0] memory_axi_wstrb;  //Write channel write strobe.
   wire [1-1:0] memory_axi_wlast;  //Write channel last word flag.
   wire [1-1:0] memory_axi_wvalid;  //Write channel valid.
   wire [1-1:0] memory_axi_wready;  //Write channel ready.
   wire [AXI_ID_W-1:0] memory_axi_bid;  //Write response channel ID.
   wire [2-1:0] memory_axi_bresp;  //Write response channel response.
   wire [1-1:0] memory_axi_bvalid;  //Write response channel valid.
   wire [1-1:0] memory_axi_bready;  //Write response channel ready.
   wire [AXI_ID_W-1:0] memory_axi_arid;  //Address read channel ID.
   wire [AXI_ADDR_W-1:0] memory_axi_araddr;  //Address read channel address.
   wire [AXI_LEN_W-1:0] memory_axi_arlen;  //Address read channel burst length.
   wire [3-1:0] memory_axi_arsize; //Address read channel burst size. This signal indicates the size of each transfer in the burst.
   wire [2-1:0] memory_axi_arburst;  //Address read channel burst type.
   wire [2-1:0] memory_axi_arlock;  //Address read channel lock type.
   wire [4-1:0] memory_axi_arcache; //Address read channel memory type. Set to 0000 if master output; ignored if slave input.
   wire [3-1:0] memory_axi_arprot; //Address read channel protection type. Set to 000 if master output; ignored if slave input.
   wire [4-1:0] memory_axi_arqos;  //Address read channel quality of service.
   wire [1-1:0] memory_axi_arvalid;  //Address read channel valid.
   wire [1-1:0] memory_axi_arready;  //Address read channel ready.
   wire [AXI_ID_W-1:0] memory_axi_rid;  //Read channel ID.
   wire [AXI_DATA_W-1:0] memory_axi_rdata;  //Read channel data.
   wire [2-1:0] memory_axi_rresp;  //Read channel response.
   wire [1-1:0] memory_axi_rlast;  //Read channel last word.
   wire [1-1:0] memory_axi_rvalid;  //Read channel valid.
   wire [1-1:0] memory_axi_rready;  //Read channel ready.
`endif

   /////////////////////////////////////////////
   // TEST PROCEDURE
   //
   initial begin
`ifdef VCD
      $dumpfile("uut.vcd");
      $dumpvars();
`endif
   end

   //
   // INSTANTIATE COMPONENTS
   //

   `include "versat_external_memory_inst.vh"

   //
   // IOb-SoC (may also include Unit Under Test)
   //
   iob_soc_versat #(
      .AXI_ID_W  (AXI_ID_W),
      .AXI_LEN_W (AXI_LEN_W),
      .AXI_ADDR_W(AXI_ADDR_W),
      .AXI_DATA_W(AXI_DATA_W)
   ) iob_soc_versat0 (
      .uart_txd_o                      (uart_txd_o),
      .uart_rxd_i                      (uart_rxd_i),
      .uart_cts_i                      (uart_cts_i),
      .uart_rts_o                      (uart_rts_o),
      `include "iob_soc_versat_pportmaps.vs"

`ifdef IOB_SOC_VERSAT_USE_EXTMEM
      .axi_awid_o(axi_awid),  //Address write channel ID.
      .axi_awaddr_o(axi_awaddr),  //Address write channel address.
      .axi_awlen_o(axi_awlen),  //Address write channel burst length.
      .axi_awsize_o(axi_awsize), //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .axi_awburst_o(axi_awburst),  //Address write channel burst type.
      .axi_awlock_o(axi_awlock),  //Address write channel lock type.
      .axi_awcache_o(axi_awcache), //Address write channel memory type. Set to 0000 if master output; ignored if slave input.
      .axi_awprot_o(axi_awprot), //Address write channel protection type. Set to 000 if master output; ignored if slave input.
      .axi_awqos_o(axi_awqos),  //Address write channel quality of service.
      .axi_awvalid_o(axi_awvalid),  //Address write channel valid.
      .axi_awready_i(axi_awready),  //Address write channel ready.
      .axi_wdata_o(axi_wdata),  //Write channel data.
      .axi_wstrb_o(axi_wstrb),  //Write channel write strobe.
      .axi_wlast_o(axi_wlast),  //Write channel last word flag.
      .axi_wvalid_o(axi_wvalid),  //Write channel valid.
      .axi_wready_i(axi_wready),  //Write channel ready.
      .axi_bid_i(axi_bid),  //Write response channel ID.
      .axi_bresp_i(axi_bresp),  //Write response channel response.
      .axi_bvalid_i(axi_bvalid),  //Write response channel valid.
      .axi_bready_o(axi_bready),  //Write response channel ready.
      .axi_arid_o(axi_arid),  //Address read channel ID.
      .axi_araddr_o(axi_araddr),  //Address read channel address.
      .axi_arlen_o(axi_arlen),  //Address read channel burst length.
      .axi_arsize_o(axi_arsize), //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .axi_arburst_o(axi_arburst),  //Address read channel burst type.
      .axi_arlock_o(axi_arlock),  //Address read channel lock type.
      .axi_arcache_o(axi_arcache), //Address read channel memory type. Set to 0000 if master output; ignored if slave input.
      .axi_arprot_o(axi_arprot), //Address read channel protection type. Set to 000 if master output; ignored if slave input.
      .axi_arqos_o(axi_arqos),  //Address read channel quality of service.
      .axi_arvalid_o(axi_arvalid),  //Address read channel valid.
      .axi_arready_i(axi_arready),  //Address read channel ready.
      .axi_rid_i(axi_rid),  //Read channel ID.
      .axi_rdata_i(axi_rdata),  //Read channel data.
      .axi_rresp_i(axi_rresp),  //Read channel response.
      .axi_rlast_i(axi_rlast),  //Read channel last word.
      .axi_rvalid_i(axi_rvalid),  //Read channel valid.
      .axi_rready_o(axi_rready),  //Read channel ready.
`endif
      .clk_i(clk_i),
      .cke_i(1'b1),
      .arst_i(arst_i),
      .trap_o(trap_o)
   );

   // interconnect clk and arst
   wire clk_interconnect;
   wire arst_interconnect;
   assign clk_interconnect  = clk_i;
   assign arst_interconnect = arst_i;

`include "iob_soc_versat_interconnect.vs"

   //instantiate the axi memory
   //IOb-SoC and SUT access the same memory.
   axi_ram #(
`ifdef IOB_SOC_VERSAT_INIT_MEM
      .FILE      ("init_ddr_contents.hex"),  //This file contains firmware for both systems
      .FILE_SIZE (2 ** (AXI_ADDR_W - 2)),
`endif
      .ID_WIDTH  (AXI_ID_W),
      .DATA_WIDTH(AXI_DATA_W),
      .ADDR_WIDTH(AXI_ADDR_W)
   ) ddr_model_mem (
      `include "iob_memory_axi_s_portmap.vs"

      .clk_i(clk_i),
      .rst_i(arst_i)
   );

   //Manually added testbench uart core. RS232 pins attached to the same pins
   //of the iob_soc_versat UART0 instance to communicate with it
   // The interface of iob_soc_versat UART0 is assumed to be the first portmapped interface (UART_*)
   iob_uart uart_tb (
      .clk_i (clk_i),
      .cke_i (1'b1),
      .arst_i(arst_i),

      .iob_valid_i (uart_valid_i),
      .iob_addr_i  (uart_addr_i),
      .iob_wdata_i (uart_wdata_i),
      .iob_wstrb_i (uart_wstrb_i),
      .iob_rdata_o (uart_rdata_o),
      .iob_rvalid_o(uart_rvalid_o),
      .iob_ready_o (uart_ready_o),

      .txd_o(uart_rxd_i),
      .rxd_i(uart_txd_o),
      .rts_o(uart_cts_i),
      .cts_i(uart_rts_o)
   );

   //Ethernet
`ifdef IOB_SOC_VERSAT_USE_ETHERNET
   //ethernet clock: 4x slower than system clock
   reg [1:0] eth_cnt = 2'b0;
   reg       eth_clk;

   always @(posedge clk_i) begin
      eth_cnt <= eth_cnt + 1'b1;
      eth_clk <= eth_cnt[1];
   end

   // Ethernet Interface signals
   assign ETH0_MRxClk = eth_clk;
   assign ETH0_MTxClk = eth_clk;

   //Manually added testbench ethernet core. MII pins attached to the same pins
   //of the iob_soc_versat ETH0 instance to communicate with it
   // The interface of iob_soc_versat ETH0 is assumed to be the first portmapped interface (ETH_*)
   iob_eth #(
      .AXI_ID_W  (AXI_ID_W),
      .AXI_ADDR_W(AXI_ADDR_W),
      .AXI_DATA_W(AXI_DATA_W),
      .AXI_LEN_W (AXI_LEN_W)
   ) eth_tb (
      .inta_o       (),
      .MTxClk       (eth_clk),
      .MTxD         (ETH0_MRxD),
      .MTxEn        (ETH0_MRxDv),
      .MTxErr       (ETH0_MRxErr),
      .MRxClk       (eth_clk),
      .MRxDv        (ETH0_MTxEn),
      .MRxD         (ETH0_MTxD),
      .MRxErr       (ETH0_MTxErr),
      .MColl        (1'b0),
      .MCrS         (1'b0),
      .MDC          (),
      .MDIO         (),
      .iob_valid_i  (ethernet_valid_i),
      .iob_addr_i   (ethernet_addr_i),
      .iob_wdata_i  (ethernet_wdata_i),
      .iob_wstrb_i  (ethernet_wstrb_i),
      .iob_rvalid_o (ethernet_rvalid_o),
      .iob_rdata_o  (ethernet_rdata_o),
      .iob_ready_o  (ethernet_ready_o),
      .axi_awid_o   (),
      .axi_awaddr_o (),
      .axi_awlen_o  (),
      .axi_awsize_o (),
      .axi_awburst_o(),
      .axi_awlock_o (),
      .axi_awcache_o(),
      .axi_awprot_o (),
      .axi_awqos_o  (),
      .axi_awvalid_o(),
      .axi_awready_i(1'b0),
      .axi_wdata_o  (),
      .axi_wstrb_o  (),
      .axi_wlast_o  (),
      .axi_wvalid_o (),
      .axi_wready_i (1'b0),
      .axi_bid_i    ({AXI_ID_W{1'b0}}),
      .axi_bresp_i  (2'b0),
      .axi_bvalid_i (1'b0),
      .axi_bready_o (),
      .axi_arid_o   (),
      .axi_araddr_o (),
      .axi_arlen_o  (),
      .axi_arsize_o (),
      .axi_arburst_o(),
      .axi_arlock_o (),
      .axi_arcache_o(),
      .axi_arprot_o (),
      .axi_arqos_o  (),
      .axi_arvalid_o(),
      .axi_arready_i(1'b0),
      .axi_rid_i    ({AXI_ID_W{1'b0}}),
      .axi_rdata_i  ({AXI_DATA_W{1'b0}}),
      .axi_rresp_i  (2'b0),
      .axi_rlast_i  (1'b0),
      .axi_rvalid_i (1'b0),
      .axi_rready_o (),
      .clk_i        (clk_i),
      .arst_i       (arst_i),
      .cke_i        (1'b1)
   );
`endif

endmodule
