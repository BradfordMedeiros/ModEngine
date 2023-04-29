#!/usr/bin/env bash

# https://github.com/brendangregg/FlameGraph

if [[ -z "$1" ]]
then
  echo "specify scene file to run with"
  exit 1
fi

echo "$@"
(cd ../ && perf record -F 99 -ag --output=./perf/perf.data -- ./build/modengine "$@")
