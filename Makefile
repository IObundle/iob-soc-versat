CORE := iob_soc_versat

SIMULATOR ?= verilator
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

TESTS:= M_Stage F_Stage SimpleCGRA AddRoundKey LookupTable MemToMem VReadToVWrite SimpleIterative Variety1

TESTS_SETUP:=$(addsuffix _setup,$(TESTS))
TESTS_SETUP_PC:=$(addsuffix _setup_pc,$(TESTS))
TESTS_PC:=$(addsuffix _pc,$(TESTS))
TESTS_FAST_PC:=$(addsuffix _fast_pc,$(TESTS))
TESTS_SIM_BUILD:=$(addsuffix _sim_build,$(TESTS))
TESTS_SIM_RUN:=$(addsuffix _sim_run,$(TESTS))
TESTS_DIR:=$(addsuffix _dir,$(TESTS))

VCD ?= 1
INIT_MEM ?= 1
USE_EXTMEM ?= 0

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

ifeq ($(USE_EXTMEM),1)
SETUP_ARGS += USE_EXTMEM
endif

TEST:= M_Stage

# TODO: For now simply set the pc-emul-run to a simple example
#N_TESTS := $(words $(TESTS))
#RANDOM_N := $(shell python3 -c "import random; print(random.randint(1,$(N_TESTS)))")
#RANDOM_TEST := $(word $(RANDOM),$(TESTS))

# Fast rules only work after at least calling setup once. They basically just skip everything that python setup does and just do what versat would do
VERSAT_CALL := ./versat
ifeq ($(DEBUG),1)
VERSAT_CALL := gdb -ex run --args ./versat
endif

versat-only:
	mkdir -p ../$(CORE)_V0.70_$(TEST)
	cd ./submodules/VERSAT ; $(MAKE) -s -j 8 versat
	cd ./submodules/VERSAT ; $(VERSAT_CALL) /home/z/AA/Versat/iob-soc-versat/versatSpec.txt -s -b=32 -T $(TEST) -O /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/hardware/src/units -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/hardware/include -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/hardware/src -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/submodules/FPU/hardware/src -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/submodules/FPU/hardware/include -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/submodules/FPU/submodules/DIV/hardware/src -I /home/z/AA/Versat/iob_soc_versat_V0.70_$(TEST)/hardware/src -H /home/z/AA/Versat/iob_soc_versat_V0.70_$(TEST)/software -o /home/z/AA/Versat/iob_soc_versat_V0.70_$(TEST)/hardware/src -O /home/z/AA/Versat/iob-soc-versat/hardware/src/units -x64

fast:
	cp ./software/src/Tests/$(TEST).cpp ../$(CORE)_V0.70_$(TEST)/software/src/test.cpp
	cp ./software/src/Tests/testbench.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	cp ./software/src/Tests/unitConfiguration.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	mkdir -p ../$(CORE)_V0.70_$(TEST) ; \
	cd ./submodules/VERSAT ; \
	$(MAKE) -s -j 8 versat ; \
	$(VERSAT_CALL) /home/z/AA/Versat/iob-soc-versat/versatSpec.txt -s -b=32 -T $(TEST) -O /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/hardware/src/units -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/hardware/include -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/hardware/src -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/submodules/FPU/hardware/src -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/submodules/FPU/hardware/include -I /home/z/AA/Versat/iob-soc-versat/submodules/VERSAT/submodules/FPU/submodules/DIV/hardware/src -I /home/z/AA/Versat/iob_soc_versat_V0.70_$(TEST)/hardware/src -H /home/z/AA/Versat/iob_soc_versat_V0.70_$(TEST)/software -o /home/z/AA/Versat/iob_soc_versat_V0.70_$(TEST)/hardware/src -O /home/z/AA/Versat/iob-soc-versat/hardware/src/units -x64

update-nix-versat:
	-nix-shell --run "versat &> /dev/null" 

fast-pc-emul: update-nix-versat
	nix-shell --run "make fast TEST=$(TEST); make -C ../$(CORE)_V0.70_$(TEST) pc-emul-test"

$(TESTS_FAST_PC): update-nix-versat 
	nix-shell --run "make fast TEST=$(subst _fast_pc,,$@) &> ../$(CORE)_V0.70_$(subst _fast_pc,,$@)/fast_setup.txt; make -C ../$(CORE)_V0.70_$(subst _fast_pc,,$@)/ pc-emul-test 1> ../$(CORE)_V0.70_$(subst _fast_pc,,$@)/pc-emul-test.txt"

fast-test-pc-emul: $(TESTS_FAST_PC)

# Need to create dirs first because we need to store makefile output into a setup.txt file inside the folder
# Could be a folder name but let's use phony names for now. Its not like the remaining part of the makefile keeps track of modified files
$(TESTS_DIR): update-nix-versat 
	mkdir -p ../$(CORE)_V0.70_$(subst _dir,,$@)

$(TESTS_SETUP): $(TESTS_DIR)
	+nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) TEST=$(subst _setup,,$@)" &> ../$(CORE)_V0.70_$(subst _setup,,$@)/setup.txt'

$(TESTS_SETUP_PC): $(TESTS_DIR)
	+nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) PC_EMUL TEST=$(subst _setup_pc,,$@)" &> ../$(CORE)_V0.70_$(subst _setup_pc,,$@)/setup.txt'

$(TESTS_PC): $(TESTS_SETUP_PC)
	+nix-shell --run 'make -C ../$(CORE)_V0.70_$(subst _pc,,$@)/ pc-emul-test 1> ../$(CORE)_V0.70_$(subst _pc,,$@)/pc-emul-test.txt'

$(TESTS_SIM_BUILD): $(TESTS_SETUP)
	+nix-shell --run 'make -C ../$(CORE)_V0.70_$(subst _sim_build,,$@)/ sim-build SIMULATOR=icarus VCD=0 1> ../$(CORE)_V0.70_$(subst _sim_build,,$@)/sim-test.txt'

$(TESTS_SIM_RUN): $(TESTS_SETUP)
	+nix-shell --run 'make -C ../$(CORE)_V0.70_$(subst _sim_run,,$@)/ sim-run SIMULATOR=$(SIMULATOR) VCD=$(VCD) 1> ../$(CORE)_V0.70_$(subst _sim_run,,$@)/sim-test.txt'

test-setup: $(TESTS_SETUP)

# Use -s and -jN flags when running tests. Usually set N to half the CPU count.
test-pc-emul: $(TESTS_PC)
test-sim-build: update-nix-versat $(TESTS_SIM_BUILD)
test-sim-run: update-nix-versat $(TESTS_SIM_RUN)

# TODO: Only test this after commiting to git
#test-clean:
#	rm -r "./$(CORE)_*"

setup:
	+nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) TEST=$(TEST)"'

setup_pc:
	+nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) PC_EMUL TEST=$(TEST)"'

pc-emul-run:
	+nix-shell --run 'make setup_pc && make -C ../$(CORE)_V0.70_$(TEST)/ pc-emul-run'

fpga-run:
	+nix-shell --run 'make setup BOARD=$(BOARD) && make -C ../$(CORE)_V0.70_$(TEST)/ fpga-run BOARD=$(BOARD)'

fpga-run-only:
	+nix-shell --run "make -C ../$(CORE)_V0.70_$(TEST)/ fpga-run BOARD=$(BOARD)"

sim-build:
	+nix-shell --run 'make setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM) && make -C ../$(CORE)_V0.70_$(TEST)/ sim-build SIMULATOR=$(SIMULATOR)'

sim-run:
	+nix-shell --run 'make setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM) && make -C ../$(CORE)_V0.70_$(TEST)/ sim-run SIMULATOR=$(SIMULATOR) VCD=$(VCD)'

.PHONY: setup test-setup
