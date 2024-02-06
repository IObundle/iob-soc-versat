#!/bin/bash 

test=$1

mkdir -p ../iob_soc_versat_V0.70_$test
make setup TEST=$test &> ../iob_soc_versat_V0.70_$test/setup.txt