include $(ROOT_DIR)/config.mk

#add itself to MODULES list
HW_MODULES+=$(IOBSOC_NAME)

#
# ADD SUBMODULES HARDWARE
#

#include LIB modules
include $(LIB_DIR)/hardware/iob_merge/hardware.mk
include $(LIB_DIR)/hardware/iob_split/hardware.mk
include $(LIB_DIR)/hardware/iob_pulse_gen/hardware.mk
include $(LIB_DIR)/hardware/iob_edge_detect/hardware.mk

#include MEM modules
include $(MEM_DIR)/hardware/rom/iob_rom_sp/hardware.mk
include $(MEM_DIR)/hardware/ram/iob_ram_dp_be/hardware.mk

#CPU
#include $(PICORV32_DIR)/hardware/hardware.mk
include $(VEXRISCV_DIR)/hardware/hardware.mk

#CACHE
include $(CACHE_DIR)/hardware/hardware.mk

#UART
include $(UART_DIR)/hardware/hardware.mk

#TIMER
include $(TIMER_DIR)/hardware/hardware.mk

#ETHERNET
include $(ETHERNET_DIR)/hardware/hardware.mk

#VERSAT
include $(VERSAT_DIR)/hardware/hardware.mk

#ILA
include $(ILA_DIR)/hardware/hardware.mk

#AXI
include $(AXI_DIR)/hardware/axiinterconnect/hardware.mk

#HARDWARE PATHS
INC_DIR:=$(HW_DIR)/include
SRC_DIR:=$(HW_DIR)/src

#DEFINES
DEFINE+=$(defmacro)DDR_DATA_W=$(DDR_DATA_W)
DEFINE+=$(defmacro)DDR_ADDR_W=$(DDR_ADDR_W)
DEFINE+=$(defmacro)AXI_ADDR_W=32

#INCLUDES
INCLUDE+=$(incdir). $(incdir)$(INC_DIR) $(incdir)$(LIB_DIR)/hardware/include

#HEADERS
VHDR+=$(INC_DIR)/system.vh $(LIB_DIR)/hardware/include/iob_intercon.vh

#axi wires to connect cache to external memory in system top
VHDR+=m_axi_wire.vh
m_axi_wire.vh:
	$(LIB_DIR)/software/python/axi_gen.py axi_wire 'm_' 'm_' 'm_'

#SOURCES

#external memory interface
ifeq ($(USE_DDR),1)
VSRC+=$(SRC_DIR)/ext_mem.v
endif

ifeq ($(TEST),)
OUTPUT_SIM_FOLDER := $(HW_DIR)/simulation/$(SIMULATOR)
INPUT_FIRM_FOLDER := $(FIRM_DIR)/
VSRC+=$(FIRM_DIR)/generated/versat_instance.v
VSRC+=$(wildcard $(FIRM_DIR)/generated/src/*.v)
INCLUDE+=$(incdir)$(FIRM_DIR)/generated
INCLUDE+=$(incdir)$(FIRM_DIR)/src
else
OUTPUT_SIM_FOLDER := $(HW_DIR)/simulation/$(SIMULATOR)/test/$(TEST)
INPUT_FIRM_FOLDER := $(FIRM_DIR)/test/$(TEST)
VSRC+=$(FIRM_DIR)/test/$(TEST)/versat_instance.v
VSRC+=$(wildcard $(FIRM_DIR)/test/$(TEST)/src/*.v)
INCLUDE+=$(incdir)$(FIRM_DIR)/test/$(TEST)/src
INCLUDE+=$(incdir)$(FIRM_DIR)/test/$(TEST)
endif

#system
VSRC+=$(SRC_DIR)/boot_ctr.v $(SRC_DIR)/int_mem.v $(SRC_DIR)/sram.v
VSRC+=system.v

VSRC+=$(wildcard $(SRC_DIR)/units/*.v)

HEXPROGS=$(OUTPUT_SIM_FOLDER)/boot.hex $(OUTPUT_SIM_FOLDER)/firmware.hex

# make system.v with peripherals
system.v: $(SRC_DIR)/system_core.v
	cp $< $@
	$(foreach p, $(PERIPHERALS), $(eval HFILES=$(shell echo `ls $($p_DIR)/hardware/include/*.vh | grep -v pio | grep -v inst | grep -v swreg | grep -v port`)) \
	$(eval HFILES+=$(notdir $(filter %swreg_def.vh, $(VHDR)))) \
	$(if $(HFILES), $(foreach f, $(HFILES), sed -i '/PHEADER/a `include \"$f\"' $@;),)) # insert header files
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed -i '/PIO/r $($p_DIR)/hardware/include/pio.vh' $@; fi;) #insert system IOs for peripheral
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/inst.vh; then sed -i '/endmodule/e cat $($p_DIR)/hardware/include/inst.vh' $@; fi;) # insert peripheral instances

# make and copy memory init files
$(OUTPUT_SIM_FOLDER)/boot.hex: $(BOOT_DIR)/boot.bin
	@mkdir -p $(OUTPUT_SIM_FOLDER)
	$(PYTHON_DIR)/makehex.py $< $(BOOTROM_ADDR_W) > $@

$(OUTPUT_SIM_FOLDER)/firmware.hex: $(INPUT_FIRM_FOLDER)/firmware.bin
	@mkdir -p $(OUTPUT_SIM_FOLDER)
	$(PYTHON_DIR)/makehex.py $< $(FIRM_ADDR_W) > $@
ifeq ($(MIG_BUS_W),64)
	$(PYTHON_DIR)/makehex.py $< $(FIRM_ADDR_W) > firmwareBase.hex
	$(SW_DIR)/python/convertFirmware.py firmwareBase.hex 64 > $@
endif
ifeq ($(MIG_BUS_W),128)
	$(PYTHON_DIR)/makehex.py $< $(FIRM_ADDR_W) > firmwareBase.hex
	$(SW_DIR)/python/convertFirmware.py firmwareBase.hex 128 > $@
endif

boot.hex: $(BOOT_DIR)/boot.bin
	$(PYTHON_DIR)/makehex.py $< $(BOOTROM_ADDR_W) > $@

firmware.hex: $(FIRM_DIR)/firmware.bin
	$(PYTHON_DIR)/makehex.py $< $(FIRM_ADDR_W) > $@
#	$(PYTHON_DIR)/hex_split.py firmware .

#clean general hardware files
hw-clean: gen-clean
	@rm -f *.v *.vh *.hex *.bin $(SRC_DIR)/system.v $(TB_DIR)/system_tb.v $(SRC_DIR)/GeneratedUnits/*.v $(SRC_DIR)/versat_instance.v 
	@rm -f $(INC_DIR)/versat_defs.vh $(INC_DIR)/versat_external_memory_inst.vh $(INC_DIR)/versat_external_memory_port.vh $(INC_DIR)/versat_external_memory_portmap.vh

.PHONY: hw-clean
