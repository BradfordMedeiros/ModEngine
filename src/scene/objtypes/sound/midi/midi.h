#pragma once

#include <arpa/inet.h>
#include <string>
#include <vector>
#include <iostream>
#include "./types.h"

std::optional<MidiFile> loadMidi(std::string filename);
