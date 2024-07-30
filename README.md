# What is IOb-SoC-Versat

IOb-SoC-Versat is a System-on-Chip (SoC) system designed to test [Versat](https://github.com/IObundle/iob-versat) generated accelerators.

## Table of Contents
- [Cloning the repository](#cloning-the-repository)
- [Running tests](#running-tests)
- [Requirements](#requirements)
- [License](#license)
- [Acknowledgements](#acknowledgements)


# Cloning the repository

The first step is to clone this repository. IOb-SoC-Versat uses git sub-module trees and GitHub will ask for your password for each downloaded module if you clone it by *https*. To avoid this,
setup GitHub access with *ssh* and type:

```bash
git clone --recursive git@github.com:IObundle/iob-soc-versat.git
cd iob-soc-versat
```

Alternatively, you can still clone this repository using *https* if you cache
your credentials before cloning the repository using: 
```bash
git config --global credential.helper 'cache --timeout <time_in_seconds>'
```

# Running tests

After cloning the repository and assuming you have all the dependencies installed,  a  test case can be tested by running the following commands:

```bash
make clean pc-emul-run TEST="TestName"
make clean sim-run TEST="TestName"
make clean fpga-run TEST="TestName"
```

TestName is the test's name. The firmware of the tests is contained inside the software/src/Tests folder, and the dataflow graphs they use are described in the file ./versatSpec.txt.

To run all tests listed in the TESTS Makefile variable, run the following command:

```bash
make test-clean test-pc-emul-run
make test-clean test-sim-run
```

The PC emulation tests are compiled and run parallel to exploit a multi-core system. The FPGA tests are compiled and run individually because our system does not support parallel FPGA flows.

# Requirements

IOb-SoC-Versat is a system on chip based on IOb-SoC, which uses IOb-Versat as a peripheral for testing the latter. Refer to the [IOb-SoC](https://github.com/IObundle/iob-soc) and [IOb-Versat](https://github.com/IObundle/iob-versat) to install all the required
dependencies.

# License

IOb-SoC-Versat is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


# Acknowledgement
This project is funded through the NGI Assure Fund, a fund established by NLnet
with financial support from the European Commission's Next Generation Internet
programme, under the aegis of DG Communications Networks, Content and Technology
under grant agreement No 957073.

<table>
    <tr>
        <td align="center" width="50%"><img src="https://nlnet.nl/logo/banner.svg" alt="NLnet foundation logo" style="width:90%"></td>
        <td align="center"><img src="https://nlnet.nl/image/logos/NGIAssure_tag.svg" alt="NGI Assure logo" style="width:90%"></td>
    </tr>
</table>
