SHELL := /bin/bash

ROOT_DIR:=../..

GLOBAL_CFLAGS:=-Wall -g -march=native -DVERSAT_DEBUG #-m32 #-msse2 #-fsanitize=address
LIBS := -lstdc++ -lm -lgcc -lc -lssl -lcrypto -pthread

include ../software.mk

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

SOC_LOG:=soc.log
ETH_LOG:=ethernet.log
EMUL_PROFILE_LOG:=emul_profile.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $($p_DIR)/software/pc-emul/pc-emul.mk))

#HEADERS
HDR+=periphs.h 

HDR+=$(FIRM_DIR)/unitConfiguration.hpp

#SOURCES
SRC+= $(UART_DIR)/software/printf.c

OBJ += $(foreach name,$(basename $(notdir $(wildcard $(FIRM_DIR)/crypto/mceliece/mceliece348864/*.c))),./build/$(name).o)
OBJ += $(foreach name,$(basename $(notdir $(wildcard $(FIRM_DIR)/crypto/mceliece/*.c))),./build/$(name).o)
OBJ += $(foreach name,$(basename $(notdir $(wildcard $(FIRM_DIR)/crypto/blake2/*.c))),./build/$(name).o)
OBJ += $(foreach name,$(basename $(notdir $(wildcard $(FIRM_DIR)/crypto/*.c))),./build/$(name).o)

OBJ += ./build/iob-uart.o
OBJ += ./build/iob_uart_swreg_pc_emul.o
OBJ += ./build/iob-timer.o
OBJ += ./build/iob_timer_swreg_pc_emul.o
OBJ += ./build/iob-ila.o
OBJ += ./build/iob_ila_swreg_emb.o
OBJ += ./build/printf.o

./build/iob-uart.o : ../../submodules/UART/software/iob-uart.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/iob-uart.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob_uart_swreg_pc_emul.o : ../../submodules/UART/software/pc-emul/iob_uart_swreg_pc_emul.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/iob_uart_swreg_pc_emul.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob-timer.o : ../../submodules/TIMER/software/iob-timer.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/iob-timer.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob_timer_swreg_pc_emul.o : ../../submodules/TIMER/software/pc-emul/iob_timer_swreg_pc_emul.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/iob_timer_swreg_pc_emul.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob-ila.o : ../../submodules/ILA/software/pc-emul/iob-ila.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/iob-ila.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/iob_ila_swreg_emb.o : iob_ila_swreg_emb.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/iob_ila_swreg_emb.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/printf.o : ../../submodules/UART/software/printf.c 
	gcc $(GLOBAL_CFLAGS) -c -o ./build/printf.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/%.o: $(FIRM_DIR)/crypto/mceliece/mceliece348864/%.c
	gcc $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/%.o: $(FIRM_DIR)/crypto/mceliece/%.c
	gcc $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/%.o: $(FIRM_DIR)/crypto/blake2/%.c
	gcc $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/%.o: $(FIRM_DIR)/crypto/%.c
	gcc $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

INCLUDE+=$(incdir)./build

SOC_OUT_BIN:=soc-out.bin

#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
INCLUDE+=$(incdir)$(FIRM_DIR)
endif

DEFINE+=$(defmacro)"CRYPTO_NAMESPACE(x)=x"

#UNITS TO VERILATE
VERILATE_UNIT := xunitF xunitM
UNIT_VERILOG += $(foreach unit,$(VERILATE_UNIT),$(HW_DIR)/src/units/$(unit).v)

UNIT_HDR+=$(foreach obj,$(VERILATE_UNIT),./build/V$(obj).h)
OBJ+=./build/firmware.o
OBJ+= ./build/unitConfiguration.o
OBJ+= ./build/versatSHA.o
OBJ+= ./build/versatMcEliece.o
OBJ+= ./build/VersatMachSuite.o
OBJ+= ./build/VersatBlake2.o
OBJ+= ./build/automaticTests.o

#CONSOLE
CONSOLE_CMD=$(ROOT_DIR)/software/console/eth_console -L

#RULES

build: gen_data fw_emul
	@mkdir -p $(HW_DIR)/src/GeneratedUnits/
	@mkdir -p debug
	@mkdir -p src

fw_emul: $(HDR) $(SRC) $(UNIT_HDR) $(CPP_OBJ) $(OBJ)
	gcc -w -o $@ $(GLOBAL_CFLAGS) $(DEFINE) $(INCLUDE) ./build/*.o $(LIBS) #-fuse-ld=lld 
	
run: build
	./fw_emul $(TEST_LOG)
	-cp versat_instance.v $(HW_DIR)/src/
	-cp versat_defs.vh $(HW_DIR)/include/
	-cp versat_data.inc $(FIRM_DIR)/
	-cp ./build/verilogWrapper.inc $(FIRM_DIR)/
	-cp src/* $(HW_DIR)/src/GeneratedUnits/

clean:
	@rm -rf fw_emul periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) $(SOC_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG)
	@rm -f versat_data.inc versat_defs.vh versat_instance.v
	@$(MAKE) -C $(SW_DIR)/python clean
	@$(MAKE) -C $(SW_TEST_DIR) clean
	@rm -f -r ./build
	@rm -f -r ./debug
	@rm -f -r ./src
	@rm -f -r ./trace_out
	@rm -f -r $(HW_DIR)/src/GeneratedUnits

test: 
	$(MAKE) run
	$(MAKE) validate

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

./build/%.o: $(FIRM_DIR)/%.cpp $(UNIT_OBJ)
	g++ $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $(FIRM_DIR)/$*.cpp -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/V%.h: $(HW_DIR)/src/units/%.v
	verilator -CFLAGS "$(VERILATE_FLAGS)" -I$(VERSAT_HW_DIR)/include --trace --cc -Mdir ./obj $<;
	VM_DEFAULT_RULES=0 $(MAKE) -C ./obj -f V$*.mk;
	mkdir -p ./build; mv ./obj/*.o ./build/;
	mv ./obj/*.h ./build/
	rm -r -f ./obj

.PHONY: build run \
	test validate profile \
	clean

