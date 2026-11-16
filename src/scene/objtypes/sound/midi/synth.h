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
#include "../../../../common/util.h"
#include "./device.h"

void tickSynth();
void midiKeyboardInput(int key, int action);

void midiAsInput(MidiFile& midiFile);
void playMidiFile(MidiFile& midiFile);
void tickMidiAsInput();