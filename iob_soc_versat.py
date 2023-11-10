#!/usr/bin/env python3
import os
import sys
import shutil

from mk_configuration import update_define
from verilog_tools import inplace_change

from iob_soc import iob_soc
from iob_vexriscv import iob_vexriscv
from iob_uart import iob_uart
from iob_versat import iob_versat
from axil2iob import axil2iob
from iob_reset_sync import iob_reset_sync

VERSAT_SPEC = "versatSpec.txt"
VERSAT_TOP = "Test"
VERSAT_EXTRA_UNITS = os.path.join(os.path.dirname(__file__), "hardware/src/units")


class iob_soc_versat(iob_soc):
    name = "iob_soc_versat"
    version = "V0.70"
    flows = "pc-emul emb sim doc fpga"
    setup_dir = os.path.dirname(__file__)

    @classmethod
    def _create_instances(cls):
        super()._create_instances()
        # Verilog modules instances if we have them in the setup list (they may not be in the list if a subclass decided to remove them).
        if iob_vexriscv in cls.submodule_list:
            cls.cpu = iob_vexriscv("cpu_0")
        if iob_versat in cls.submodule_list:
            cls.versat = iob_versat(
                "VERSAT0",
                parameters={
                    "versat_spec": VERSAT_SPEC,
                    "versat_top": VERSAT_TOP,
                    "extra_units": VERSAT_EXTRA_UNITS,
                },
            )
            cls.peripherals.append(cls.versat)

    @classmethod
    def _create_submodules_list(cls, extra_submodules=[]):
        """Create submodules list with dependencies of this module"""
        super()._create_submodules_list(
            [
                {"interface": "peripheral_axi_wire"},
                {"interface": "intmem_axi_wire"},
                {"interface": "dBus_axi_wire"},
                {"interface": "iBus_axi_wire"},
                {"interface": "dBus_axi_m_port"},
                {"interface": "iBus_axi_m_port"},
                {"interface": "dBus_axi_m_portmap"},
                {"interface": "iBus_axi_m_portmap"},
                iob_vexriscv,
                iob_versat,
                axil2iob,
                iob_reset_sync,
                # (iob_uart, {"purpose": "simulation"}),
            ]
            + extra_submodules
        )
        i = 0
        while i < len(cls.submodule_list):
            if type(cls.submodule_list[i]) == type and cls.submodule_list[i].name in [
                "iob_picorv32",
                "axi_interconnect",
            ]:
                cls.submodule_list.pop(i)
                continue
            i += 1

    @classmethod
    def _setup_confs(cls, extra_confs=[]):
        # Append confs or override them if they exist
        super()._setup_confs(
            [
                {
                    "name": "INIT_MEM",
                    "type": "M",
                    "val": False,
                    "min": "0",
                    "max": "1",
                    "descr": "Used to select running linux.",
                },
                {
                    "name": "USE_EXTMEM",
                    "type": "M",
                    "val": True,
                    "min": "0",
                    "max": "1",
                    "descr": "Always use external memory in the SoC.",
                },
                {
                    "name": "N_CORES",
                    "type": "P",
                    "val": "1",
                    "min": "1",
                    "max": "32",
                    "descr": "Number of CPU cores used in the SoC.",
                },
                {
                    "name": "BOOTROM_ADDR_W",
                    "type": "P",
                    "val": "15",
                    "min": "1",
                    "max": "32",
                    "descr": "Boot ROM address width",
                },
                {
                    "name": "SRAM_ADDR_W",
                    "type": "P",
                    "val": "15",
                    "min": "1",
                    "max": "32",
                    "descr": "SRAM address width",
                },
                {
                    "name": "MEM_ADDR_W",
                    "type": "P",
                    "val": "26",
                    "min": "1",
                    "max": "32",
                    "descr": "Memory bus address width",
                },
            ]
            + extra_confs
        )
