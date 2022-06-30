#!/bin/bash

if [[ -z "$1" ]];
then
  echo "specify path to shader as arg"
  exit 1
fi
cgc -oglsl -strict -glslWerror -profile gp5vp $1
