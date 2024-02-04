# (c) 2022-Present IObundle, Lda, all rights reserved
#
# This makefile is used at build-time in $(BUILD_DIR)/software/Makefile
#

include ../config_build.mk

-include VerilatorMake.mk

#########################################
#            General settings           #
#########################################

PYTHON_DIR = ../scripts

# include local build segment
# all previously defined variables can be overwritten in this file
ifneq ($(wildcard sw_build.mk),)
include sw_build.mk
endif

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
CFLAGS ?=-Os -nostdlib -march=$(MFLAGS) -mabi=ilp32 --specs=nano.specs -Wcast-align=strict $(USER_CFLAGS)
LFLAGS ?= -Wl,-Bstatic,-T,$(TEMPLATE_LDS),--strip-debug
LLIBS ?=-lgcc -lc -lnosys
INCLUDES ?=-I. -Isrc

#########################################
#         PC emulation settings         #
#########################################

# compiler flags
EMUL_CFLAGS ?=-Os -std=gnu99 -Wl,--strip-debug

CONSOLE_CMD ?=rm -f soc2cnsl cnsl2soc; ../scripts/console.py -L

EMUL_INCLUDES ?=-I. -Isrc

EMUL_HDR+=$(wildcard src/*_emul.h)

EMUL_SRC+=$(wildcard src/*_emul.c)

#########################################
#            Embedded targets           #
#########################################

test_emb.o: src/test.cpp 
	$(TOOLCHAIN_PREFIX)g++ -c -o $@ $(CFLAGS) $(LFLAGS) $(INCLUDES) $< $(LLIBS)

build: $(UTARGETS)

iob_soc_versat_firmware.elf: $(TEMPLATE_LDS) $(HDR) test_emb.o  $(SRC) 
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(INCLUDES) $(SRC) test_emb.o $(LLIBS)
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

fw_emul: $(EMUL_HDR) $(EMUL_SRC) test.cpp libaccel.a
	gcc -o $@ $(EMUL_CFLAGS) $(EMUL_INCLUDES) $(EMUL_SRC) test.o -lgcc -lc -lm -ldl libaccel.a -lstdc++

build_emul: fw_emul
	@echo "build"

#board client command
BOARD_GRAB_CMD ?=../scripts/board_client.py grab 300

test.log: build_emul
	$(BOARD_GRAB_CMD) -s './fw_emul' -c '$(CONSOLE_CMD)'

run_emul: test.log
	test "$$(cat test.log)" = "Test passed!"

.PHONY: build_emul run_emul

#########################################
#            General targets            #
#########################################

clean:
	@rm -rf *.bin *.elf *.map *.hex
	@rm -rf fw_emul test.log soc2cnsl cnsl2soc
	@rm -rf *.txt iob_soc_conf.h

.PHONY: clean
