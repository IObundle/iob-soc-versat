SHELL := /bin/bash

ROOT_DIR:=$(abspath $(shell pwd)/../..)

DEBUG_GUI := 0

LOCAL_CFLAGS := -Wall -g -Werror=return-type -march=native -DVERSAT_DEBUG
GLOBAL_CFLAGS:= $(LOCAL_CFLAGS) -std=c++17 #-fsanitize=address #-msse2

include ../software.mk

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

SOC_LOG:=soc.log
ETH_LOG:=ethernet.log
EMUL_PROFILE_LOG:=emul_profile.log

#SUBMODULES
VERSAT_SPEC_INCLUDE:=-I ./generated -I ./build -I ./src

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $($p_DIR)/software/pc-emul/pc-emul.mk))

LIBS := $(VERSAT_DIR)/libversat.a -lstdc++ -lm -lgcc -lc -pthread -ldl

ifeq ($(VERSAT_DEBUG),1)
LIBS += -lSDL2  -lGL -ldl
endif

# Remove sources that we will precompile into objects
OBJ_SOURCES:= ../../submodules/UART/software/iob-uart.c ../../submodules/UART/software/pc-emul/iob_uart_swreg_pc_emul.c ../../submodules/TIMER/software/iob-timer.c ../../submodules/TIMER/software/pc-emul/iob_timer_swreg_pc_emul.c ../../submodules/ILA/software/pc-emul/iob-ila.c ./iob_ila_swreg_emb.c ../../submodules/UART/software/printf.c 

OBJ_SOURCES:=$(foreach file,$(OBJ_SOURCES),$(realpath $(file)))
OBJ_SOURCES+=iob_ila_swreg_emb.c
SRC:=$(filter-out $(OBJ_SOURCES),$(SRC))

OBJ += ./build/iob-uart.o
OBJ += ./build/iob_uart_swreg_pc_emul.o
OBJ += ./build/iob-timer.o
OBJ += ./build/iob_timer_swreg_pc_emul.o
OBJ += ./build/iob-ila.o
OBJ += ./build/iob_ila_swreg_emb.o
OBJ += ./build/printf.o

./build/iob-uart.o : ../../submodules/UART/software/iob-uart.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/iob-uart.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 
./build/iob_uart_swreg_pc_emul.o : ../../submodules/UART/software/pc-emul/iob_uart_swreg_pc_emul.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/iob_uart_swreg_pc_emul.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 
./build/iob-timer.o : ../../submodules/TIMER/software/iob-timer.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/iob-timer.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 
./build/iob_timer_swreg_pc_emul.o : ../../submodules/TIMER/software/pc-emul/iob_timer_swreg_pc_emul.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/iob_timer_swreg_pc_emul.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 
./build/iob-ila.o : ../../submodules/ILA/software/pc-emul/iob-ila.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/iob-ila.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 
./build/iob_ila_swreg_emb.o : iob_ila_swreg_emb.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/iob_ila_swreg_emb.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 
./build/printf.o : ../../submodules/UART/software/printf.c 
	$(COMPILER) $(LOCAL_CFLAGS) -c -o ./build/printf.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

SOC_OUT_BIN:=soc-out.bin

DEFINE+=$(defmacro)"CRYPTO_NAMESPACE(x)=x"

#CONSOLE
CONSOLE_CMD=$(ROOT_DIR)/software/console/console -L

#RULES
ifeq ($(TEST),)
OUTPUT_FOLDER:=./generated
FW_NAME:=./generated/fw_emul
else
OUTPUT_FOLDER:=./test/$(TEST)
FW_NAME:=./test/$(TEST)/fw_emul
endif

OPTS:=-s

INCLUDE+=$(incdir)$(OUTPUT_FOLDER)

temp := $(shell mkdir -p $(OUTPUT_FOLDER))
temp2 := $(shell mkdir -p ./build)

EXTRA_SOURCES:=$(foreach file,$(VSRC) $(VERSAT_VSRC),-S $(file))
HDR+=$(FIRM_DIR)/Tests/testbench.h # Header but contains code common to all testbenches

LIBS+=$(OUTPUT_FOLDER)/libaccel.a

$(OUTPUT_FOLDER)/versat_accel.h: $(VERSAT_EXE) $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v) testVersatSpecification.txt
	$(VERSAT_EXE) $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v) $(EXTRA_SOURCES) $(V_INCLUDE) -I $(VERSAT_DIR)/hardware/include -I $(HW_DIR)/include testVersatSpecification.txt $(OPTS) -T $(TYPE_NAME) -o $(OUTPUT_FOLDER)
	$(MAKE) -C $(OUTPUT_FOLDER) all

$(FW_NAME): $(OUTPUT_FOLDER)/versat_accel.h $(HDR) $(SRC) $(UNIT_HDR) $(CPP_OBJ) $(FIRMWARE) $(OBJ) $(wildcard $(OUTPUT_FOLDER)/*.o) $(FIRMWARE) 
	$(COMPILER) -DPC -g -w -o $(FW_NAME) $(GLOBAL_CFLAGS) $(DEFINE) $(INCLUDE) -I$(VERILATOR_INCLUDE) -I$(OUTPUT_FOLDER) $(FIRMWARE) $(VERSAT_SPEC_OBJS) $(SRC) ./build/*.o $(OUTPUT_FOLDER)/*.o $(LIBS)  

build: $(FW_NAME)

run: $(FW_NAME)
	#$(CONSOLE_CMD) & $(OUTPUT_FOLDER)/fw_emul
	@cd $(OUTPUT_FOLDER); ./fw_emul

clean:
	@rm -rf fw_emul periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) $(SOC_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG)
	@rm -f versat_data.inc versat_defs.vh versat_instance.v versat_external_memory_inst.vh versat_external_memory_port.vh versat_external_memory_portmap.vh
	@rm -f -r ./test ./build ./debug ./src ./generated periphs_tmp.h

.PHONY: build run versat test \
	clean force-build

