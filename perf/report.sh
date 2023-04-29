#!/usr/bin/env bash

if [[ ! -f ./perf.data ]];
then
  echo "run perf.sh first"
  exit 1
fi
perf report
