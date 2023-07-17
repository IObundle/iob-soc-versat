defmacro:=-D
incdir:=-I
include $(ROOT_DIR)/config.mk

DEFINE+=$(defmacro)BAUD=$(BAUD)
DEFINE+=$(defmacro)FREQ=$(FREQ)

#compiler settings
TOOLCHAIN_PREFIX:=riscv64-unknown-elf-
CFLAGS=-Os -O2 -nostdlib -march=$(MFLAGS) -mabi=ilp32 --specs=nano.specs -Wcast-align=strict
LFLAGS+= -Wl,-Bstatic,-T,../template.lds,--strip-debug
LLIBS=-lgcc -lc -lnosys

MFLAGS=$(MFLAGS_BASE)$(MFLAG_M)$(MFLAG_C)

MFLAGS_BASE:=rv32i

ifeq ($(USE_MUL_DIV),1)
MFLAG_M=m
endif

ifeq ($(USE_COMPRESSED),1)
MFLAG_C=c
endif

#INCLUDE
INCLUDE+=$(incdir)$(SW_DIR) $(incdir).

#add iob-lib to include path
INCLUDE+=$(incdir)$(LIB_DIR)/software/include

#headers
HDR=$(SW_DIR)/system.h periphs.h

include $(VERSAT_DIR)/sharedHardware.mk
VERSAT_EXE:=$(VERSAT_DIR)/versat

ifeq ($(TEST),)
TYPE_NAME:=SMVMCPU
FIRMWARE:=$(FIRM_DIR)/firmware.c
COMPILER:=gcc
else
TYPE_NAME:=$(TEST)
ifneq ($(wildcard $(FIRM_DIR)/Tests/$(TEST).c),)
	FIRMWARE:=$(FIRM_DIR)/Tests/$(TEST).c
	COMPILER:=gcc
else
	FIRMWARE:=$(FIRM_DIR)/Tests/$(TEST).cpp
	COMPILER:=g++
endif
endif

VERSAT_THIS_HARDWARE := $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v)
VERSAT_THIS_INCLUDE :=  -I $(VERSAT_DIR)/hardware/include -I $(HW_DIR)/include -I $(HW_DIR)/src/units

versat:
	$(MAKE) -C $(VERSAT_DIR) versat

$(VERSAT_EXE): versat # Makes sure versat is updated

#common sources (none so far)
#SRC=$(SW_DIR)/*.c

#peripherals' base addresses
periphs.h: periphs_tmp.h
	@is_diff=`diff -q -N $@ $<`; if [ "$$is_diff" ]; then cp $< $@; fi
	#@rm periphs_tmp.h

periphs_tmp.h:
	$(foreach p, $(PERIPHERALS), $(shell echo "#define $p_BASE (1<<$P) |($p<<($P-N_SLAVES_W))" >> $@) )

build-all:
	$(MAKE) -C $(FIRM_DIR) build
	$(MAKE) -C $(BOOT_DIR) build

debug:
	echo $(DEFINE)
	echo $(INCLUDE)

clean-all: gen-clean
	$(MAKE) -C $(FIRM_DIR) clean
	$(MAKE) -C $(BOOT_DIR) clean

.PHONY: build-all debug clean-all
