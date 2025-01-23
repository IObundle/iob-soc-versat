CUR_DIR=$(shell pwd)

CORE := iob_soc_versat

SIMULATOR ?= verilator
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

FAST_COMPILE_VERSAT := $(MAKE) -C ./submodules/VERSAT -j 8 versat

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

TESTS+= MemToMem
TESTS+= VReadToVWrite
TESTS+= VReadMultipleToVWriteMultiple
TESTS+= TestShare
TESTS+= TestStatic
TESTS+= TestDelays
TESTS+= TestDuty
TESTS+= F_Stage
TESTS+= M_Stage
TESTS+= AddRoundKey

#TESTS+= DMATest # TODO: This was blocking. Take another look
#TESTS+= TestShareStatic # TODO: The test was not testing anything, just printing stuff. Make a pass and see if it works as expected

# Failing for now, need to check them out later:
#TESTS+= Variety1
TESTS+= TestMerge
TESTS+= TestMerge2
TESTS+= TestMergeInputs
TESTS+= TestMergeHasChild

VERSAT_SPEC:=versatSpec.txt

VCD ?= 0
INIT_MEM ?= 0
USE_EXTMEM ?= 1

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

ifeq ($(USE_EXTMEM),1)
SETUP_ARGS += USE_EXTMEM
endif

TEST?=SimpleExample
SETUP_ARGS+=TEST="$(TEST)"

# Fast rules only work after at least calling setup once. They basically just skip everything that python setup does and just do what versat would do, while copying some files quickly.
# Basically a fast way of running by assuming that there is no "large" change, just versat or software changes.

VERSAT_CALL := ./versat
ifeq ($(DEBUG),1)
VERSAT_CALL := gdb -ex run --args ./versat
endif

VERSAT_ARGUMENTS:=$(CUR_DIR)/$(VERSAT_SPEC) -s -b32 -d -t $(TEST) -u $(CUR_DIR)/submodules/VERSAT/hardware/src/units 
VERSAT_ARGUMENTS+=-I $(CUR_DIR)/submodules/VERSAT/hardware/src -O $(CUR_DIR)/../iob_soc_versat_V0.70_$(TEST)/software
VERSAT_ARGUMENTS+=-o $(CUR_DIR)/../iob_soc_versat_V0.70_$(TEST)/hardware/src -g $(CUR_DIR)/../debug -u $(CUR_DIR)/hardware/src/units -x64

# Single test rules
setup:
	+nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) TEST=$(TEST)"'

setup_pc:
	+nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) PC_EMUL TEST=$(TEST)"'

pc-emul-run:
	+nix-shell --run 'make setup_pc && make -C ../$(CORE)_V0.70_$(TEST)/ pc-emul-run'

no-nix-pc-emul-run:
	make setup_pc && make -C ../$(CORE)_V0.70_$(TEST)/ pc-emul-run

fpga-run:
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM) && make -C ../$(CORE)_V0.70_$(TEST)/ fpga-fw-build BOARD=$(BOARD)'
	make -C ../$(CORE)_V0.70_$(TEST)/ fpga-run BOARD=$(BOARD)

sim-build:
	+nix-shell --run 'make setup INIT_MEM=1 USE_EXTMEM=$(USE_EXTMEM) TEST=$(TEST) && make -C ../$(CORE)_V0.70_$(TEST)/ sim-build SIMULATOR=$(SIMULATOR) VCD=$(VCD)'

sim-run:
	+nix-shell --run 'make setup INIT_MEM=1 USE_EXTMEM=$(USE_EXTMEM) TEST=$(TEST) && make -C ../$(CORE)_V0.70_$(TEST)/ sim-run SIMULATOR=$(SIMULATOR) VCD=$(VCD)'

no-nix-sim-run:
	make setup INIT_MEM=1 USE_EXTMEM=$(USE_EXTMEM) TEST=$(TEST) && make -C ../$(CORE)_V0.70_$(TEST)/ sim-run SIMULATOR=$(SIMULATOR) VCD=$(VCD)

fpga-run-only:
	cp ./software/src/Tests/$(TEST).cpp ../$(CORE)_V0.70_$(TEST)/software/src/test.cpp
	cp ./software/src/Tests/testbench.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	cp ./software/src/Tests/unitConfiguration.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	+nix-shell --run "make -C ../$(CORE)_V0.70_$(TEST)/ fpga-fw-build fpga-run BOARD=$(BOARD)"

versat-only:
	mkdir -p ../$(CORE)_V0.70_$(TEST)
	cd ./submodules/VERSAT ; $(MAKE) -j 8 versat
	cd ./submodules/VERSAT ; $(VERSAT_CALL) $(VERSAT_ARGUMENTS)

versat-only-no-compile:
	mkdir -p ../$(CORE)_V0.70_$(TEST)
	cd ./submodules/VERSAT ; $(VERSAT_CALL) $(VERSAT_ARGUMENTS)

# Multi test rules

test-setup:
	+nix-shell --run 'parallel ./scripts/setup.sh ::: $(TESTS)'

test-versat-only:
	cd ./submodules/VERSAT ; $(MAKE) -j 8 versat
	parallel ./scripts/versat-only.sh ::: $(TESTS)

test-simulate:
	python3 ./scripts/test.py simulate testInfo.json

test-disable-failed:
	$(FAST_COMPILE_VERSAT) && python3 ./scripts/test.py disable-failed testInfo.json

test-reenable:
	$(FAST_COMPILE_VERSAT) && python3 ./scripts/test.py reenable testInfo.json

test-update:
	$(FAST_COMPILE_VERSAT) && python3 ./scripts/test.py run testInfo.json

test-no-update:
	$(FAST_COMPILE_VERSAT) && python3 ./scripts/test.py run-only testInfo.json

test-reset:
	python3 ./scripts/test.py reset testInfo.json

test-pc-emul-run:
	+nix-shell --run 'parallel ./scripts/pc-emul.sh ::: $(TESTS)'

test-sim-build:
	+nix-shell --run 'parallel ./scripts/sim-build.sh ::: $(TESTS)'

test-sim-run:
	+nix-shell --run 'parallel ./scripts/sim-run.sh ::: $(TESTS)'

test-clean:
	parallel ./scripts/clean.sh ::: $(TESTS)

fast:
	mkdir -p ../$(CORE)_V0.70_$(TEST)/
	mkdir -p ../$(CORE)_V0.70_$(TEST)/software/
	mkdir -p ../$(CORE)_V0.70_$(TEST)/software/src/
	cp ./software/src/Tests/$(TEST).cpp ../$(CORE)_V0.70_$(TEST)/software/src/test.cpp
	cp ./software/src/Tests/testbench.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	cp ./software/src/Tests/unitConfiguration.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	mkdir -p ../$(CORE)_V0.70_$(TEST) ; \
	cd ./submodules/VERSAT ; \
	$(MAKE) -s -j 8 versat ; \
	$(VERSAT_CALL) $(VERSAT_ARGUMENTS)

fast-pc-emul:
	nix-shell --run "make fast TEST=$(TEST); make -C ../$(CORE)_V0.70_$(TEST) pc-emul-test"

fast-software-emul:
	cp -R ./software/src/ ../$(CORE)_V0.70_$(TEST)/software/
	cp ./software/src/Tests/$(TEST).cpp ../$(CORE)_V0.70_$(TEST)/software/src/test.cpp
	cp ./software/src/Tests/testbench.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	cp ./software/src/Tests/unitConfiguration.hpp ../$(CORE)_V0.70_$(TEST)/software/src/
	nix-shell --run "make -C ../$(CORE)_V0.70_$(TEST) pc-emul-test"

fast-test-pc-emul:
	+nix-shell --run 'parallel ./scripts/fast-pc-emul.sh ::: $(TESTS)'

clean-all:
	rm -r ../$(CORE)_V0.70_*/

.PHONY: setup test-setup

test-make:
	echo "test"
