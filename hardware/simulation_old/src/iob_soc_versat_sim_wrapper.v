`timescale 1ns / 1ps

`include "bsp.vh"
`include "iob_soc_versat_conf.vh"
`include "versat_defs.vh"

//Peripherals _swreg_def.vh file includes.
`include "iob_uart_swreg_def.vh"
`include "iob_versat_swreg_def.vh"

`ifndef IOB_UART_SWREG_ADDR_W
`define IOB_UART_SWREG_ADDR_W 16
`endif
`ifndef IOB_SOC_VERSAT_DATA_W
`define IOB_SOC_VERSAT_DATA_W 32
`endif

module iob_soc_versat_sim_wrapper (
   output trap_o,
   //IOb-SoC uart
   input uart_avalid,
   input [`IOB_UART_SWREG_ADDR_W-1:0] uart_addr,
   input [`IOB_SOC_VERSAT_DATA_W-1:0] uart_wdata,
   input [3:0] uart_wstrb,
   output [`IOB_SOC_VERSAT_DATA_W-1:0] uart_rdata,
   output uart_ready,
   output uart_rvalid,
   input [1-1:0] clk_i,  //V2TEX_IO System clock input.
   input [1-1:0] rst_i  //V2TEX_IO System reset, asynchronous and active high.
);

   localparam AXI_ID_W = 4;
   localparam AXI_LEN_W = 8;
   localparam AXI_ADDR_W = `DDR_ADDR_W;
   localparam AXI_DATA_W = `DDR_DATA_W;

   wire clk = clk_i;
   wire arst = rst_i;

   `include "iob_soc_versat_wrapper_pwires.vs"

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
      `include "iob_soc_versat_pportmaps.vs"

      .clk_i(clk),
      .cke_i(1'b1),
      .arst_i(arst),
      .trap_o(trap_o)
   );

`include "iob_soc_versat_interconnect.vs"

/*
`ifdef IOB_SOC_VERSAT_USE_EXTMEM
`ifdef VERSAT_IO
   //instantiate axi interconnect
   axi_interconnect #(
      .ID_WIDTH    (AXI_ID_W),
      .DATA_WIDTH  (AXI_DATA_W),
      .ADDR_WIDTH  (AXI_ADDR_W),
      .M_ADDR_WIDTH(AXI_ADDR_W),
      .S_COUNT     (2),
      .M_COUNT     (1)
   ) system_axi_interconnect (
      .clk(clk),
      .rst(arst),

      // Need to use manually defined connections because awlock and arlock of interconnect is only on bit for each slave
      .s_axi_awid(axi_awid),  //Address write channel ID.
      .s_axi_awaddr(axi_awaddr),  //Address write channel address.
      .s_axi_awlen(axi_awlen),  //Address write channel burst length.
      .s_axi_awsize  (axi_awsize),  //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .s_axi_awburst(axi_awburst),  //Address write channel burst type.
      .s_axi_awlock({axi_awlock[2], axi_awlock[0]}),  //Address write channel lock type.
      .s_axi_awcache (axi_awcache), //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .s_axi_awprot  (axi_awprot),  //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .s_axi_awqos(axi_awqos),  //Address write channel quality of service.
      .s_axi_awvalid(axi_awvalid),  //Address write channel valid.
      .s_axi_awready(axi_awready),  //Address write channel ready.
      .s_axi_wdata(axi_wdata),  //Write channel data.
      .s_axi_wstrb(axi_wstrb),  //Write channel write strobe.
      .s_axi_wlast(axi_wlast),  //Write channel last word flag.
      .s_axi_wvalid(axi_wvalid),  //Write channel valid.
      .s_axi_wready(axi_wready),  //Write channel ready.
      .s_axi_bid(axi_bid),  //Write response channel ID.
      .s_axi_bresp(axi_bresp),  //Write response channel response.
      .s_axi_bvalid(axi_bvalid),  //Write response channel valid.
      .s_axi_bready(axi_bready),  //Write response channel ready.
      .s_axi_arid(axi_arid),  //Address read channel ID.
      .s_axi_araddr(axi_araddr),  //Address read channel address.
      .s_axi_arlen(axi_arlen),  //Address read channel burst length.
      .s_axi_arsize  (axi_arsize),  //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .s_axi_arburst(axi_arburst),  //Address read channel burst type.
      .s_axi_arlock({axi_arlock[2], axi_arlock[0]}),  //Address read channel lock type.
      .s_axi_arcache (axi_arcache), //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .s_axi_arprot  (axi_arprot),  //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .s_axi_arqos(axi_arqos),  //Address read channel quality of service.
      .s_axi_arvalid(axi_arvalid),  //Address read channel valid.
      .s_axi_arready(axi_arready),  //Address read channel ready.
      .s_axi_rid(axi_rid),  //Read channel ID.
      .s_axi_rdata(axi_rdata),  //Read channel data.
      .s_axi_rresp(axi_rresp),  //Read channel response.
      .s_axi_rlast(axi_rlast),  //Read channel last word.
      .s_axi_rvalid(axi_rvalid),  //Read channel valid.
      .s_axi_rready(axi_rready),  //Read channel ready.

      // Used manually defined connections because awlock and arlock of interconnect is only on bit.
      .m_axi_awid(memory_axi_awid[0+:AXI_ID_W]),  //Address write channel ID.
      .m_axi_awaddr(memory_axi_awaddr[0+:AXI_ADDR_W]),  //Address write channel address.
      .m_axi_awlen(memory_axi_awlen[0+:AXI_LEN_W]),  //Address write channel burst length.
      .m_axi_awsize  (memory_axi_awsize[0+:3]),              //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .m_axi_awburst(memory_axi_awburst[0+:2]),  //Address write channel burst type.
      .m_axi_awlock(memory_axi_awlock[0+:1]),  //Address write channel lock type.
      .m_axi_awcache (memory_axi_awcache[0+:4]),             //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .m_axi_awprot  (memory_axi_awprot[0+:3]),              //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .m_axi_awqos(memory_axi_awqos[0+:4]),  //Address write channel quality of service.
      .m_axi_awvalid(memory_axi_awvalid[0+:1]),  //Address write channel valid.
      .m_axi_awready(memory_axi_awready[0+:1]),  //Address write channel ready.
      .m_axi_wdata(memory_axi_wdata[0+:AXI_DATA_W]),  //Write channel data.
      .m_axi_wstrb(memory_axi_wstrb[0+:(AXI_DATA_W/8)]),  //Write channel write strobe.
      .m_axi_wlast(memory_axi_wlast[0+:1]),  //Write channel last word flag.
      .m_axi_wvalid(memory_axi_wvalid[0+:1]),  //Write channel valid.
      .m_axi_wready(memory_axi_wready[0+:1]),  //Write channel ready.
      .m_axi_bid(memory_axi_bid[0+:AXI_ID_W]),  //Write response channel ID.
      .m_axi_bresp(memory_axi_bresp[0+:2]),  //Write response channel response.
      .m_axi_bvalid(memory_axi_bvalid[0+:1]),  //Write response channel valid.
      .m_axi_bready(memory_axi_bready[0+:1]),  //Write response channel ready.
      .m_axi_arid(memory_axi_arid[0+:AXI_ID_W]),  //Address read channel ID.
      .m_axi_araddr(memory_axi_araddr[0+:AXI_ADDR_W]),  //Address read channel address.
      .m_axi_arlen(memory_axi_arlen[0+:AXI_LEN_W]),  //Address read channel burst length.
      .m_axi_arsize  (memory_axi_arsize[0+:3]),              //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .m_axi_arburst(memory_axi_arburst[0+:2]),  //Address read channel burst type.
      .m_axi_arlock(memory_axi_arlock[0+:1]),  //Address read channel lock type.
      .m_axi_arcache (memory_axi_arcache[0+:4]),             //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .m_axi_arprot  (memory_axi_arprot[0+:3]),              //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .m_axi_arqos(memory_axi_arqos[0+:4]),  //Address read channel quality of service.
      .m_axi_arvalid(memory_axi_arvalid[0+:1]),  //Address read channel valid.
      .m_axi_arready(memory_axi_arready[0+:1]),  //Address read channel ready.
      .m_axi_rid(memory_axi_rid[0+:AXI_ID_W]),  //Read channel ID.
      .m_axi_rdata(memory_axi_rdata[0+:AXI_DATA_W]),  //Read channel data.
      .m_axi_rresp(memory_axi_rresp[0+:2]),  //Read channel response.
      .m_axi_rlast(memory_axi_rlast[0+:1]),  //Read channel last word.
      .m_axi_rvalid(memory_axi_rvalid[0+:1]),  //Read channel valid.
      .m_axi_rready(memory_axi_rready[0+:1]),  //Read channel ready.

      //optional signals
      .s_axi_awuser  (2'b0),
      .s_axi_wuser   (2'b0),
      .s_axi_aruser  (2'b0),
      .s_axi_buser   (),
      .s_axi_ruser   (),
      .m_axi_awuser  (),
      .m_axi_wuser   (),
      .m_axi_aruser  (),
      .m_axi_buser   (1'b0),
      .m_axi_ruser   (1'b0),
      .m_axi_awregion(),
      .m_axi_arregion()
   );
`else
   //instantiate axi interconnect
   axi_interconnect #(
      .ID_WIDTH    (AXI_ID_W),
      .DATA_WIDTH  (AXI_DATA_W),
      .ADDR_WIDTH  (AXI_ADDR_W),
      .M_ADDR_WIDTH(AXI_ADDR_W),
      .S_COUNT     (1),
      .M_COUNT     (1)
   ) system_axi_interconnect (
      .clk(clk),
      .rst(arst),

      // Need to use manually defined connections because awlock and arlock of interconnect is only on bit for each slave
      .s_axi_awid(axi_awid),  //Address write channel ID.
      .s_axi_awaddr(axi_awaddr),  //Address write channel address.
      .s_axi_awlen(axi_awlen),  //Address write channel burst length.
      .s_axi_awsize  (axi_awsize),  //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .s_axi_awburst(axi_awburst),  //Address write channel burst type.
      .s_axi_awlock({axi_awlock[0]}),  //Address write channel lock type.
      .s_axi_awcache (axi_awcache), //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .s_axi_awprot  (axi_awprot),  //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .s_axi_awqos(axi_awqos),  //Address write channel quality of service.
      .s_axi_awvalid(axi_awvalid),  //Address write channel valid.
      .s_axi_awready(axi_awready),  //Address write channel ready.
      .s_axi_wdata(axi_wdata),  //Write channel data.
      .s_axi_wstrb(axi_wstrb),  //Write channel write strobe.
      .s_axi_wlast(axi_wlast),  //Write channel last word flag.
      .s_axi_wvalid(axi_wvalid),  //Write channel valid.
      .s_axi_wready(axi_wready),  //Write channel ready.
      .s_axi_bid(axi_bid),  //Write response channel ID.
      .s_axi_bresp(axi_bresp),  //Write response channel response.
      .s_axi_bvalid(axi_bvalid),  //Write response channel valid.
      .s_axi_bready(axi_bready),  //Write response channel ready.
      .s_axi_arid(axi_arid),  //Address read channel ID.
      .s_axi_araddr(axi_araddr),  //Address read channel address.
      .s_axi_arlen(axi_arlen),  //Address read channel burst length.
      .s_axi_arsize  (axi_arsize),  //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .s_axi_arburst(axi_arburst),  //Address read channel burst type.
      .s_axi_arlock({axi_arlock[0]}),  //Address read channel lock type.
      .s_axi_arcache (axi_arcache), //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .s_axi_arprot  (axi_arprot),  //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .s_axi_arqos(axi_arqos),  //Address read channel quality of service.
      .s_axi_arvalid(axi_arvalid),  //Address read channel valid.
      .s_axi_arready(axi_arready),  //Address read channel ready.
      .s_axi_rid(axi_rid),  //Read channel ID.
      .s_axi_rdata(axi_rdata),  //Read channel data.
      .s_axi_rresp(axi_rresp),  //Read channel response.
      .s_axi_rlast(axi_rlast),  //Read channel last word.
      .s_axi_rvalid(axi_rvalid),  //Read channel valid.
      .s_axi_rready(axi_rready),  //Read channel ready.

      // Used manually defined connections because awlock and arlock of interconnect is only on bit.
      .m_axi_awid(memory_axi_awid[0+:AXI_ID_W]),  //Address write channel ID.
      .m_axi_awaddr(memory_axi_awaddr[0+:AXI_ADDR_W]),  //Address write channel address.
      .m_axi_awlen(memory_axi_awlen[0+:AXI_LEN_W]),  //Address write channel burst length.
      .m_axi_awsize  (memory_axi_awsize[0+:3]),              //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .m_axi_awburst(memory_axi_awburst[0+:2]),  //Address write channel burst type.
      .m_axi_awlock(memory_axi_awlock[0+:1]),  //Address write channel lock type.
      .m_axi_awcache (memory_axi_awcache[0+:4]),             //Address write channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .m_axi_awprot  (memory_axi_awprot[0+:3]),              //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .m_axi_awqos(memory_axi_awqos[0+:4]),  //Address write channel quality of service.
      .m_axi_awvalid(memory_axi_awvalid[0+:1]),  //Address write channel valid.
      .m_axi_awready(memory_axi_awready[0+:1]),  //Address write channel ready.
      .m_axi_wdata(memory_axi_wdata[0+:AXI_DATA_W]),  //Write channel data.
      .m_axi_wstrb(memory_axi_wstrb[0+:(AXI_DATA_W/8)]),  //Write channel write strobe.
      .m_axi_wlast(memory_axi_wlast[0+:1]),  //Write channel last word flag.
      .m_axi_wvalid(memory_axi_wvalid[0+:1]),  //Write channel valid.
      .m_axi_wready(memory_axi_wready[0+:1]),  //Write channel ready.
      .m_axi_bid(memory_axi_bid[0+:AXI_ID_W]),  //Write response channel ID.
      .m_axi_bresp(memory_axi_bresp[0+:2]),  //Write response channel response.
      .m_axi_bvalid(memory_axi_bvalid[0+:1]),  //Write response channel valid.
      .m_axi_bready(memory_axi_bready[0+:1]),  //Write response channel ready.
      .m_axi_arid(memory_axi_arid[0+:AXI_ID_W]),  //Address read channel ID.
      .m_axi_araddr(memory_axi_araddr[0+:AXI_ADDR_W]),  //Address read channel address.
      .m_axi_arlen(memory_axi_arlen[0+:AXI_LEN_W]),  //Address read channel burst length.
      .m_axi_arsize  (memory_axi_arsize[0+:3]),              //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .m_axi_arburst(memory_axi_arburst[0+:2]),  //Address read channel burst type.
      .m_axi_arlock(memory_axi_arlock[0+:1]),  //Address read channel lock type.
      .m_axi_arcache (memory_axi_arcache[0+:4]),             //Address read channel memory type. Transactions set with Normal, Non-cacheable, Modifiable, and Bufferable (0011).
      .m_axi_arprot  (memory_axi_arprot[0+:3]),              //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
      .m_axi_arqos(memory_axi_arqos[0+:4]),  //Address read channel quality of service.
      .m_axi_arvalid(memory_axi_arvalid[0+:1]),  //Address read channel valid.
      .m_axi_arready(memory_axi_arready[0+:1]),  //Address read channel ready.
      .m_axi_rid(memory_axi_rid[0+:AXI_ID_W]),  //Read channel ID.
      .m_axi_rdata(memory_axi_rdata[0+:AXI_DATA_W]),  //Read channel data.
      .m_axi_rresp(memory_axi_rresp[0+:2]),  //Read channel response.
      .m_axi_rlast(memory_axi_rlast[0+:1]),  //Read channel last word.
      .m_axi_rvalid(memory_axi_rvalid[0+:1]),  //Read channel valid.
      .m_axi_rready(memory_axi_rready[0+:1]),  //Read channel ready.

      //optional signals
      .s_axi_awuser  (1'b0),
      .s_axi_wuser   (1'b0),
      .s_axi_aruser  (1'b0),
      .s_axi_buser   (),
      .s_axi_ruser   (),
      .m_axi_awuser  (),
      .m_axi_wuser   (),
      .m_axi_aruser  (),
      .m_axi_buser   (1'b0),
      .m_axi_ruser   (1'b0),
      .m_axi_awregion(),
      .m_axi_arregion()
   );
`endif
`endif
*/


`ifdef IOB_SOC_VERSAT_USE_EXTMEM
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
      .axi_awid_i(memory_axi_awid),  //Address write channel ID.
      .axi_awaddr_i(memory_axi_awaddr),  //Address write channel address.
      .axi_awlen_i(memory_axi_awlen),  //Address write channel burst length.
      .axi_awsize_i(memory_axi_awsize), //Address write channel burst size. This signal indicates the size of each transfer in the burst.
      .axi_awburst_i(memory_axi_awburst),  //Address write channel burst type.
      .axi_awlock_i(memory_axi_awlock),  //Address write channel lock type.
      .axi_awcache_i(memory_axi_awcache), //Address write channel memory type. Set to 0000 if master output; ignored if slave input.
      .axi_awprot_i(memory_axi_awprot), //Address write channel protection type. Set to 000 if master output; ignored if slave input.
      .axi_awqos_i(memory_axi_awqos),  //Address write channel quality of service.
      .axi_awvalid_i(memory_axi_awvalid),  //Address write channel valid.
      .axi_awready_o(memory_axi_awready),  //Address write channel ready.
      .axi_wdata_i(memory_axi_wdata),  //Write channel data.
      .axi_wstrb_i(memory_axi_wstrb),  //Write channel write strobe.
      .axi_wlast_i(memory_axi_wlast),  //Write channel last word flag.
      .axi_wvalid_i(memory_axi_wvalid),  //Write channel valid.
      .axi_wready_o(memory_axi_wready),  //Write channel ready.
      .axi_bid_o(memory_axi_bid),  //Write response channel ID.
      .axi_bresp_o(memory_axi_bresp),  //Write response channel response.
      .axi_bvalid_o(memory_axi_bvalid),  //Write response channel valid.
      .axi_bready_i(memory_axi_bready),  //Write response channel ready.
      .axi_arid_i(memory_axi_arid),  //Address read channel ID.
      .axi_araddr_i(memory_axi_araddr),  //Address read channel address.
      .axi_arlen_i(memory_axi_arlen),  //Address read channel burst length.
      .axi_arsize_i(memory_axi_arsize), //Address read channel burst size. This signal indicates the size of each transfer in the burst.
      .axi_arburst_i(memory_axi_arburst),  //Address read channel burst type.
      .axi_arlock_i(memory_axi_arlock),  //Address read channel lock type.
      .axi_arcache_i(memory_axi_arcache), //Address read channel memory type. Set to 0000 if master output; ignored if slave input.
      .axi_arprot_i(memory_axi_arprot), //Address read channel protection type. Set to 000 if master output; ignored if slave input.
      .axi_arqos_i(memory_axi_arqos),  //Address read channel quality of service.
      .axi_arvalid_i(memory_axi_arvalid),  //Address read channel valid.
      .axi_arready_o(memory_axi_arready),  //Address read channel ready.
      .axi_rid_o(memory_axi_rid),  //Read channel ID.
      .axi_rdata_o(memory_axi_rdata),  //Read channel data.
      .axi_rresp_o(memory_axi_rresp),  //Read channel response.
      .axi_rlast_o(memory_axi_rlast),  //Read channel last word.
      .axi_rvalid_o(memory_axi_rvalid),  //Read channel valid.
      .axi_rready_i(memory_axi_rready),  //Read channel ready.

      .clk_i(clk),
      .rst_i(arst)
   );
`endif

   //Manually added testbench uart core. RS232 pins attached to the same pins
   //of the iob_soc_versat UART0 instance to communicate with it
   // The interface of iob_soc_versat UART0 is assumed to be the first portmapped interface (UART_*)
   wire cke = 1'b1;
   iob_uart uart_tb (
      .clk_i (clk),
      .cke_i (cke),
      .arst_i(arst),

      .iob_avalid_i(uart_avalid),
      .iob_addr_i  (uart_addr),
      .iob_wdata_i (uart_wdata),
      .iob_wstrb_i (uart_wstrb),
      .iob_rdata_o (uart_rdata),
      .iob_rvalid_o(uart_rvalid),
      .iob_ready_o (uart_ready),

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

   always @(posedge clk) begin
      eth_cnt <= eth_cnt + 1'b1;
      eth_clk <= eth_cnt[1];
   end

   // Ethernet Interface signals
   assign ETHERNET0_MRxClk = eth_clk;
   assign ETHERNET0_MTxClk = eth_clk;

   //add core test module in testbench
   iob_eth_tb_gen eth_tb (
      .clk  (clk),
      .reset(arst),

      // This module acts like a loopback
      .MRxClk(ETHERNET0_MTxClk),
      .MRxD  (ETHERNET0_MTxD),
      .MRxDv (ETHERNET0_MTxEn),
      .MRxErr(ETHERNET0_MTxErr),

      // The wires are thus reversed
      .MTxClk(ETHERNET0_MRxClk),
      .MTxD  (ETHERNET0_MRxD),
      .MTxEn (ETHERNET0_MRxDv),
      .MTxErr(ETHERNET0_MRxErr),

      .MColl(1'b0),
      .MCrS (1'b0),

      .MDC (),
      .MDIO(),
   );
`endif

endmodule