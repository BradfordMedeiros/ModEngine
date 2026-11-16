#include "./types.h"

std::string print(MidiEventType& event){
	std::string value;
	{
		auto unknownMidiEvent = std::get_if<UnknownMidiEvent>(&event);
		if (unknownMidiEvent){
			return "[UNKNOWN EVENT]";
		}
	}
	{
		auto midiNoteOn = std::get_if<MidiNoteOn>(&event);
		if (midiNoteOn){
			std::string value = "[MIDI NOTE ON(";
			value += "note = " + std::to_string(midiNoteOn -> note) + " ";
			value += "velocity = " + std::to_string(midiNoteOn -> velocity);
			value += ")]";
			return value;
		}
	}

	{
		auto midiNoteOff = std::get_if<MidiNoteOff>(&event);
		if (midiNoteOff){
			std::string value = "[MIDI NOTE OFF(";
			value += "note = " + std::to_string(midiNoteOff -> note) + " ";
			value += "velocity = " + std::to_string(midiNoteOff -> velocity);
			value += ")]";
			return value;
		}
	}

	{
		auto midiPolyphonicAftertouch = std::get_if<MidiPolyphonicAftertouch>(&event);
		if (midiPolyphonicAftertouch){
			std::string value = "[MIDI Aftertouch(";
			value += "note = " + std::to_string(midiPolyphonicAftertouch -> note) + " ";
			value += "pressure = " + std::to_string(midiPolyphonicAftertouch -> pressure);
			value += ")]";
			return value;		
		}
	}
	{
		auto midiControlChange = std::get_if<MidiControlChange>(&event);
		if (midiControlChange){
			std::string value = "[MIDI ControlChange(";
			value += "controller = " + std::to_string(midiControlChange -> controller) + " ";
			value += "value = " + std::to_string(midiControlChange -> value);
			value += ")]";
			return value;		
		}	
	}

	{
		auto midiProgramChange = std::get_if<MidiProgramChange>(&event);
		if (midiProgramChange){
			std::string value = "[MIDI MidiProgramChange(";
			value += "program = " + std::to_string(midiProgramChange -> program);
			value += ")]";
			return value;		
		}	
	}

	{
		auto midiChannelAftertouch = std::get_if<MidiChannelAftertouch>(&event);
		if (midiChannelAftertouch){
			std::string value = "[MIDI MidiChannelAftertouch(";
			value += "pressure = " + std::to_string(midiChannelAftertouch -> pressure);
			value += ")]";
			return value;		
		}	
	}

	{
		auto midiPitchBend = std::get_if<MidiPitchBend>(&event);
		if (midiPitchBend){
			std::string value = "[MIDI MidiChannelAftertouch(";
			value += "pitchBend = " + std::to_string(midiPitchBend -> pitchBend);
			value += ")]";
			return value;		
		}	
		
	}
	

	return "[PRINT EVENT TYPE ERROR]";
}
std::string print(MidiEventCategory category){
	if (category == MIDI_CHANNEL_EVENT){
		return "[MIDI_CHANNEL_EVENT]";
	}else if (category == MIDI_META_EVENT){
		return "[MIDI_META_EVENT]";
	}else if (category == MIDI_SYS_EX_EVENT){
		return "[MIDI_SYS_EX_EVENT]";
	}else if (category == MIDI_UNKNOWN_EVENT){
		return "[UNKNOWN ERROR]";
	}
	return "[PRINT CATEGORY ERROR]";
}
std::string print(MidiTrack& track){
	std::string value;
	value += "    events.size = " + std::to_string(track.events.size()) + "\n";
	
	value += std::string("    events = [") + "\n";
	for (auto& event : track.events){
		std::string eventValue;
		value += "      category = " +  print(event.category) + "\n";
		value += "        " + print(event.event);
		value += "\n";
	}
	value += "    ]\n";
	
	return value;
}
std::string print(MidiFile& midiFile){
	std::string value;
	value += "midifile: [\n";
	value += "  tracks = " + std::to_string(midiFile.tracks.size()) + "\n";
	for (auto& track : midiFile.tracks){
		value += print(track);
	}
	value += "\n]";
	return value;
}

double convertEventToTimeSeconds(MidiFile& midiFile, MidiEvent& midiEvent) {
    uint32_t tempo = 500000; // 120 BPM
    return static_cast<double>(midiEvent.absoluteOffset) * tempo /  (static_cast<double>(midiFile.division) * 1000000.0);
}


std::vector<std::string> midiNoteNames = {
    "C-1", "C#-1", "D-1", "D#-1", "E-1", "F-1", "F#-1", "G-1", "G#-1", "A-1", "A#-1", "B-1",
    "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
    "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
    "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
    "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
    "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
    "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
    "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
    "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
    "C8", "C#8", "D8", "D#8", "E8", "F8", "F#8", "G8", "G#8", "A8", "A#8", "B8",
    "C9", "C#9", "D9", "D#9", "E9", "F9", "F#9", "G9"
};

std::optional<std::string> noteNameFromInt(int note){
	if (note < 0 || note >= midiNoteNames.size()){
		return std::nullopt;
	}
	return midiNoteNames.at(note);
}