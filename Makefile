CORE := iob_soc_versat

SIMULATOR ?= verilator
BOARD ?= AES-KU040-DB-G

DISABLE_LINT:=1

LIB_DIR:=submodules/IOBSOC/submodules/LIB
include $(LIB_DIR)/setup.mk

INIT_MEM ?= 0
USE_EXTMEM := 1

ifeq ($(INIT_MEM),1)
SETUP_ARGS += INIT_MEM
endif

setup:
	nix-shell --run 'make build-setup SETUP_ARGS="$(SETUP_ARGS)"'

pc-emul-run:
	nix-shell --run 'make clean setup && make -C ../$(CORE)_V*/ pc-emul-run'

.PHONY: setup
