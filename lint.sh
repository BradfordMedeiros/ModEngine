#!/usr/bin/env bash

# sudo apt-get install cppcheck

# probably can configure this better but this is what I'm currently using
# it doesn't seem to actually be aware of usage so it's definitely not hooked up write but w/e for now.

cppcheck --language=c++ --enable=all ./src/

