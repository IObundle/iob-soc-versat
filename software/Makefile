# (c) 2022-Present IObundle, Lda, all rights reserved
#
# This makefile is used at build-time in $(BUILD_DIR)/software/Makefile
#

include ../config_build.mk

include VerilatorMake.mk

#########################################
#           Embedded settings           #
#########################################

TEMPLATE_LDS ?=template.lds

MFLAGS=$(MFLAGS_BASE)$(MFLAG_M)$(MFLAG_C)
MFLAGS_BASE:=rv32i
ifeq ($(USE_MUL_DIV),1)
MFLAG_M=m
endif
ifeq ($(USE_COMPRESSED),1)
MFLAG_C=c
endif

#default compiler settings
TOOLCHAIN_PREFIX ?=riscv64-unknown-elf-
CFLAGS ?=-Os -nostdlib -march=$(MFLAGS) -mabi=ilp32 --specs=nano.specs -Wcast-align=strict
LFLAGS ?= -Wl,-Bstatic,-T,$(TEMPLATE_LDS),--strip-debug
LLIBS ?=-lgcc -lc -lnosys
INCLUDES ?=-I. -Isrc

#########################################
#         PC emulation settings         #
#########################################

# compiler flags
EMUL_CFLAGS=-g -DPC #-Os -std=gnu99 -Wl,--strip-debug

CONSOLE_CMD=rm -f soc2cnsl cnsl2soc; ../scripts/console.py -L

EMUL_INCLUDES=-I. -Isrc

EMUL_HDR+=$(wildcard src/*_emul.h)

EMUL_SRC+=$(wildcard src/*_emul.c)

#########################################
#            General settings           #
#########################################

# include local build segment
# all previously defined variables can be overwritten in this file
ifneq ($(wildcard sw_build.mk),)
include sw_build.mk
endif

#########################################
#            Embedded targets           #
#########################################

test_emb.o: src/test.cpp 
	$(TOOLCHAIN_PREFIX)g++ -c -o $@ -Os -nostdlib -march=$(MFLAGS) -mabi=ilp32 $(INCLUDES) $< $(LLIBS)

build: $(UTARGETS)
	
iob_soc_versat_firmware.elf: $(TEMPLATE_LDS) $(HDR) $(SRC) test_emb.o
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(INCLUDES) $(SRC) test_emb.o $(LLIBS)
	$(TOOLCHAIN_PREFIX)objdump -d $@ > assembly.asm
	$(TOOLCHAIN_PREFIX)objcopy -O binary $@ iob_soc_versat_firmware.bin

iob_soc_versat_boot.elf: $(TEMPLATE_LDS) $(HDR) $(SRC)
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(INCLUDES) $(SRC) $(LLIBS)
	$(TOOLCHAIN_PREFIX)objcopy -O binary $@ iob_soc_versat_boot.bin

ifneq ($(BOARD),)
FPGA_TOOL:=$(shell find ../hardware/fpga -name $(BOARD) | cut -d"/" -f5)
endif

debug:
	@echo $(TEMPLATE_LDS)
	@echo $(FW_SRC)
	@echo $(BOOT_SRC)
	@echo $(HDR)

.PHONY: build debug

#########################################
#         PC emulation targets          #
#########################################

test.o: src/test.cpp 
	g++ -c -o $@ $< $(EMUL_CFLAGS) $(EMUL_INCLUDES)

fw_emul: $(EMUL_HDR) $(EMUL_SRC) test.o libaccel.a
	gcc -o $@ $(EMUL_CFLAGS) $(EMUL_INCLUDES) test.o $(filter-out src/iob-versat.c,$(EMUL_SRC)) -lgcc -lc -lm -ldl libaccel.a -lstdc++

build_emul: fw_emul
	@echo "build"

#board client command
BOARD_GRAB_CMD=../scripts/board_client.py grab 300

run_emul: build_emul
	$(BOARD_GRAB_CMD) -s './fw_emul' -c '$(CONSOLE_CMD)'

test_emul: run_emul
	test "$$(cat test.log)" = "Test passed!"

.PHONY: build_emul run_emul test_emul

#########################################
#            General targets            #
#########################################

clean:
	@rm -rf *.bin *.elf *.map *.hex
	@rm -rf fw_emul test.log soc2cnsl cnsl2soc
	@rm -rf *.txt iob_soc_conf.h

.PHONY: clean
