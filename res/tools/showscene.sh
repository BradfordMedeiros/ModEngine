#!/bin/bash

# Example dot graph
#strict graph {
#  "root(-1)" -- "!onelight(0)"
#  "root(-1)" -- "!seclight(1)"
#  "root(-1)" -- ">camtest(2)"
#  "root(-1)" -- "]grid1(3)"
#  "root(-1)" -- "box1(4)"
#  "box1(4)" -- "box2(5)"
#  "root(-1)" -- "collisionobj(6)"
#  "root(-1)" -- "maincolumn(7)"
#}

if [[ -z "$1" ]]; then
  rm ./out.png
  rm ./testgraph.dot
else
  echo "$1" > testgraph.dot && dot ./testgraph.dot -Tpng -o out.png && rm ./testgraph.dot && xdg-open ./out.png 
fi 


