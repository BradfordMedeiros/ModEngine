#!/bin/sh

dotconfig=$(./plugin dotviz)
(cd ../../ModEngine/res/tools && ./showscene.sh "$dotconfig")
