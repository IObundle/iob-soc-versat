# iob-soc-sha
SoC to run the SHA256 program on a RISC-V processor, with or without
acceleration using the VERSAT2.0 Coarse Grained Reconfigurable Array as a
hardware accelerator.

# Setup
Clone the repository and the submodules with:
```
git clone --recursive git@github.com:IObundle/iob-soc-sha.git
```
or using the url:
```
git clone --recursive https://github.com/IObundle/iob-soc-sha.git
```
* * *
# PC Emulation
The iob-soc-sha system can build and run an environment for PC with:
```
make pc-emul-run
```
This target performs the Short Message Test for Byte-Oriented `sha256()` 
implementations from the 
[NIST Cryptograpphic Algorithm Validation
Program](https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program/secure-hashing).

The test vectors are a set of 65 messages from 0 to 64 byte length. The 
implementation program only receives the messages and outputs the corresponding
message digests (MD). An external script compares the implementation output with
the expected MD from the test vectors.

The implementation output can be checked manually from terminal and
`software/pc-emul/ethernet.log`

### Clean environment
To clean the workspace after PC emulation:
```
make pc-emul-clean
```
### Requirements
PC emulation program requires:
- Git
- Make
- gcc
- Python 3.6+

* * *
# RISCV Emulation
The iob-soc-sha system can be emulated using a verilog simulator like icarus 
with:
```Make
# Test with all supported simulators
make test-sim
# Test with a specific simulator
make sim-test SIMULATOR=icarus
make sim-test SIMULATOR=verilator
```

### Clean environment
To clean the workspace after the RISCV emulation:
```
make test-sim-clean
```

### Requirements/Setup
RISCV emulation requires:
- PC Emulation requirements
- RISCV toolchain
    - Add the RISCV toolchain to you `PATH` variable in `$HOME/.bashrc`:
    ```
    export RISCV=/path/to/riscv/bin
    export PATH=$RISCV:$PATH
    ```
- Verilog simulator, for example: 
    - [icarus verilog](https://github.com/steveicarus/iverilog)  
    - [verilator](https://github.com/verilator/verilator)

# FPGA Execution
The system can be tested on FPGA with:
```
make test-fpga
```

The results can be manually checked in the terminal and in
`hardware/fpga/<tool>/<board>/ethernet.log`, where `<tool>` is the tool used
for synthesis and `<board>` is the board directory name.

The system has been tested with the `AES-KU040-DB-G` board from Xilinx. In that
case the results can be found in: `hardware/fpga/vivado/AES-KU040-DB-G`. 

### Clean environment
To clean the workspace after the FPGA execution:
```
make test-fpga-clean
```

### Requirements/Setup
FPGA execution requires:
- Supported FPGA board
- Setup environment for FPGA execution
    - Add the executable paths and license servers in `$HOME/.bashrc`:
    ```
    export VIVADOPATH=/path/to/vivado
    ...
    export LM_LICENSE_FILE=port@licenseserver.myorg.com;lic_or_dat_file
    ```
    - Follow [IOb-soc's README](https://github.com/IObundle/iob-soc#readme) for
    more installation details.

# Profiling
The system can be profiled using a 
[Timer core](https://www.github.com/IObundle/iob-timer.git), a software 
controlled counter.

The `pc-emul` version simulates the counter behaviour by calling the C standard
`<timer.h>` library.

The profiling is available for `pc-emul` using the following command:
```
make pc-emul-profile
```
The `pc-emul-profile` target outputs an `emul_profile.log` file with the
profiling information.

For `fpga` profiling run the following command:
```
make fpga-run-profile
```
The `fpga-run-profile` target outputs a `fpga_profile.log` file with the 
profiling information.

# Ethernet
The system supports ethernet communication using the 
[IOb-Eth core](https://github.com/IObundle/iob-eth).

Check [IO-Eth's README](https://github.com/IObundle/iob-eth#readme) for setup 
instructions and further details.

# Versat
### Versat Custom Functional Units
The acceleration of SHA application requires the design of custom functional
units (FUs). These FUs can be validated with unit tests by running the command:
```
make test-versat-fus
```
The custom FUs are in `hardware/src/units/`.

# Versat Tutorial

Versat is a tool that helps the automatic generation of coarse grain accelerators. Versat acts as a compiler, transforming a high level specification written in a custom language into verilog and source files that are used to configure the accelerator.


### Dataflow paradigm

The Versat architecture follows the dataflow paradigm. Computation is performed by instantiating and connecting units between themselves. Data flows from unit to unit and the units perform computations that are then carried out to further units along the circuit. 

Due to the nature of the computation, the units need to be capable of performing computations in a fixed amount of cycles. Versat does not support time variable units.

The default workflow when using a Versat accelerator is as follows: The user configures the units by writing configuration data into a configuration register. The user then instructs the accelerator to start a run. This executes the accelerator until it finishes processing the current

### Units

The units utilized by Versat are the building blocks of more advanced circuits. They represent the most basic forms of computation supported by Versat and are described directly in verilog. For simple operations, like integer addition and bit shifting, Versat supports them directly (every operation supported by Verilog is supported by Versat). Furthermore, Versat contains a set of default units that are used to perform more complex calculations, like floating point operations, as well as units designed to control data like memories, lookup tables and VRead/VWrite units, which are designed to transfer memory from and to RAM.

In order to integrate with the accelerator and to provide mechanisms of configurability and data I/O, units are expected to implement certain interfaces (in verilog), following a specific nomenclature and well as a specific port direction and bitsize. 

The interfaces, wire formats expected and their usages are as follows:

...

### Data validity and run time

The accelerator generated by Versat uses a static method of handling data validity. Each unit that requires the ability to differenciate between valid data and garbage can do so by implementing the delay interface. The entire process of calculating the delays and making sure that the accelerator is correctly configured in this aspect is handled automatically by Versat. The end user should not need to know these details. 

However, for cases where we need to process a variable amount of data, we can also use the done interface. Techically, the done interface is.

The usage of the done interface is fundamental to implement the most important units of Versat: The VRead and VWrite units. These units use a databus interface to eventually connect to the system RAM which allows them to transfer large amount of data between RAM and Versat. Since accessing system RAM takes a variable amount of time, and Versat can only operate on fixed time units, the VRead and VWrite units use a double buffer strategy: while data is being transfer by RAM, the data present in a memory is being outputted or stored into the circuit.

This design means that a simple processing of the form: RAM -> Circuit -> RAM requires 3 accelerator runs: The first transfers data from RAM into a VRead unit. The second run uses the valid data inside the VRead unit, which is processsed and stored inside the VWrite unit and the third run writes that data to RAM. Maximal throughput is achieved when the accelerator is using VUnits to transfer data and is computing using valid data.

### Configuration and shadow register

The architecture used by Versat stores all the configurations of the units in a large register contained inside the accelerator. This register stores the configurations (the units must not store configuration data)

By default, Versat generates the accelerator with an additional "shadow register" which allows the accelerator to change configuration while running. The shadow register stores the configuration that is being used by the accelerator during a run and any change to the configuration register is not registed until the next run of the accelerator. To maximize performance it is expected that the firmware is constructed such that the configuration is performed simultaniously with the accelerator running.

### Versat specification

Accelerators are designed by usage of a high level custom language. This custom language syntax is still fluctuating as Versat improves and potentially changes, but it is stable enough to perform usable work with. Inspired by verilog and C, the language is used to describe entities using a hierachical approach: Entities can contain other entities and so on. Entities can only reference previously declared entities (prevents recursive references, which are not permitted in dataflow designs). 

Entities can be of different types, with the type being the first identifier that starts a declaration.

The simplest entity is a "module". This entity corresponds to a simple grouping of instantiated units and their connections. The module defines a list of inputs, using names and a syntax similar to C, where we can define simple inputs or arrays of inputs.

After defining the inputs the body of the definition is separated into two portions: The upper side portion is used to instantiate units. The lower side is used to connect units between themselves.

<Insert an image of spec with good text>

ExampleVersatSpec.txt contains good examples with comments to explain how everything works.

The datapath defined by a module can contain loops in certain conditions. Units that act like memories and registers can serve both as inputs and outputs of the datapath. However Versat does not support "internal" loops. 

More advanced entities are described below.

### Generated software

In order to simplify the user ability to interact with the generated accelerator, a header file is generated that contains a set of easy to use structures and functions to help configure and use the accelerator. 

Like other peripherals, Versat needs to be initialized and the versat_init function must be called with the base address of the accelerator. This function must be the first function called before anything else.

As mentioned previously, the configuration of the accelerator is stored inside the accelerator in a large register. The content of this register is mimicked by a C structure.

The easy of use can be seen in this example, where we store constants inside the units, run the accelerator and immediatly print the result.

<Insert picture with Spec and C code of a simple addition>.

<Talk about merge as well in regards to generated software>

The generated structs are always the same given the same inputs. Furthermore, the generated structures try to model the same hierarchy than the hierarchy used in versat specs. This allows users to define high level functions that can use pointers to pass higher hierarchy units to perform configuration.

No difference is needed in order to run the pc emulation vs the verilog simulator. Both modes of testing the design use the same Versat API present in the header file. The difference in implementation are handled automatically by Versat.

### Memory mapping

Memory mapping is a special interface that allows CPU to access units directly using a memory mapped interface. For the most part, this is used to fill units that contain memories but do not implement databus interfaces (like LookupTables). 

Due to the way pc emulation is implemented, we cannot access memory mapped units in the same form we access configurations. All memory mapped accesses must be done by using specific functions. Furthermore, large memory transfers should be performed by using the VersatMemoryCopy functions, which can use a DMA to speedup the transfer.

Because the memory mapping relies on the 

<Can we implement a struct directly instead of doing the stupid address thing?>

### Configuration sharing.

In order to facilitate the design of SIMD portions of code, we implement two ways of sharing configurations between units. Note that units with shared configuration are still individual working with their own inputs, outputs and internal states. Sharing configuration does not imply sharing anything else, including state interfaces which are not shared.

Of the two ways, the simplest is to define a share construct directly inside a module. We can only share configurations between units of the same type and as such the share construct is defined by using the share keyword, defining the type and then using a block to define all the shared instances.

Shared units constructed this way have their configurations shared directly inside the generated structures, by using a union instead of a structure in C. In general, when using shared units, the format of the generated structure is a good way of figuring out how the configuration is being shared.

The second way is through the use of the word static before an instance declaration. Mimicking the concept found in object oriented languages, a static unit is a unit that is associated to the module instead of the instance. Every instance of the module will instantiate the static unit that will contain the same configuration as every other instance of the same unit, regardless of the position on the hierarchy. Unlike configuration sharing, static units contain their own structure used to configure them. Static units are useful to implement registers that share constants instead of using extra inputs and outputs to pass the data into lower hiearchy units.

<Insert image or something>

### Merge

Due to the nature of dataflow computations, it is likely that. The ability to "merge" modules is therefore used not only to potentially save logic and reduce resources used, but it is also used to simplify the design of more complex datapaths while offering a simpler interface for the user. Note that Versat not only produces the merged circuit but also generates all the necessary code to offer an almost complete transparent.

Merged entities can share units. Shared units also share all their interfaces and content.

Merged circuits are not restricted by the loop. Units that use configuration sharing cannot be merged (yet) but the sharing of configurations still persists in the merged circuit.

Due to the fact that units can contain internal data and that the datapath can loop, it is helpful to have control over which units are merged and 

### Merge software configuration

A merged accelerator contains various datapaths, and at any given point only one of these datapaths is currently active. For these types of accelerators, the software generated contains more functions designed to help the user configure the datapath. User only needs to call a function using an Enum and the 

Since configurations are shared, the structures generated for merged accelerators can differ then structures generated for the non merged units equivalents. The generated structs can contain holes in specific offsets. Versat tries to make the conversion of code that 

### Versat program

Versat itself is an executable that behaves like a compiler. It expects a specification file, the name of the Top level entity and it outputs all the previously mentioned files. A nix file allows versat to be compiled and run very easily by simply doing a CallPackage in the top nix file (as can be seen in default.nix) and afterwards, entering a nix-shell should compile and prepare the environment to utilize Versat. To run it individually, simply call ./versat.

<More information can be found by calling versat -h. (Need to do this)>

### Misc.


