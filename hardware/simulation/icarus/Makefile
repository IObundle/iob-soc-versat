ROOT_DIR:=../../..

defmacro:=-D
incdir:=-I

SIMULATOR=icarus
SIM_SERVER=$(IVSIM_SERVER)
SIM_USER=$(IVSIM_USER)
SIM_PROC=a.out

include ../simulation.mk

ifeq ($(TEST),)
SIM_FOLDER:=.
else
SIM_FOLDER:=test/$(TEST)
endif

#simulator flags
VLOG = iverilog -pfileline=1 -W all -g2005-sv $(INCLUDE) $(DEFINE)

comp: $(SIM_FOLDER)/$(SIM_PROC)

#simulation executable
$(SIM_FOLDER)/$(SIM_PROC):
ifneq ($(TEST),)
	@mkdir -p test/$(TEST)
endif
	$(VLOG) $(VSRC)
	cp $(SIM_PROC) $(SIM_FOLDER)/

exec:
	cd $(SIM_FOLDER); ./$(SIM_PROC) -fst

clean: clean-remote
	@rm -f $(SIM_PROC) $(HDR) *.mem system.fst system.vcd
	@rm -rf $(SIM_FOLDER) test

.PHONY: comp exec clean debug
