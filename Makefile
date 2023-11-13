CORE := iob_soc_versat

SIMULATOR ?= verilator
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

VCD ?= 1
INIT_MEM ?= 1
USE_EXTMEM ?= 0

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

ifeq ($(USE_EXTMEM),1)
SETUP_ARGS += USE_EXTMEM
endif

setup:
	nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS)"'

pc-emul-run:
	nix-shell --run 'make clean setup && make -C ../$(CORE)_V*/ pc-emul-run'

sim-run:
#	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM) && make -C ../$(CORE)_V*/ fw-build'
	nix-shell --run 'make clean setup INIT_MEM=$(INIT_MEM) USE_EXTMEM=$(USE_EXTMEM) && make -C ../$(CORE)_V*/ sim-run SIMULATOR=$(SIMULATOR) VCD=$(VCD)'

.PHONY: setup
