#!/bin/bash

EXEC_FILE="./BitTorrentSingleProcessor" # path to the simulation executable
INI_FILE="../simulations/MultiplePeers.ini" # path to the configuration file
CONFIG="MultiplePeers" # name of the configuration
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
    echo -e "r$i:\n\t$EXEC_FILE -f $INI_FILE -c $CONFIG -u Cmdenv -r $i > $LOG_DIR/out_$i.log 2> $LOG_DIR/err_$i.log"
done
