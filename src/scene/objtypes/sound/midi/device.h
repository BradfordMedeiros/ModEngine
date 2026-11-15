#pragma once

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "./types.h"

struct MidiDeviceInfo {
	int client;
	std::string clientName;
	int port;
	std::string portName;

};
std::vector<MidiDeviceInfo> listMidiDevices();
std::string print(MidiDeviceInfo& deviceInfo);
std::string print(std::vector<MidiDeviceInfo> devices);


struct MidiInput {
    snd_seq_t* seq = nullptr;
    int port = -1;
};
std::optional<MidiInput> initMidiInput(int client, int devicePort);
void readMidiInput(MidiInput& midi, std::vector<MidiEvent>& events);
void shutdownMidiInput(MidiInput& midi);