# What is IOb-SoC-Versat

IOb-SoC-Versat is a System-on-Chip (SoC) system designed to test [Versat](https://github.com/IObundle/iob-versat) generated accelerators.

## Clone the repository

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

## Running tests

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


## Nix environment

You can use
[nix-shell](https://nixos.org/download.html#nix-install-linux) to run
IOb-SoC-Versat in a [Nix](https://nixos.org/) environment with all dependencies available except for Vivado and Quartus for FPGA compilation and running.

After installing `nix-shell,` it can be initialized by calling any Makefile target in the IOb-SoC-Versat root directory, for example
```bash
make setup
```

The first time it runs, `nix-shell` will automatically install all the required dependencies. This process can take a couple of hours, but after that, you can enjoy IOb-SoC-Versat and not worry about installing software tools.

## Dependencies

You may install all the dependencies manually and run IOb-SoC-Versat without nix-shell. The following tools should be installed:
- GNU Bash >=5.1.16
- GNU Make >=4.3
- RISC-V GNU Compiler Toolchain =2022.06.10  (Instructions at the end of this README)
- Python3 >=3.10.6
- Python3-Parse >=1.19.0

Optional tools, depending on the desired run strategy:
- Icarus Verilog >=10.3
- Verilator >=5.002
- gtkwave >=3.3.113
- Vivado >=2020.2
- Quartus >=20.1

Older versions of the dependencies above may work, but they must still be tested. It is recommended to use a Nix environment.

## Set environment variables for local or remote building and running

The various simulators, FPGA compilers, and FPGA boards may run locally or
remotely. For running a tool remotely, you need to set two environmental
variables: the server logical name and the server user name. Consider placing
these settings in your `.bashrc` file so that they apply to every session.


### Set up the remote simulator server

Using the open-source simulator Icarus Verilog (`iverilog`) as an example, note that in
`submodules/hardware/simulation/icarus.mk,` the variable for the server logical name,
`SIM_SERVER,` is set to `IVSIM_SERVER,` and the variable for the user name,
`SIM_USER` is set to `IVSIM_USER`.

To run the simulator on the server *mysimserver.myorg.com* as user *ivsimuser*, set the following environmental
variables beforehand, or place them in your `.bashrc` file:

```bash
export IVSIM_SERVER=ivsimserver.myorg.com
export IVSIM_USER=ivsimuser
```

When you start the simulation, IOb-SoC-Versat simulation Makefile will log you on to the server using `ssh,` then `rsync` the files to a remote build directory and run the simulation there. If you do not set these variables, the simulator will run on your local machine if installed.

### Set up the remote FPGA toolchain and board servers

Using the CYCLONEV-GT-DK board as an example, note that in
`hardware/fpga/quartus/CYCLONEV-GT-DK/Makefile,` the variable for the FPGA tool
server logical name, `FPGA_SERVER,` is set to `QUARTUS_SERVER,` and the
variable for the user name, `FPGA_USER`, is set to `QUARTUS_USER`; the
variable for the board server, `BOARD_SERVER,` is set to `CYC5_SERVER`, and
the variable for the board user, `BOARD_USER,` is set to `CYC5_USER`. As in the
previous example, set these variables as follows:

```bash
export QUARTUS_SERVER=quartusserver.myorg.com
export QUARTUS_USER=quartususer
export CYC5_SERVER=cyc5server.myorg.com
export CYC5_USER=cyc5username
```

In each remote server, the environment variable for the license server used must be defined as in the following example:

```bash
export LM_LICENSE_FILE=port@licenseserver.myorg.com;lic_or_dat_file
```
