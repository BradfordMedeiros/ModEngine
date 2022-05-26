#!/bin/sh

MODENGINE_BINARY={{MODENGINE_BINARY}}
(cd $MODENGINE_BINARY && ./build/modengine --fullscreen -r ./res/scenes/example.p.rawscene)
