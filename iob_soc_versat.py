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
from iob2axil import iob2axil
from iob_reset_sync import iob_reset_sync

VERSAT_SPEC = "versatSpec.txt"
VERSAT_TOP = "Test"
VERSAT_EXTRA_UNITS = os.path.join(os.path.dirname(__file__), "hardware/src/units")


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

    return f"../{name}_V0.70_{testName}"


class iob_soc_versat(iob_soc):
    name = "iob_soc_versat"
    version = "V0.70"
    flows = "pc-emul emb sim doc fpga"
    setup_dir = os.path.dirname(__file__)
    build_dir = GetBuildDir(name)

    @classmethod
    def _create_instances(cls):
        super()._create_instances()
        # Verilog modules instances if we have them in the setup list (they may not be in the list if a subclass decided to remove them).
        print(cls.submodule_list, file=sys.stderr)

        if iob_versat in cls.submodule_list:
            cls.versat = iob_versat(
                "VERSAT0",
                parameters={
                    "versat_spec": VERSAT_SPEC,
                    "versat_top": GetTestName(),
                    "extra_units": VERSAT_EXTRA_UNITS,
                },
            )
            cls.peripherals.append(cls.versat)
        # if iob_vexriscv in cls.submodule_list:
        #    cls.cpu = iob_vexriscv("cpu_0")

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
                # iob_vexriscv,
                iob_versat,
                # axil2iob,
                # iob2axil,
                iob_reset_sync,
                # (iob_uart, {"purpose": "simulation"}),
            ]
            + extra_submodules
        )

        # Remove iob_picorv32 because we want vexriscv
        """
        i = 0
        while i < len(cls.submodule_list):
            if type(cls.submodule_list[i]) == type and cls.submodule_list[i].name in [
                "iob_picorv32"
            ]:
                cls.submodule_list.pop(i)
                continue
            i += 1
        """

    @classmethod
    def _post_setup(cls):
        super()._post_setup()

        print("=== SOC_VERSAT ===")

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

        shutil.rmtree(f"{cls.build_dir}/software/src/Tests")


#    @classmethod
#    def _setup_confs(cls, extra_confs=[]):
