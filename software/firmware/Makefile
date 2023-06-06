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

#versat
include $(VERSAT_DIR)/software/embedded/embedded.mk

#ila
include $(ILA_DIR)/software/embedded/embedded.mk

#HEADERS
HDR+=periphs.h

#SOURCES
SRC+= firmware.S firmware.cpp $(UART_DIR)/software/printf.c
SRC+= $(wildcard  $(FIRM_DIR)/crypto/*.c)
SRC+= $(FIRM_DIR)/ila.c

#INCLUDE+=$(incdir)$(FIRM_DIR)/../pc-emul/build
#SRC+= $(wildcard $(FIRM_DIR)/crypto/mceliece/mceliece348864/*.c)
#SRC+= $(wildcard $(FIRM_DIR)/crypto/mceliece/*.c)

ifeq ($(HARDWARE_TEST),)
INCLUDE+=$(incdir)$(FIRM_DIR)/../pc-emul/build
INCLUDE+=$(incdir)$(FIRM_DIR)/../pc-emul/generated
OUTPUT_SIM_FOLDER := $(HW_DIR)/simulation/verilator
OUTPUT_DIR:=.
OUTPUT_NAME:=firmware.elf
else
INCLUDE+=$(incdir)$(FIRM_DIR)/../pc-emul/build
INCLUDE+=$(incdir)$(FIRM_DIR)/../pc-emul/test/$(HARDWARE_TEST)/
OUTPUT_SIM_FOLDER := $(HW_DIR)/simulation/verilator/test/$(HARDWARE_TEST)
OUTPUT_DIR:=./test/$(HARDWARE_TEST)
OUTPUT_NAME:=./test/$(HARDWARE_TEST)/firmware.elf
endif

#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
endif

#SIMULATION
ifeq ($(SIM),1)
DEFINE+=$(defmacro)SIM
endif

#RULES

#build: firmware.elf

#firmware.elf: ../template.lds $(HDR) $(SRC) gen_data
#	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(LLIBS) -lgcc -lc -lnosys -fno-exceptions
#	$(TOOLCHAIN_PREFIX)objcopy -O binary firmware.elf firmware.bin

build: $(OUTPUT_NAME)

$(OUTPUT_NAME): ../template.lds $(HDR) $(SRC) gen_data
ifneq ($(HARDWARE_TEST),)
	@mkdir -p $(OUTPUT_DIR)
endif
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(LLIBS) -lgcc -lc -lnosys -fno-exceptions
	$(TOOLCHAIN_PREFIX)objdump -drwC -S $@ > assembly.asm
	$(TOOLCHAIN_PREFIX)objcopy -O binary $(OUTPUT_DIR)/firmware.elf $(OUTPUT_DIR)/firmware.bin

clean:
	@rm -rf firmware.bin firmware.elf firmware.map *.hex periphs.h UARTsw_reg.h TIMERsw_reg.h *swreg.h \
		*swreg_emb.c \
		*d_in.bin *d_out.bin \
		verilogWrapper.inc versat_data.inc

.PHONY: build clean
