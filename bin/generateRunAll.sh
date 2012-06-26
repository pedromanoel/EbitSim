#!/bin/bash

EXEC_FILE="../src/EbitSim" # path to the simulation executable
INI_FILE="../simulations/BasicConfig.ini" # path to the configuration file
CONFIG="BasicTopology" # name of the configuration
LOG_DIR="./logs"

if [[ $# != 1 ]]; then
    echo "Usage: $0 <count>"
    exit 1
fi

declare -i num_runs=$1

a=""
for (( i=0; i < $num_runs; i++ ))
do
    a="$a r$i"
done

echo ".PHONY:$a"
echo
echo ".PHONY: all"
echo
echo "all: $a"
echo

for (( i=0; i < $num_runs; i++ ))
do
    echo -e "r$i:\n\t./Cmdenv $i > $LOG_DIR/out_$i.log 2> $LOG_DIR/err_$i.log"
done
