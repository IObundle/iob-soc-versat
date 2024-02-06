#!/bin/bash 

test=$1

mkdir -p ../iob_soc_versat_V0.70_$test
make sim-build &> ../iob_soc_versat_V0.70_$test/sim-build.txt