#!/usr/bin/env python3
import os
import sys
import shutil

from mk_configuration import update_define
from verilog_tools import inplace_change

from iob_soc import iob_soc
from iob_module import iob_module
from iob_vexriscv import iob_vexriscv
from iob_uart import iob_uart

from iob_versat import CreateVersatClass
from axil2iob import axil2iob
from iob2axil import iob2axil
from iob_reset_sync import iob_reset_sync

VERSAT_SPEC = "versatSpec.txt"
VERSAT_TOP = "Test"
VERSAT_EXTRA_UNITS = os.path.realpath(
    os.path.join(os.path.dirname(__file__), "hardware/src/units")
)

print("IOB_SOC_VERSAT", file=sys.stderr)


def GetTestName():
    # Check for test type
    testName = "M_Stage"  # Default test
    for arg in sys.argv[1:]:
        if arg[:5] == "TEST=":
            testName = arg[5:]

    return testName


def GetBuildDir(name):
    testName = GetTestName()

    # TODO: Remove default test and use the version string if not running a test
    return os.path.realpath(f"../{name}_V0.70_{testName}")


pc_emul = False
for arg in sys.argv[1:]:
    if arg == "PC_EMUL":
        pc_emul = True


class iob_soc_versat(iob_soc):
    name = "iob_soc_versat"
    version = "V0.70"
    flows = "pc-emul emb sim doc fpga"
    setup_dir = os.path.dirname(__file__)
    build_dir = GetBuildDir(name)

    @classmethod
    def _create_instances(cls):
        super()._create_instances()

        # if iob_vexriscv in cls.submodule_list:
        #    cls.cpu = iob_vexriscv("cpu_0")
        if cls.versat_type in cls.submodule_list:
            cls.versat = cls.versat_type("VERSAT0", "Versat accelerator")
            cls.peripherals.append(cls.versat)

    @classmethod
    def _create_submodules_list(cls, extra_submodules=[]):
        """Create submodules list with dependencies of this module"""

        cls.versat_type = CreateVersatClass(
            pc_emul,
            VERSAT_SPEC,
            GetTestName(),
            VERSAT_EXTRA_UNITS,
            GetBuildDir("iob_soc_versat"),
            os.path.realpath(cls.setup_dir + "../debug/"),
        )

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
                cls.versat_type,
                iob_reset_sync,
            ]
            + extra_submodules
        )

        # Remove iob_picorv32 because we want vexriscv
        # i = 0
        # while i < len(cls.submodule_list):
        #    if type(cls.submodule_list[i]) == type and cls.submodule_list[i].name in [
        #        "iob_picorv32",
        #        # "iob_cache",
        #    ]:
        #        cls.submodule_list.pop(i)
        #        continue
        #    i += 1

    @classmethod
    def _post_setup(cls):
        super()._post_setup()

        shutil.copy(
            f"{cls.build_dir}/software/src/Tests/{GetTestName()}.cpp",
            f"{cls.build_dir}/software/src/test.cpp",
        )

        shutil.copy(
            f"{cls.build_dir}/software/src/Tests/testbench.hpp",
            f"{cls.build_dir}/software/src/",
        )

        shutil.copy(
            f"{cls.build_dir}/software/src/Tests/unitConfiguration.hpp",
            f"{cls.build_dir}/software/src/",
        )

        shutil.copy(
            f"{cls.build_dir}/software/src/Tests/simpleTest.hpp",
            f"{cls.build_dir}/software/src/",
        )

        shutil.copytree(
            f"{cls.setup_dir}/hardware/src/units",
            f"{cls.build_dir}/hardware/src",
            dirs_exist_ok=True,
        )

        shutil.rmtree(f"{cls.build_dir}/software/src/Tests")

    @classmethod
    def _setup_confs(cls, extra_confs=[]):
        # Append confs or override them if they exist

        confs = [
            {
                "name": "SRAM_ADDR_W",
                "type": "P",
                "val": "18",
                "min": "1",
                "max": "32",
                "descr": "SRAM address width",
            }
        ]

        if cls.versat_type.USE_EXTMEM:
            print("USING EXT MEM BECAUSE VERSAT", file=sys.stderr)
            confs.append(
                {
                    "name": "USE_EXTMEM",
                    "type": "M",
                    "val": True,
                    "min": "0",
                    "max": "1",
                    "descr": "Versat AXI implies External memory",
                }
            )

        super()._setup_confs(confs)
