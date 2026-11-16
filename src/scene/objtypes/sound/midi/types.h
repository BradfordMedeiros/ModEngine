#pragma once

#include "../../../../common/util.h"

// Channel Events 
struct MidiNoteOn{ uint8_t note; uint8_t velocity; };
struct MidiNoteOff{ uint8_t note; uint8_t velocity; };
struct MidiPolyphonicAftertouch {  uint8_t note; uint8_t pressure; };
struct MidiControlChange { uint8_t controller; uint8_t value; };
struct MidiProgramChange { uint8_t program; };
struct MidiChannelAftertouch { uint8_t pressure; };
struct MidiPitchBend { uint16_t pitchBend; };

// Meta Events
struct MidiSetTempo {
    uint32_t microsecondsPerQuarter;
};

// SysEx Events 
// (none)

struct UnknownMidiEvent{};
typedef std::variant<
	UnknownMidiEvent, 
	MidiNoteOn, 
	MidiNoteOff, 
	MidiPolyphonicAftertouch, 
	MidiControlChange, 
	MidiProgramChange, 
	MidiChannelAftertouch, 
	MidiPitchBend,
	MidiSetTempo
> MidiEventType;

enum MidiEventCategory { MIDI_CHANNEL_EVENT, MIDI_META_EVENT, MIDI_SYS_EX_EVENT, MIDI_UNKNOWN_EVENT };

struct MidiEvent {
	int midiEventId = 0; // Just for internal uniqueness
	uint32_t offset = 0;  // microseconds per quater note  .  Tempo is microsends to play a quarter note
	uint8_t channel = -1;
	MidiEventCategory category;
	MidiEventType event;
};
struct MidiTrack {
	std::vector<MidiEvent> events;
};
struct MidiFile {
	uint16_t division;
	std::vector<MidiTrack> tracks;
};


std::string print(MidiEventType& event);
std::string print(MidiEventCategory category);
std::string print(MidiTrack& track);
std::string print(MidiFile& midiFile);

