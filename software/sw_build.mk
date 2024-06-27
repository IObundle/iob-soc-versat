#########################################
#            Embedded targets           #
#########################################
ROOT_DIR ?=..
# Local embedded makefile settings for custom bootloader and firmware targets.

#Function to obtain parameter named $(1) in verilog header file located in $(2)
#Usage: $(call GET_MACRO,<param_name>,<vh_path>)
GET_MACRO = $(shell grep "define $(1)" $(2) | rev | cut -d" " -f1 | rev)

#Function to obtain parameter named $(1) from iob_soc_versat_conf.vh
GET_IOB_SOC_VERSAT_CONF_MACRO = $(call GET_MACRO,IOB_SOC_VERSAT_$(1),../src/iob_soc_versat_conf.vh)

iob_soc_versat_boot.hex: ../../software/iob_soc_versat_boot.bin
	../../scripts/makehex.py $< $(call GET_IOB_SOC_VERSAT_CONF_MACRO,BOOTROM_ADDR_W) > $@

iob_soc_versat_firmware.hex: iob_soc_versat_firmware.bin
ifeq ($(USE_EXTMEM),1)
	../../scripts/makehex.py $< $(call GET_IOB_SOC_VERSAT_CONF_MACRO,MEM_ADDR_W) > $@
else
	../../scripts/makehex.py $< $(call GET_IOB_SOC_VERSAT_CONF_MACRO,SRAM_ADDR_W) > $@
endif
	../../scripts/hex_split.py iob_soc_versat_firmware .

iob_soc_versat_firmware.bin: ../../software/iob_soc_versat_firmware.bin
	cp $< $@

../../software/%.bin:
	make -C ../../ fw-build

UTARGETS+=build_iob_soc_versat_software

TEMPLATE_LDS=src/$@.lds

IOB_SOC_VERSAT_INCLUDES=-I. -Isrc -Isrc/McEliece -Isrc/McEliece/common

IOB_SOC_VERSAT_LFLAGS=-Wl,-Bstatic,-T,$(TEMPLATE_LDS),--strip-debug

# FIRMWARE SOURCES
IOB_SOC_VERSAT_FW_SRC=src/iob_soc_versat_firmware.S
IOB_SOC_VERSAT_FW_SRC+=src/iob_soc_versat_firmware.c
IOB_SOC_VERSAT_FW_SRC+=src/printf.c

IOB_SOC_VERSAT_FW_SRC+=$(wildcard src/McEliece/*.c)
IOB_SOC_VERSAT_FW_SRC+=$(wildcard src/McEliece/common/*.c)

# PERIPHERAL SOURCES
IOB_SOC_VERSAT_FW_SRC+=$(wildcard src/iob-*.c)
IOB_SOC_VERSAT_FW_SRC+=$(filter-out %_emul.c, $(wildcard src/*swreg*.c))

# BOOTLOADER SOURCES
IOB_SOC_VERSAT_BOOT_SRC+=src/iob_soc_versat_boot.S
IOB_SOC_VERSAT_BOOT_SRC+=src/iob_soc_versat_boot.c
IOB_SOC_VERSAT_BOOT_SRC+=$(filter-out %_emul.c, $(wildcard src/iob*uart*.c))
IOB_SOC_VERSAT_BOOT_SRC+=$(filter-out %_emul.c, $(wildcard src/iob*cache*.c))

build_iob_soc_versat_software: iob_soc_versat_firmware iob_soc_versat_boot

iob_soc_versat_firmware:
	make $@.elf INCLUDES="$(IOB_SOC_VERSAT_INCLUDES)" LFLAGS="$(IOB_SOC_VERSAT_LFLAGS) -Wl,-Map,$@.map" SRC="$(IOB_SOC_VERSAT_FW_SRC)" TEMPLATE_LDS="$(TEMPLATE_LDS)"

iob_soc_versat_boot:
	make $@.elf INCLUDES="$(IOB_SOC_VERSAT_INCLUDES)" LFLAGS="$(IOB_SOC_VERSAT_LFLAGS) -Wl,-Map,$@.map" SRC="$(IOB_SOC_VERSAT_BOOT_SRC)" TEMPLATE_LDS="$(TEMPLATE_LDS)"


.PHONY: build_iob_soc_versat_software iob_soc_versat_firmware iob_soc_versat_boot

#########################################
#         PC emulation targets          #
#########################################
# Local pc-emul makefile settings for custom pc emulation targets.

# SOURCES
EMUL_SRC+=src/iob_soc_versat_firmware.c
EMUL_SRC+=src/printf.c

# PERIPHERAL SOURCES
EMUL_SRC+=$(wildcard src/iob-*.c)


#Auto-generated target to create init_ddr_contents.hex
HEX+=init_ddr_contents.hex
# init file for external mem with firmware of both systems
init_ddr_contents.hex: iob_soc_versat_firmware.hex
	../../scripts/joinHexFiles.py $^ - 24 > $@
