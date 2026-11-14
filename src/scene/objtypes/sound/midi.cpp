#include "./midi.h"

#pragma pack(push, 1)
struct RawMidiHeader {
    char magic[4];       // MThd
    uint32_t length = 0;     // 6
    uint16_t format = 0;
    uint16_t trackCount = 0;
    uint16_t division = 0;
};
#pragma pack(pop)

std::string print(RawMidiHeader& midiHeader){
	std::string value;
	value += std::string("midiheader = [") + "\n";
	value += "  magic = " + std::string(midiHeader.magic, 4) + "\n";
	value += "  length = " + std::to_string(midiHeader.length) + "\n";
	value += "  format = " + std::to_string(midiHeader.format) + "\n";
	value += "  trackCount = " + std::to_string(midiHeader.trackCount) + "\n";
	value += "  division = " + std::to_string(midiHeader.division) + "\n";
	value += "]\n";
	return value;
}

#pragma pack(push, 1)
struct RawMidiTrack {
	char magic[4];  // Mtrk
	uint32_t length = 0;   
};
#pragma pack(pop)

std::string print(RawMidiTrack& midiTrack){
	std::string value;
	value += std::string("miditrack = [") + "\n";
	value += "  magic = " + std::string(midiTrack.magic, 4) + "\n";
	value += "  length = " + std::to_string(midiTrack.length) + "\n";
	value += "]\n";
	return value;
}


uint32_t readMidiVariableLength(FILE* file){
    uint32_t value = 0;
    while (true){
        uint8_t byte = 0;
        fread(&byte, 1, 1, file);

        value = (value << 7) | (byte & 0x7F);

        if ((byte & 0x80) == 0){
            break;
        }
    }
    return value;
}


struct MidiNoteOn{ uint8_t note; uint8_t velocity; };
struct MidiNoteOff{ uint8_t note; uint8_t velocity; };
struct MidiPolyphonicAftertouch {  uint8_t note; uint8_t pressure; };
struct MidiControlChange { uint8_t controller; uint8_t value; };
struct MidiProgramChange { uint8_t program; };
struct MidiChannelAftertouch { uint8_t pressure; };
struct MidiPitchBend { uint16_t pitchBend; };


struct UnknownMidiEvent{};
typedef std::variant<UnknownMidiEvent, MidiNoteOn, MidiNoteOff, MidiPolyphonicAftertouch, MidiControlChange, MidiProgramChange, MidiChannelAftertouch, MidiPitchBend> MidiEventType;

enum MidiEventCategory { MIDI_CHANNEL_EVENT, MIDI_META_EVENT, MIDI_SYS_EX_EVENT, MIDI_UNKNOWN_EVENT };

struct MidiEvent {
	uint32_t offset;
	MidiEventCategory category;
	MidiEventType event;
};
struct MidiTrack {
	std::vector<MidiEvent> events;
};
struct MidiFile {
	std::vector<MidiTrack> tracks;
};


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

void loadMidi(std::string filename){
	MidiFile midiFile{};

	RawMidiHeader midiHeader{};
	FILE* file = fopen(filename.c_str(), "rb");
	modassert(file, "file handle is null loadMidi");

	fread(&midiHeader, sizeof(midiHeader), 1, file);

	static_assert(sizeof(RawMidiHeader) == 14); // if this gets hit the compiler has a padding issue

	midiHeader.length = ntohl(midiHeader.length);
	midiHeader.format = ntohs(midiHeader.format);
	midiHeader.trackCount = ntohs(midiHeader.trackCount);
	midiHeader.division = ntohs(midiHeader.division);

	std::cout << print(midiHeader) << std::endl;

	modassert(midiHeader.magic[0] == 'M' && midiHeader.magic[1] == 'T' && midiHeader.magic[2] == 'h' && midiHeader.magic[3] == 'd', std::string("Unexpected midiHeader - probably not a midi file: ") + std::string(midiHeader.magic, 4));;
	modassert(midiHeader.length == 6, "midi expected length 6");  

	for (int i = 0; i < midiHeader.trackCount; i++){
		MidiTrack midiTrack{};
		RawMidiTrack rawMidiTrack{};

		//MidiTrack midiTrack{};

		static_assert(sizeof(RawMidiTrack) == 8); // if this gets hit the compiler has a padding issue
		fread(&rawMidiTrack, sizeof(rawMidiTrack), 1, file);

		rawMidiTrack.length = ntohl(rawMidiTrack.length);

		modassert(rawMidiTrack.magic[0] == 'M' && rawMidiTrack.magic[1] == 'T' && rawMidiTrack.magic[2] == 'r' && rawMidiTrack.magic[3] == 'k', std::string("Unexpected midiTrack - probably not a midi file: ") + std::string(rawMidiTrack.magic, 4));;

		//fseek(file, midiTrack.length, SEEK_CUR);

		long trackStart = ftell(file);
		long trackCurrent = trackStart;

		uint8_t runningStatus = 0;

		int numberOfEvents = 0;
		while (trackCurrent - trackStart < rawMidiTrack.length){
    		// Read the event

			numberOfEvents++;
    		auto deltaTime = readMidiVariableLength(file);

			midiTrack.events.push_back(MidiEvent {
				.offset = deltaTime,
				.category = MIDI_UNKNOWN_EVENT,
				.event = UnknownMidiEvent{},
			});
			auto& event = midiTrack.events.at(midiTrack.events.size() - 1);

    		uint8_t statusByte = 0;
    		modassert(fread(&statusByte, 1, 1, file) == 1,  "Failed to read MIDI status byte");

    		uint8_t firstDataByte = 0;
    		bool hasFirstDataByte = false;


    		if (statusByte >= 0x80 && statusByte <= 0xEF){
    		    runningStatus = statusByte;
    		}else if (statusByte < 0x80){
    		    modassert(runningStatus != 0, "Invalid MIDI running status");
		
    		    firstDataByte = statusByte;
    		    hasFirstDataByte = true;
    		    statusByte = runningStatus;
    		}

    		if (statusByte >= 0x80 && statusByte <= 0xEF){
    		    // Channel event
    		    event.category = MIDI_CHANNEL_EVENT;
    		    uint8_t eventType = statusByte & 0xF0;
    		    uint8_t channel = statusByte & 0x0F;
    		    if (eventType == 0x80){
    		        // Note Off
    		        uint8_t note = firstDataByte;
    		        uint8_t velocity;
    		        if (!hasFirstDataByte) fread(&note, 1, 1, file);
    		        fread(&velocity, 1, 1, file);
    		        event.event = MidiNoteOff{
    		        	.note = note,
    		        	.velocity = velocity,
    		        };
    		    }else if (eventType == 0x90){
    		        // Note On
    		        uint8_t note = firstDataByte;
    		        uint8_t velocity;
    		        if (!hasFirstDataByte) fread(&note, 1, 1, file);
    		        fread(&velocity, 1, 1, file);
    		        event.event = MidiNoteOn{
    		        	.note = note,
    		        	.velocity = velocity,
    		        };
    		    }else if (eventType == 0xA0){
    		        // Polyphonic Aftertouch
    		        uint8_t note = firstDataByte;
    		        uint8_t pressure;
    		        if (!hasFirstDataByte) fread(&note, 1, 1, file);
    		        fread(&pressure, 1, 1, file);
    		        event.event = MidiPolyphonicAftertouch {
    		        	.note = note,
    		        	.pressure = pressure,
    		        };
    		    }else if (eventType == 0xB0){
    		        // Control Change
    		        uint8_t controller = firstDataByte;
    		        uint8_t value;
    		        if (!hasFirstDataByte) fread(&controller, 1, 1, file);
    		        fread(&value, 1, 1, file);
    		        event.event = MidiControlChange {
    		        	.controller = controller,
    		        	.value = value,
    		        };
    		    }else if (eventType == 0xC0){
    		        // Program Change
    		        uint8_t program = firstDataByte;
    		        if (!hasFirstDataByte) fread(&program, 1, 1, file);
    		        event.event = MidiProgramChange {
    		        	.program = program,
    		        };
    		    }else if (eventType == 0xD0){
    		        // Channel Aftertouch
    		        uint8_t pressure = firstDataByte;
    		        if (!hasFirstDataByte) fread(&pressure, 1, 1, file);
    		        event.event = MidiChannelAftertouch {
    		        	.pressure = pressure,
    		        };
    		    }else if (eventType == 0xE0){
    		        // Pitch Bend
    		        uint8_t lsb = firstDataByte;
    		        uint8_t msb;
    		        if (!hasFirstDataByte) fread(&lsb, 1, 1, file);
    		        fread(&msb, 1, 1, file);
    		        uint16_t pitchBend = (msb << 7) | lsb;
    		        event.event = MidiPitchBend {
    		        	.pitchBend = pitchBend,
    		        };
    		    }else{
    		        modassert(false, "default case channel error");
    		    }
    		}else if (statusByte == 0xFF){
    		    // Meta event
    		    // just skips past it for now.  
     		    event.category = MIDI_META_EVENT;

    			// Meta event
    			runningStatus = 0;

    			uint8_t metaType = 0;
    			fread(&metaType, 1, 1, file);

    			uint32_t metaLength = readMidiVariableLength(file);

    			std::vector<uint8_t> metaData(metaLength);
    			fread(metaData.data(), 1, metaLength, file);

    		}else if (statusByte == 0xF0 || statusByte == 0xF7){
    		    // SysEx event
       		    event.category = MIDI_SYS_EX_EVENT;

    		    runningStatus = 0;

    			uint32_t sysexLength = readMidiVariableLength(file);

		    	fseek(file, sysexLength, SEEK_CUR);
    		}else{
    		    modassert(false, "Unknown MIDI event status");
    		}

    		trackCurrent = ftell(file);
    		modassert(trackCurrent - trackStart <= rawMidiTrack.length, "MIDI track read past declared length");
		}

		midiFile.tracks.push_back(midiTrack);
		std::cout << print(rawMidiTrack) << std::endl;
		std::cout << "number of events: " << numberOfEvents << std::endl;

	}

	std::cout << print(midiFile) << std::endl;

	modassert(false, "loadMidi not yet implemented");
}