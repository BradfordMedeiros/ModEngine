#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <AL/alut.h>
#include <AL/alext.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <queue>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vorbis/vorbisfile.h>
#include "../../../common/util.h"

void tickSynth();
void noteOn(uint8_t note, uint8_t velocity);
void noteOff(uint8_t note);



void midiKeyboardInput(int key, int action);