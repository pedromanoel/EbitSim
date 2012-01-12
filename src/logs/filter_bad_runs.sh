#!/bin/bash
# Extract info about bad runs
(for f in err_*.log; do echo -n "$f: "; tail $f | grep terminate; echo; done;) > bad_runs
grep 'err_\([0-9]\+\)\.log: terminate' bad_runs | sed 's/err_\([0-9]\+\)\.log: terminate.*/\1/' > bad_runs_filtered
# Must filter to show only the Run numbers with this
# :g!/err_\([0-9]\+\)\.log: terminate\|#/d
# :%s/err_\([0-9]\+\)\.log: terminate.*/\1/
# :'<, '>sort n
(for f in `cat bad_runs_filtered`; do echo -n "Run $f: "; cat <(grep -o "endSimulation\|SIGINT" out_$f.log); done;) > bad_runs_details

for r in `cat bad_runs_filtered`;
do
    echo -n "r$r "
done
echo

rm bad_runs bad_runs_filtered
