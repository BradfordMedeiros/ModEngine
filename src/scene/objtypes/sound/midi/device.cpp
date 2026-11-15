#include "./device.h"

/*

aseqdump -l
 Port    Client name                      Port name
  0:0    System                           Timer
  0:1    System                           Announce
 14:0    Midi Through                     Midi Through Port-0
 24:0    Keystation 61 MK3                Keystation 61 MK3(USB MIDI)
 24:1    Keystation 61 MK3                Keystation 61 MK3(Transport)

aseqdump -p 24:0
Waiting for data. Press Ctrl+C to end.
Source  Event                  Ch  Data
 24:0   Note on                14, note 52, velocity 118
 24:0   Note off               14, note 52, velocity 74
 24:0   Note on                14, note 52, velocity 102
 24:0   Note off               14, note 52, velocity 77


*/




std::vector<MidiDeviceInfo> listMidiDevices() {
	std::vector<MidiDeviceInfo> deviceInfo;
    snd_seq_t* seq = nullptr;

    int result = snd_seq_open(&seq, "default",  SND_SEQ_OPEN_INPUT, 0);

    if (result < 0) {
        printf("Failed to open ALSA sequencer: %s\n", snd_strerror(result));
        return {};
    }

    snd_seq_client_info_t* clientInfo;
    snd_seq_port_info_t* portInfo;

    snd_seq_client_info_alloca(&clientInfo);
    snd_seq_port_info_alloca(&portInfo);

    snd_seq_client_info_set_client(clientInfo, -1);

    while (snd_seq_query_next_client(seq, clientInfo) >= 0) {
        int client = snd_seq_client_info_get_client(clientInfo);
        const char* clientName = snd_seq_client_info_get_name(clientInfo);
        snd_seq_port_info_set_client(portInfo, client);
        snd_seq_port_info_set_port(portInfo, -1);
        while (snd_seq_query_next_port(seq, portInfo) >= 0) {
            unsigned int caps = snd_seq_port_info_get_capability(portInfo);
            if (!(caps & SND_SEQ_PORT_CAP_READ)){
                continue;
            }
            if (!(caps & SND_SEQ_PORT_CAP_SUBS_READ)){
                continue;
            }
            int port = snd_seq_port_info_get_port(portInfo);
            const char* portName = snd_seq_port_info_get_name(portInfo);
            //printf("Client: %d | Port: %d | %s | %s\n", client, port, clientName, portName);
            deviceInfo.push_back(MidiDeviceInfo {
            	.client = client,
            	.clientName = clientName,
            	.port = port,
            	.portName = portName,
            });
        }
    }

    snd_seq_close(seq);
    return deviceInfo;
}

std::string print(MidiDeviceInfo& deviceInfo){
	std::string value;
	value += "client = " + std::to_string(deviceInfo.client) + ", " ;
	value += "client name = " + deviceInfo.clientName  + ", " ;
	value += "port = " + std::to_string(deviceInfo.port)  + ", " ;
	value += "port name = " + deviceInfo.portName;
	return value;
}
std::string print(std::vector<MidiDeviceInfo> devices){
	std::string value = "[\n";
	for (auto& device : devices){
		value += "  " + print(device) + "\n";
	}
	value += "]";
	return value;
}






std::optional<MidiInput> initMidiInput(int client, int devicePort) {
	MidiInput midi{};
    int result = snd_seq_open(&midi.seq, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);

    if (result < 0) {
        printf("Failed to open MIDI sequencer: %s\n", snd_strerror(result));
        return std::nullopt;
    }

    snd_seq_set_client_name(midi.seq, "ModEngine");

    midi.port = snd_seq_create_simple_port(
        midi.seq,
        "MIDI Input",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION
    );

    if (midi.port < 0) {
        printf("Failed to create MIDI input port\n");
        snd_seq_close(midi.seq);
        midi.seq = nullptr;
        return std::nullopt;
    }

    result = snd_seq_connect_from(
        midi.seq, midi.port, client, devicePort
    );

    if (result < 0) {
        printf("Failed to connect MIDI device: %s\n", snd_strerror(result));
        snd_seq_close(midi.seq);
        midi.seq = nullptr;
        midi.port = -1;
        return std::nullopt;
    }

    return midi;
}

void readMidiInput(MidiInput& midi, std::vector<MidiEvent>& events) {
    snd_seq_event_t* event = nullptr;

    while (snd_seq_event_input(midi.seq, &event) >= 0) {
        MidiEvent midiEvent{};
        midiEvent.offset = 0;
        midiEvent.channel = 0;
        midiEvent.category = MIDI_CHANNEL_EVENT;

        switch (event->type) {
            case SND_SEQ_EVENT_NOTEON:
                midiEvent.channel = event->data.note.channel;

                if (event->data.note.velocity == 0) {
                    midiEvent.event = MidiNoteOff{
                        event->data.note.note,
                        0
                    };
                } else {
                    midiEvent.event = MidiNoteOn{
                        event->data.note.note,
                        event->data.note.velocity
                    };
                }
                break;

            case SND_SEQ_EVENT_NOTEOFF:
                midiEvent.channel = event->data.note.channel;
                midiEvent.event = MidiNoteOff{
                    event->data.note.note,
                    event->data.note.velocity
                };
                break;

            case SND_SEQ_EVENT_KEYPRESS:
                midiEvent.channel = event->data.note.channel;
                midiEvent.event = MidiPolyphonicAftertouch{
                    event->data.note.note,
                    event->data.note.velocity
                };
                break;

            case SND_SEQ_EVENT_CONTROLLER:
                midiEvent.channel = event->data.control.channel;
                midiEvent.event = MidiControlChange{
                    static_cast<uint8_t>(event->data.control.param),
                    static_cast<uint8_t>(event->data.control.value)
                };
                break;

            case SND_SEQ_EVENT_PGMCHANGE:
                midiEvent.channel = event->data.control.channel;
                midiEvent.event = MidiProgramChange{
                    static_cast<uint8_t>(event->data.control.value)
                };
                break;

            case SND_SEQ_EVENT_CHANPRESS:
                midiEvent.channel = event->data.control.channel;
                midiEvent.event = MidiChannelAftertouch{
                    static_cast<uint8_t>(event->data.control.value)
                };
                break;

            case SND_SEQ_EVENT_PITCHBEND:
                midiEvent.channel = event->data.control.channel;
                midiEvent.event = MidiPitchBend{
                    static_cast<uint16_t>(
                        std::clamp(
                            event->data.control.value + 8192,
                            0,
                            16383
                        )
                    )
                };
                break;

            default:
                midiEvent.category = MIDI_UNKNOWN_EVENT;
                midiEvent.event = UnknownMidiEvent{};
                break;
        }

        events.push_back(midiEvent);

        snd_seq_free_event(event);
        event = nullptr;
    }
}

void shutdownMidiInput(MidiInput& midi) {
    if (!midi.seq) {
    	return;
    }

    snd_seq_delete_simple_port(midi.seq, midi.port);
    snd_seq_close(midi.seq);

    midi.seq = nullptr;
    midi.port = -1;
}