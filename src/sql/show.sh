#!/bin/sh

dotconfig=$(./plugin dotviz)
(cd ../../res/tools && ./showscene.sh "$dotconfig")
