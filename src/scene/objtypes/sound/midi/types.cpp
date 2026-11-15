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
