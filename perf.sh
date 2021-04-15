#!/usr/bin/env bash

# https://github.com/brendangregg/FlameGraph

if [[ -z "$1" ]]
then
  echo "specify scene file to run with"
  exit 1
fi

echo "$@"
perf record -F 99 -ag -- ./build/modengine "$@" 
perf report --stdio
