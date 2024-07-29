#!/bin/bash 

BASE=$(tput sgr0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)

test=$1

mkdir -p ../iob_soc_versat_V0.70_$test
# timeout amount depends on number of tests and pc specs, change it as needed
timeout 180 make sim-run TEST=$test &> ../iob_soc_versat_V0.70_$test/sim-run.txt

exit_status=$?
if [ $exit_status -eq 124 ]; then
   printf "%20s : ${YELLOW}Timeout!${BASE}\n" $test
elif [ $exit_status -eq 0 ]; then
   printf "%20s : ${GREEN}OK!${BASE}\n" $test
else
   printf "%20s : ${RED}Failed!!!${BASE}\n" $test
fi
