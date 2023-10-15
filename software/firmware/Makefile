ROOT_DIR=../..
include ../software.mk

#additional linker flag
LFLAGS+=-Wl,-Map,firmware.map

#cache
ifeq ($(USE_DDR),1)
include $(CACHE_DIR)/software/software.mk
endif

#uart
include $(UART_DIR)/software/embedded/embedded.mk

#timer
include $(TIMER_DIR)/software/embedded/embedded.mk

#ethernet
include $(ETHERNET_DIR)/software/embedded/embedded.mk

#versat
include $(VERSAT_DIR)/software/embedded/embedded.mk

#ila
include $(ILA_DIR)/software/embedded/embedded.mk

#HEADERS
HDR+=periphs.h

#SOURCES
SRC+= firmware.S $(FIRMWARE) $(UART_DIR)/software/printf.c
SRC+= $(wildcard  $(FIRM_DIR)/crypto/*.c)
SRC+= $(FIRM_DIR)/ila.c

ifeq ($(TEST),)
OUTPUT_DIR:=.
OUTPUT_NAME:=firmware.elf
GENERATED:=$(FIRM_DIR)/generated
temp := $(shell mkdir -p $(GENERATED))
else
OUTPUT_DIR:=./test/$(TEST)
OUTPUT_NAME:=./test/$(TEST)/firmware.elf
temp := $(shell mkdir -p $(OUTPUT_DIR))
GENERATED:=$(FIRM_DIR)/test/$(TEST)/
temp2 := $(shell mkdir -p $(GENERATED))
endif

INCLUDE+=$(incdir)$(GENERATED)

#SIMULATION
ifeq ($(SIM),1)
DEFINE+=$(defmacro)SIM
endif

#RULES

ifeq ($(COMPILER),g++)
SRC+=$(CPP_SRC)
endif

#build: firmware.elf

EXTRA_SOURCES:=$(foreach file,$(EXTRA_VERSAT_SRC),-S $(file))

$(GENERATED)/versat_accel.h: $(VERSAT_EXE)
	$(VERSAT_EXE) -d -s -x32 -b=$(MIG_BUS_W) $(VERSAT_THIS_HARDWARE) $(EXTRA_SOURCES) $(V_INCLUDE) $(VERSAT_THIS_INCLUDE) $(FIRM_DIR)/../pc-emul/testVersatSpecification.txt $(OPTS) -T $(TYPE_NAME) -o $(GENERATED)

#firmware.elf: ../template.lds $(HDR) $(SRC) gen_data
#	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(LLIBS) -lgcc -lc -lnosys -fno-exceptions
#	$(TOOLCHAIN_PREFIX)objcopy -O binary firmware.elf firmware.bin

build: $(OUTPUT_NAME)

$(OUTPUT_NAME): ../template.lds $(HDR) $(SRC) $(GENERATED)/versat_accel.h
ifneq ($(HARDWARE_TEST),)
	@mkdir -p $(OUTPUT_DIR)
endif
	$(TOOLCHAIN_PREFIX)$(COMPILER) -std=c++17 -o $@ $(CFLAGS) $(LFLAGS) $(DEFINE) $(INCLUDE) interruptHandler.c $(SRC) $(LLIBS) -lgcc -lc -lnosys -fno-exceptions 
	$(TOOLCHAIN_PREFIX)objdump -drwC -S $@ > assembly.asm
	$(TOOLCHAIN_PREFIX)objcopy -O binary $(OUTPUT_DIR)/firmware.elf $(OUTPUT_DIR)/firmware.bin
	$(TOOLCHAIN_PREFIX)objcopy -O binary $(OUTPUT_DIR)/firmware.elf firmware.bin

clean:
	@rm -rf firmware.bin firmware.elf firmware.map *.hex periphs.h periphs_tmp.h eth_frame_struct.h UARTsw_reg.h TIMERsw_reg.h *swreg.h assembly.asm \
		*swreg_emb.c \
		*d_in.bin *d_out.bin generated src test

.PHONY: build clean
