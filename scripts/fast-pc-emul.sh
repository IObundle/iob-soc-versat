#!/bin/bash 

BASE=$(tput sgr0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)

test=$1

mkdir -p ../iob_soc_versat_V0.70_$test
make fast-pc-emul TEST=$test &> ../iob_soc_versat_V0.70_$test/fast-pc-emul-test.txt
if [ $? -eq 0 ]; then
   printf "%20s : ${GREEN}OK!${BASE}\n" $test
else
   printf "%20s : ${RED}Failed!!!${BASE}\n" $test
fi
