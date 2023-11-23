CORE := iob_soc_versat

SIMULATOR ?= verilator
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

TESTS:= M_Stage SimpleCGRA F_Stage AddRoundKey LookupTable MemToMem VReadToVWrite # SMVMBlock 

TESTS_SETUP:=$(addsuffix _setup,$(TESTS))
TESTS_PC:=$(addsuffix _pc,$(TESTS))
TESTS_SIM:=$(addsuffix _sim,$(TESTS))
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

# Need to create dirs first because we need to store makefile output into a setup.txt file inside the folder
# Could be a folder name but let's use phony names for now. Its not like the remaining part of the makefile keeps track of modified files
$(TESTS_DIR): 
	mkdir -p ../$(CORE)_V0.70_$(subst _dir,,$@)

$(TESTS_SETUP): $(TESTS_DIR)
	nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) TEST=$(subst _setup,,$@)" &> ../$(CORE)_V0.70_$(subst _setup,,$@)/setup.txt'

test-setup: $(TESTS_SETUP)

$(TESTS_PC):  $(TESTS_SETUP)
	nix-shell --run 'make -C ../$(CORE)_V0.70_$(subst _pc,,$@)/ pc-emul-run &> ../$(CORE)_V0.70_$(subst _pc,,$@)/pc-emul-run.txt'

test-pc-emul-run: $(TESTS_PC)

$(TESTS_SIM): $(TESTS_SETUP)
	nix-shell --run 'make -C ../$(CORE)_V0.70_$(subst _sim,,$@)/ sim-run &> ../$(CORE)_V0.70_$(subst _sim,,$@)/sim-run.txt'

# TODO: Only test this after commiting to git
#test-clean:
#	rm -r "./$(CORE)_*"

setup:
	nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS) TEST=$(TEST)"'

pc-emul-run:
	nix-shell --run 'make clean setup && make -C ../$(CORE)_V0.70_$(TEST)/ pc-emul-run'

sim-run:
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM) && make -C ../$(CORE)_V0.70_$(TEST)/ sim-run SIMULATOR=$(SIMULATOR) VCD=$(VCD)'

.PHONY: setup test-setup
