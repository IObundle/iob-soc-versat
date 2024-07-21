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

Versat is a tool that helps the automatic generation of coarse grain accelerators. Versat acts as a compiler, transforming a high level specification written in a custom language into an accelerator coded in verilog as well as C source files that define the expected interface with the accelerator  can be used to configure the accelerator.

### Dataflow paradigm

The Versat architecture follows the dataflow paradigm. Computation is performed by instantiating and connecting units between themselves. Data flows from unit to unit and the units perform computations that are then carried out to further units along the circuit. 

Due to the nature of the computation, the units need to be capable of performing computations in a fixed amount of cycles. Versat does not support time variable units.

The default workflow when using a Versat accelerator is as follows: The user configures the units by writing configuration data into a configuration register. The user then instructs the accelerator to start a run. This executes the accelerator until it finishes processing the current

### Units

The units utilized by Versat are the building blocks of more advanced circuits. They represent the most basic forms of computation supported by Versat and are described directly in verilog. For simple operations, like integer addition and bit shifting, Versat supports them directly (every operation supported by Verilog is supported by Versat). Furthermore, Versat contains a set of default units that are used to perform more complex calculations, like floating point operations, as well as units designed to control data like memories, lookup tables and VRead/VWrite units, which are designed to transfer memory from and to RAM.

Units are coded in Verilog and other than the specific interfaces that they must implement, there is no specific on the way they are implemented. Units can contain internal state. Units can also implement any form of logic when the accelerator is not running, but 

### Versat specification

Accelerators are designed using a high level custom made language. The syntax is still fluctuating as Versat improves and potentially changes, but it is stable enough to perform usable work with. Inspired by verilog and C, the language is used to describe entities using a hierachical approach: Entities can contain other entities and so on. Entities can only reference previously declared entities (prevents recursive references, which are not permitted in dataflow designs). 

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

As mentioned previously, the configuration of the accelerator is stored inside the accelerator in a large register. To make the configuration part as easy as possible for the programmer, the generated header defines a couple of structs that match the structure of the configuration register. From the POV of the programmner, configuring units is the same as initializing the members of a structure.

The easy of use can be seen in this example, where we store constants inside the units, run the accelerator and immediatly print the result.

<Insert picture with Spec and C code of a simple addition>.

<Talk about merge as well in regards to generated software>

The generated structs follow the hierarchy. Both the simple units as well as the higher level modules will generate a struct that contains the lower levels. For the same unit inputs, the generated 
struct will be the same. This allows the programmer to create higher level functions in order to simplify the act of configuring the accelerator.

The same logic is applied to state. Generated structures map the generated state register and the programmer only needs to read the members of a struct in order to read the state of the units.

The memory mapping contains some slight differences which are explained further down.

No difference is needed in order to run the pc emulation vs the verilog simulator. Both modes of testing the design use the same Versat API present in the header file. The difference in implementation are handled automatically by Versat.

### Custom units.

Versat is also capable of integrating custom units, as long as they follow the interface expected by Versat.

In order to integrate with the accelerator and to provide mechanisms of configurability and data I/O, units are expected to implement certain interfaces (in verilog), following a specific nomenclature and well as specific port direction and bitsize. 

The interfaces, wire formats expected and their usages are as follows:

...

Standard interface: Wires: clk, rst, run, running.

Inputs: The input interface allows the unit to output data into the circuit. The wire must be an input of size DATA_W and must contain a name in the format "inN" where N is a sequential number starting from zero.

Outputs: The input interface allows the unit to receive data from the circuit. The wire must be an output of size DATA_W and must contain a name in the format "outN" where N is a sequential number starting from zero.

Delay: Some units need to know when the input is valid data or not. Since Versat uses a static format of tracking validity of data, Versat knows before accelerator run how many cycles must pass before a given unit receives it receives valid data. For units that only contain output interfaces, delay indicates when the unit must start outputting data.

Done: The Done interface is responsible for controlling the runtime of the accelerator. Only when every unit in the circuit asserts Done does the run terminate. Any unit that implements the Done interface must assert it eventually otherwise the accelerator runs forever. 

Databus interface: The databus interface allows the unit to access external RAM. Versat takes care of handling the complexity of connecting to the external RAM, instead offering a simple interface to the Units. The usage of a databus interface mandates the need to implement the Done interface as well.

Memory mapped: Memory mapped interfaces allow units to offer a mapped interface to outside world. This interface usually implements the need to implement an external memory interface although it is not required.

External memories: For units that need to store large amounts of data, Versat offers a simple interface that automatically instantiates memories.

Config: The config interface is the interface that allows unit to receive simple forms of configuration information. Config is constant throughout a accelerator run. Any wire of any size with a direction of input whose name is not associated to any other interface is considered a config wire.

State: State allows the unit to output simple forms of data to the outside. State is supposed to serve as a way for units to make internal state observable by the outside world. This means that, for the most part, state info is supposed to remain constant while the accelerator is not running, althought this is not required. One example of the use of state info is to allow CPU to read the state of registers or the amount of data stored in a unit, if it decides to expose it.

### Data validity and run time

The accelerator generated by Versat uses a static method of handling data validity. Each unit that requires the ability to differenciate between valid data and garbage can do so by implementing the delay interface. The entire process of calculating the delays and making sure that the accelerator is correctly configured in this aspect is handled automatically by Versat. The end user should not need to know these details. 

When using the databus interface and when the unit has finished processing data, it should assert the Done interface to 

However, for cases where we need to process a variable amount of data, we can also use the done interface. Techically, the done interface is.

The usage of the done interface is fundamental to implement the most important units of Versat: The VRead and VWrite units. These units use a databus interface to eventually connect to the system RAM which allows them to transfer large amount of data between RAM and Versat. Since accessing system RAM takes a variable amount of time, and Versat can only operate on fixed time units, the VRead and VWrite units use a double buffer strategy: while data is being transfer by RAM, the data present in a memory is being outputted or stored into the circuit.

This design means that a simple processing of the form: RAM -> Circuit -> RAM requires 3 accelerator runs: The first transfers data from RAM into a VRead unit. The second run uses the valid data inside the VRead unit, which is processsed and stored inside the VWrite unit and the third run writes that data to RAM. Maximal throughput is achieved when the accelerator is using VUnits to transfer data and is computing using valid data.

### Configuration and shadow register

The architecture used by Versat stores all the configurations of the units in a large register contained inside the accelerator. This register stores the configurations and remains constant during the entire .

By default, Versat generates the accelerator with an additional "shadow register" which allows the accelerator to change configuration while running. The shadow register stores the configuration that are being used by the accelerator during a run and any change to the configuration register is not registed until the next run of the accelerator. To maximize performance it is expected that firmware code performs configuration of the next run while the accelerator is executing.

### Memory mapping

Memory mapping is a special interface that allows units to map a portion of the address space. For the most part, this is used to fill units that contain memories but do not implement databus interfaces (like LookupTables).

In order to support pc emulation, we cannot access memory mapped units in the same form we access configurations. All memory mapped accesses must be done by using specific functions.

Furthermore, large memory transfers should be performed by using the VersatMemoryCopy function, which can use an internal DMA to speedup the transfer. The DMA only supports transfers between memory and the accelerator. It does not support memory to memory or accelerator to accelerator transfers.

<Can we implement a struct directly instead of doing the stupid address thing?>

### Configuration sharing.

In order to facilitate the design of SIMD portions of code, we implement two ways of sharing the config interface between units. Note that units with shared configs are still individual working with their own inputs, outputs and internal states. Sharing configuration does not imply sharing anything else, including state interfaces which are not shared.

Of the two ways, the simplest is to define a share construct directly inside a module. We can only share configurations between units of the same type and as such the share construct is defined by using the share keyword, defining the type and then using a block to define all the shared instances.

Shared units constructed this way have their configurations shared directly inside the generated structures, by using a union instead of a structure in C. In general, when using shared units, the format of the generated structure is a good way of figuring out how the configuration is being shared.

The second way is through the use of the word static before an instance declaration. Mimicking the concept found in object oriented languages, a static unit is a unit that is associated to the module instead of the instance. Every instance of the module will instantiate the static unit that will contain the same configuration as every other instance of the same unit, regardless of the position on the hierarchy. Unlike configuration sharing, static units contain their own structure used to configure them. Static units are useful to implement registers that share constants instead of using extra inputs and outputs to pass the data into lower hiearchy units.

<Insert image or something>

### Merge

Due to the nature of dataflow computations, it is likely that. The ability to "merge" modules is therefore used not only to potentially save logic and reduce resources used, but it is also used to simplify the design of more complex datapaths while offering a simpler interface for the user. Note that Versat not only produces the merged circuit but also generates all the necessary code to offer an almost complete transparent.

The merge operation is a complex operation and currently contains some restrictions: Iterative units 

Merged entities can share units. Shared units also share all their interfaces and content.

Merged circuits are not restricted by the loop. Units that use configuration sharing cannot be merged (yet) but the sharing of configurations still persists in the merged circuit.

Due to the fact that units can contain internal data and that the datapath can loop, it is helpful to have control over which units are merged or not. This can be accomplished by using an extended form of the merge construct. First, names need to be given to each type. 

### Merge software configuration

A merged accelerator contains various datapaths, and at any given point only one of these datapaths is currently active. For these types of accelerators, the software generated contains more functions designed to help the user configure the datapath. User only needs to call a function using an Enum and the 

Since configurations are shared, the structures generated for merged accelerators can differ from the structures generated for the non merged units equivalents. The generated structs can contain holes in specific offsets. Versat tries to make the changes from non merged to merged as <soft> as possible. 

### Versat program

Versat itself is an executable that behaves like a compiler. It expects a specification file, the name of the Top level entity and it outputs all the previously mentioned files. A nix file allows versat to be compiled and run very easily by simply doing a CallPackage in the top nix file (as can be seen in default.nix) and afterwards, entering a nix-shell should compile and prepare the environment to utilize Versat. To run it individually, simply call ./versat.

<More information can be found by calling versat -h. (Need to do this)>

### Misc.


Example of a simple Module.
Example of static.
Example of share.


Example of connecting units.
Example of grouping connections.
Example of operations. (Expressions - Also talk about how the name of the expression can be reused multiple times as if it was equality in normal C code)
Example of iterative.
Example of arrays.
Example of ports.
Example of delays.

