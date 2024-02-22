VTOP?=$(NAME)

VFLAGS+=--cc --exe -I. -I../src -Isrc --top-module $(VTOP)
VFLAGS+=-Wno-lint --Wno-UNOPTFLAT
VFLAGS+=--no-timing
# Include embedded headers
VFLAGS+=--threads 4 --trace-coverage --coverage -O3 -CFLAGS "-O1 -march=native -I../../../software/src -I../../../software"

ifeq ($(VCD),1)
VFLAGS+=--trace-fst
VFLAGS+=-DVCD -CFLAGS "-DVCD"
endif

SIM_SERVER=$(VSIM_SERVER)
SIM_USER=$(VSIM_USER)

SIM_OBJ=V$(VTOP)

comp: $(VHDR) $(VSRC) $(HEX)
	verilator $(VFLAGS) $(VSRC) src/$(NAME)_tb.cpp
	cd ./obj_dir && make -j 8 -f $(SIM_OBJ).mk

exec: comp
	./obj_dir/$(SIM_OBJ)

clean: gen-clean
	@rm -rf obj_dir

very-clean: clean

.PHONY: comp exec clean
