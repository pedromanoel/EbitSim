#!/bin/bash
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <runs> [CONFIG]"
    exit 1
fi

EXEC_FILE="../src/EbitSim" # path to the simulation executable
INI_FILE="../simulations/BasicConfig.ini" # path to the configuration file
if [[ $# == 2 ]]; then
    CONFIG=$2
else
    CONFIG="SimpleTopology" # name of the configuration
fi
LOG_DIR="./logs"

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
    echo -e "r$i:\n\t./Cmdenv $i $CONFIG"
done
