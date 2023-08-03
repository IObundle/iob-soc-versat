SHELL = /bin/bash
MAKEFLAGS += --no-builtin-rules
ROOT_DIR:=.
include $(ROOT_DIR)/config.mk

ila-build: ilaFormat.txt
	$(ILA_PYTHON_DIR)/ilaGenerateSource.py ilaFormat.txt ila.c
	$(ILA_PYTHON_DIR)/ilaGenerateVerilog.py ilaFormat.txt $(HW_DIR)/include/
	cp ila.c $(FIRM_DIR)/
	cp ila.c $(PC_DIR)/

ila-generate-vcd: ilaFormat.txt ilaData.txt
	$(ILA_PYTHON_DIR)/ilaDataToVCD.py ilaFormat.txt ilaData.txt ilaOut.vcd

ila-clean:
	@rm -f $(HW_DIR)/include/signal_inst.vh $(FIRM_DIR)/ila.c $(PC_DIR)/ila.c ila.c
	$(MAKE) -C $(ILA_DIR) clean-all

#
# BUILD EMBEDDED SOFTWARE
#

fw-build: ila-build
	$(MAKE) -C $(FIRM_DIR) build-all

fw-clean:
	$(MAKE) -C $(FIRM_DIR) clean-all

fw-debug:
	$(MAKE) -C $(FIRM_DIR) debug

gen-versat:
	$(MAKE) -C $(PC_DIR) gen-versat

#
# EMULATE ON PC
#

pc-emul-build: ila-build
	$(MAKE) -C $(PC_DIR) build

pc-emul-force-build:
	$(MAKE) -C $(PC_DIR) force-build

pc-emul-run: pc-emul-build 
	$(MAKE) -C $(PC_DIR) run

pc-emul-clean: fw-clean
	$(MAKE) -C $(PC_DIR) clean

pc-emul-test: pc-emul-clean
	$(MAKE) -C $(PC_DIR) test

run-versat:
	$(MAKE) -C $(PC_DIR) run-versat

#
# SIMULATE RTL
#

sim-build: ila-build
	$(MAKE) fw-build
	$(MAKE) -C $(SIM_DIR) build

./hardware/src/versat_instance.v:
	$(MAKE) -C $(PC_DIR) run
	$(MAKE) fw-build SIM=1
	$(MAKE) -C $(SIM_DIR) build

sim-run: sim-build
	$(MAKE) -C $(SIM_DIR) run

sim-clean: fw-clean
	$(MAKE) -C $(SIM_DIR) clean

sim-one-build: pc-emul-force-build sim-build

sim-test:
	$(MAKE) -C $(SIM_DIR) test

sim-versat-fus:
	$(MAKE) -C $(SIM_DIR) xunitM SIMULATOR=icarus
	$(MAKE) -C $(SIM_DIR) xunitF SIMULATOR=icarus

sim-debug:
	$(MAKE) -C $(SIM_DIR) debug

#
# HARDWARE BUILD AND RUN
#

fpga-fw-build: ila-build
	$(MAKE) fw-build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)

fpga-build: ila-build
	$(MAKE) -C $(BOOT_DIR) build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)
	#$(MAKE) fw-build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)
	$(MAKE) -C $(BOARD_DIR) build

fpga-run:
	$(MAKE) -C $(BOARD_DIR) run TEST_LOG="$(TEST_LOG)"

fpga-clean: fw-clean
	$(MAKE) -C $(BOARD_DIR) clean

fpga-veryclean:
	$(MAKE) -C $(BOARD_DIR) veryclean

fpga-debug:
	$(MAKE) -C $(BOARD_DIR) debug

fpga-test:
	$(MAKE) -C $(BOARD_DIR) test

#
# COMPILE DOCUMENTS
#
doc-build:
	$(MAKE) -C $(DOC_DIR) $(DOC).pdf

doc-clean:
	$(MAKE) -C $(DOC_DIR) clean

doc-test:
	$(MAKE) -C $(DOC_DIR) test

#
# CLEAN
#

clean: pc-emul-clean sim-clean fpga-clean doc-clean ila-clean

#
# TEST ALL PLATFORMS
#

test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim:
	$(MAKE) sim-test SIMULATOR=verilator
	$(MAKE) sim-test SIMULATOR=icarus

test-sim-clean:
	$(MAKE) sim-clean SIMULATOR=verilator
	$(MAKE) sim-clean SIMULATOR=icarus

test-fpga:
	$(MAKE) fpga-test BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-test BOARD=AES-KU040-DB-G

test-fpga-clean:
	$(MAKE) fpga-clean BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-clean BOARD=AES-KU040-DB-G

test-doc:
	$(MAKE) fpga-clean BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-clean BOARD=AES-KU040-DB-G
	$(MAKE) fpga-build BOARD=CYCLONEV-GT-DK
	$(MAKE) fpga-build BOARD=AES-KU040-DB-G
	$(MAKE) doc-test DOC=pb
	$(MAKE) doc-test DOC=presentation

test-doc-clean:
	$(MAKE) doc-clean DOC=pb
	$(MAKE) doc-clean DOC=presentation

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean test-doc-clean

$(TESTS):
	$(MAKE) -C $(PC_DIR) build TEST=$@

tests: $(TESTS)
	$(foreach i, $(TESTS),$(MAKE) -s -C $(PC_DIR) run TEST=$i;)

single-test:
	@echo "Trying " $(TEST)
	$(MAKE) -C $(PC_DIR) build TEST=$(TEST)
	$(MAKE) -C $(PC_DIR) run

versat-clean:
	$(MAKE) -C ./submodules/VERSAT clean

debug:
	@echo $(UART_DIR)
	@echo $(CACHE_DIR)

.PHONY: fw-build fw-clean fw-debug \
	pc-emul-build pc-emul-run pc-emul-clean pc-emul-test \
	sim-build sim-run sim-clean sim-test \
	fpga-build fpga-run fpga-clean fpga-test \
	doc-build doc-clean doc-test \
	clean \
	test-pc-emul test-pc-emul-clean \
	test-sim test-sim-clean \
	test-fpga test-fpga-clean \
	test-doc test-doc-clean \
	test test-clean \
	debug
	#ila-build ila-generate-vcd ila-clean

Makefile: ;

.SUFFIXES:

