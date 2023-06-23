SHELL := /bin/bash

ROOT_DIR:=../..

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
LIBS += -lSDL2  -lGL -ldl

#HEADERS
HDR+=periphs.h 

#SOURCES
#SRC+= $(UART_DIR)/software/printf.c

OBJ += ./build/iob-uart.o
OBJ += ./build/iob_uart_swreg_pc_emul.o
OBJ += ./build/iob-timer.o
OBJ += ./build/iob_timer_swreg_pc_emul.o
OBJ += ./build/iob-ila.o
OBJ += ./build/iob_ila_swreg_emb.o
OBJ += ./build/printf.o

./build/iob-uart.o : ../../submodules/UART/software/iob-uart.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/iob-uart.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob_uart_swreg_pc_emul.o : ../../submodules/UART/software/pc-emul/iob_uart_swreg_pc_emul.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/iob_uart_swreg_pc_emul.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob-timer.o : ../../submodules/TIMER/software/iob-timer.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/iob-timer.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob_timer_swreg_pc_emul.o : ../../submodules/TIMER/software/pc-emul/iob_timer_swreg_pc_emul.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/iob_timer_swreg_pc_emul.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob-ila.o : ../../submodules/ILA/software/pc-emul/iob-ila.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/iob-ila.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob_ila_swreg_emb.o : iob_ila_swreg_emb.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/iob_ila_swreg_emb.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/printf.o : ../../submodules/UART/software/printf.c 
	gcc $(LOCAL_CFLAGS) -c -o ./build/printf.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

#INCLUDE+=$(incdir)./build
INCLUDE+=$(VERSAT_INCLUDE)
#OBJ+= $(wildcard ./generated/*.o)

SOC_OUT_BIN:=soc-out.bin

DEFINE+=$(defmacro)"CRYPTO_NAMESPACE(x)=x"

#CONSOLE
CONSOLE_CMD=$(ROOT_DIR)/software/console/eth_console -L

#RULES
ifeq ($(TEST),)
OUTPUT_FOLDER := ./generated
FW_NAME:=./generated/fw_emul
TYPE_NAME:=CustomInstruction
FIRMWARE:=$(FIRM_DIR)/firmware.cpp
FIRMWARE_GENERATED:=$(FIRM_DIR)/generated
OPTS:=-s
else
OUTPUT_FOLDER := ./test/$(TEST)
FW_NAME:=./test/$(TEST)/fw_emul
TYPE_NAME:=$(TEST)
FIRMWARE:=$(FIRM_DIR)/Tests/$(TEST).cpp
OPTS:=-s
FIRMWARE_GENERATED:=$(FIRM_DIR)/generated
endif

INCLUDE+=$(incdir)$(OUTPUT_FOLDER)

temp := $(shell mkdir -p $(OUTPUT_FOLDER))
temp2 := $(shell mkdir -p ./build)
temp3 := $(shell $(MAKE) -C $(VERSAT_DIR) versat) # This is a bad way of doing things because the clean target now takes forever
temp4 := $(shell mkdir -p $(FIRMWARE_GENERATED))

VERSAT_EXE:=$(VERSAT_DIR)/versat

SRC+=$(FIRM_DIR)/Tests/testbench.hpp # Header but contains code common to all testbenches

build: gen_data $(FW_NAME)

force-build: gen_data
	g++ -w -o fw_emul $(GLOBAL_CFLAGS) $(DEFINE) $(INCLUDE) ./build/*.o $(FIRM_DIR)/firmware.cpp $(LIBS) #-fuse-ld=lld 

$(OUTPUT_FOLDER)/accel.hpp: $(VERSAT_EXE) $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v) testVersatSpecification.txt
	$(VERSAT_EXE) $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v) -I $(VERSAT_DIR)/hardware/include testVersatSpecification.txt $(OPTS) -T $(TYPE_NAME) -o $(OUTPUT_FOLDER)
	$(MAKE) -C $(OUTPUT_FOLDER) all

$(FIRMWARE_GENERATED)/accel.hpp: $(VERSAT_EXE) $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v) testVersatSpecification.txt
	$(VERSAT_EXE) $(SHARED_HARDWARE) $(wildcard $(HW_DIR)/src/units/*.v) -I $(VERSAT_DIR)/hardware/include testVersatSpecification.txt -T $(TYPE_NAME) -o $(FIRMWARE_GENERATED)

$(FW_NAME): $(FIRMWARE_GENERATED)/accel.hpp $(OUTPUT_FOLDER)/accel.hpp $(HDR) $(SRC) $(UNIT_HDR) $(CPP_OBJ) $(FIRMWARE) $(OBJ) $(wildcard $(OUTPUT_FOLDER)/*.o) $(FIRMWARE) 
	g++ -g -w -o $(FW_NAME) $(GLOBAL_CFLAGS) $(DEFINE) $(INCLUDE) -I$(VERILATOR_INCLUDE) -I$(OUTPUT_FOLDER) $(FIRMWARE) $(VERSAT_SPEC_OBJS) $(SRC) ./build/*.o $(OUTPUT_FOLDER)/*.o $(LIBS)

run: $(FW_NAME)
	@cd $(OUTPUT_FOLDER); ./fw_emul

clean:
	@rm -rf fw_emul periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) $(SOC_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG)
	@rm -f versat_data.inc versat_defs.vh versat_instance.v versat_external_memory_inst.vh versat_external_memory_port.vh versat_external_memory_portmap.vh
	@$(MAKE) -C $(SW_DIR)/python clean
	@$(MAKE) -C $(SW_TEST_DIR) clean
	@rm -f -r ./test ./build ./debug ./src ./generated

validate:
	$(MAKE) -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

profile: clean $(EMUL_PROFILE_LOG)
	@printf "\n=== PROFILE LOG ===\n"
	@cat $(EMUL_PROFILE_LOG)
	@printf "=== PROFILE LOG ===\n"

$(SOC_LOG): 
	$(MAKE) run PROFILE=1 TEST_LOG="> $(SOC_LOG)"

$(EMUL_PROFILE_LOG): $(SOC_LOG)
	@grep "PROFILE:" $< > $(EMUL_PROFILE_LOG)

build-test: $(FW_NAME)

run-test:
	@cd $(OUTPUT_FOLDER); ./fw_emul;

.PHONY: build run versat \
	test validate profile \
	clean force-build

