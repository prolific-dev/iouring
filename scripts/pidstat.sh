#!/bin/bash

pids=($(ps aux | grep 'nat' | grep -v 'grep' | awk '{print $2}'))
#pids=($(ps aux | grep 'bufcopy' | grep -v 'grep' | awk '{print $2}'))

if [ "${#pids[@]}" -gt 0 ]; then
    echo "Number of PIDs: ${#pids[@]}"
else
    echo "No PIDs found"
    exit 1
fi

pidstat_flag=""

for pid in "${pids[@]}"; do
    pidstat_flag+="-p $pid "
done

pidstat_cmd="pidstat -u $pidstat_flag 1" 

echo "$pidstat_cmd"
${pidstat_cmd}

