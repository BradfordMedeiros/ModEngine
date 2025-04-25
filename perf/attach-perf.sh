#!/usr/bin/env bash

# https://github.com/brendangregg/FlameGraph

process_id=$(pgrep modengine)
if [[ -z "$process_id" ]]
then
	echo "no process for modengine found"
	exit 1
fi

(cd ../ && perf record -g --output=./perf/perf.data -i -p $process_id)
