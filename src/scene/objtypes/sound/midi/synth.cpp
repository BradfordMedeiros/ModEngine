#include "./synth.h"

struct SynthVoice {
    bool active = false;
    uint8_t note = 0;
    float frequency = 0.0f;
    float phase = 0.0f;
    float volume = 0.0f;
};

struct Synthesizer {
    SynthVoice voices[32];
    ALuint source;
    ALuint buffers[3];
    static constexpr int sampleRate = 48000;
    static constexpr int bufferSize = 256 * 4;
    int16_t audioBuffer[bufferSize];
};

void noteOn(Synthesizer& synth, uint8_t note, uint8_t velocity) {
    SynthVoice* voice = nullptr;
    for (auto& v : synth.voices) {
        if (!v.active) {
            voice = &v;
            break;
        }
    }
    if (!voice) {
        return;
    }
    voice->active = true;
    voice->note = note;
    voice->frequency = 440.0f * powf(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    voice->phase = 0.0f;
    voice->volume = (velocity / 127.0f) * 0.2f;
}

void noteOff(Synthesizer& synth, uint8_t note) {
    for (auto& voice : synth.voices) {
        if (voice.active && voice.note == note) {
            voice.active = false;
        }
    }
}

void generateAudio(Synthesizer& synth) {
    for (int i = 0; i < synth.bufferSize; i++) {
        float output = 0.0f;
        for (auto& voice : synth.voices) {
            if (!voice.active) {
                continue;
            }
            output += sinf(voice.phase) * voice.volume;
            voice.phase += 2.0f * M_PI * voice.frequency / synth.sampleRate;
            if (voice.phase >= 2.0f * M_PI){
                voice.phase -= 2.0f * M_PI;
            }
        }
        output = std::clamp(output, -1.0f, 1.0f);
        synth.audioBuffer[i] = static_cast<int16_t>(output * 32767.0f);
    }
}

void initSynthesizer(Synthesizer& synth) {
    alGenSources(1, &synth.source);
    alGenBuffers(3, synth.buffers);
    for (int i = 0; i < 3; i++) {
        memset(synth.audioBuffer, 0, sizeof(synth.audioBuffer));
        alBufferData(synth.buffers[i], AL_FORMAT_MONO16, synth.audioBuffer, sizeof(synth.audioBuffer), synth.sampleRate);
        alSourceQueueBuffers(synth.source, 1, &synth.buffers[i]);
    }
    alSourcePlay(synth.source);
}

void updateSynthesizer(Synthesizer& synth) {
    ALint processed = 0;
    alGetSourcei(synth.source, AL_BUFFERS_PROCESSED, &processed);
    while (processed--) {
        ALuint buffer;
        alSourceUnqueueBuffers(synth.source, 1, &buffer);
        generateAudio(synth);
        alBufferData(buffer, AL_FORMAT_MONO16, synth.audioBuffer, sizeof(synth.audioBuffer), synth.sampleRate);
        alSourceQueueBuffers(synth.source, 1, &buffer);
    }
    ALint state;
    alGetSourcei(synth.source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING){
        alSourcePlay(synth.source);
    }
}

std::optional<MidiInput> midiInput;

static Synthesizer synth{};
static bool hasInitSynth = false;

void noteOn(uint8_t note, uint8_t velocity){
    modassert(hasInitSynth, "synth not init");
    noteOn(synth, note, velocity);
}

void noteOff(uint8_t note){
    modassert(hasInitSynth, "synth not init");
    noteOff(synth, note);
}

void tickSynth(){
	if (!hasInitSynth){
		initSynthesizer(synth);
		hasInitSynth = true;
	}
	updateSynthesizer(synth);

    if (midiInput.has_value()){
       std::vector<MidiEvent> midiEvents;
       readMidiInput(midiInput.value(), midiEvents);


       if (midiEvents.size() > 0){
        std::cout << "midi size: " << midiEvents.size() << std::endl;
        for(auto& event : midiEvents){
          std::cout << "midi: " << print(event.event) << std::endl;


          MidiNoteOn* midiNoteOn = std::get_if<MidiNoteOn>(&event.event);
          MidiNoteOff* midiNoteOff = std::get_if<MidiNoteOff>(&event.event);
          if (midiNoteOn){
            noteOn(midiNoteOn -> note, midiNoteOn -> velocity);
          }
          if (midiNoteOff){
            noteOff(midiNoteOff -> note);
          }
            //struct MidiNoteOn{ uint8_t note; uint8_t velocity; };
            //struct MidiNoteOff{ uint8_t note; uint8_t velocity; };
    
        }        
       }

    }
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

std::unordered_map<int, int> keyboardToMidi = {
    {'A', 48}, // C3
    {'W', 49}, // C#3
    {'S', 50}, // D3
    {'E', 51}, // D#3
    {'D', 52}, // E3
    {'F', 53}, // F3
    {'T', 54}, // F#3
    {'G', 55}, // G3
    {'Y', 56}, // G#3
    {'H', 57}, // A3
    {'U', 58}, // A#3
    {'J', 59}, // B3
    {'K', 60}, // C4
    {'O', 61}, // C#4
    {'L', 62}, // D4
    {'P', 63}, // D#4
    {';', 64}, // E4
    {'\'', 65} // F4
};



void midiKeyboardInput(int key, int action){
  static int currentDevice = 0;
  if (key == 'Y' && action == 1){
    auto devices = listMidiDevices();
    std::cout << "midi: " << currentDevice << " | " << print(devices.at(currentDevice)) << std::endl;
  }
  if (key == 'U' && action == 1){
    auto devices = listMidiDevices();
    currentDevice--;
    if (currentDevice < 0){
        currentDevice = 0;
    }
    std::cout << "midi: " << currentDevice << " | " << print(devices.at(currentDevice)) << std::endl;
  }
  if (key == 'I' && action == 1){
    auto devices = listMidiDevices();
    currentDevice++;
    if (currentDevice >= devices.size()){
        currentDevice = devices.size() - 1;
    }
    std::cout << "midi: " << currentDevice << " | " << print(devices.at(currentDevice)) << std::endl;
  }

  if (key == 'O' && action == 1){
    if (midiInput.has_value()){
        shutdownMidiInput(midiInput.value());
        midiInput = std::nullopt;
        std::cout << "midi: device shutdown" << std::endl;

    }else{  
        auto devices = listMidiDevices();
        auto device = devices.at(currentDevice);
        auto midiInitValue = initMidiInput(device.client, device.port);
        modassert(midiInitValue.has_value(), "could not connect to midi device");
        midiInput = midiInitValue.value();
        std::cout << "midi: device connected" << std::endl;

    }


  }

  //static int note = 60;
  //if (keyboardToMidi.find(key) != keyboardToMidi.end()){
  //  auto midiNote = keyboardToMidi.at(key);
  //  if (action == 1){
  //      noteOn(midiNote, 20);
  //  }else if (action == 0){
  //      noteOff(midiNote);
  //  }
  //}
}


double convertEventToTimeSeconds(MidiFile& midiFile, MidiEvent& midiEvent) {
    uint32_t tempo = 500000; // 120 BPM
    return static_cast<double>(midiEvent.offset) * tempo /  (static_cast<double>(midiFile.division) * 1000000.0);
}


double timeSeconds(bool realtime);

std::optional<MidiFile> playingMidiFile;
std::optional<float> midiStartTime;
std::vector<int> nextIdToPlayPerTrack {};

void playMidiFile(MidiFile& midiFile){
    std::cout << "midi playback: start" << std::endl;
    if (!playingMidiFile.has_value()){
        playingMidiFile = midiFile;
        midiStartTime = timeSeconds(true);

        nextIdToPlayPerTrack = {};
        for (auto& track : midiFile.tracks){
            nextIdToPlayPerTrack.push_back(0);
        }
    }else{
        playingMidiFile = std::nullopt;
        midiStartTime = std::nullopt;
        nextIdToPlayPerTrack = {};
    }
}

void tickMidiAsInput(){
    if (!playingMidiFile.has_value()){
        return;
    }
    auto& midiFile = playingMidiFile.value();

    auto currentTime = timeSeconds(true);
    for (int i = 0; i < midiFile.tracks.size(); i++){
        int nextIdToPlay = nextIdToPlayPerTrack.at(i);
        for (int j = nextIdToPlay; j < midiFile.tracks.at(i).events.size(); j++){
            auto& event = midiFile.tracks.at(i).events.at(j);
            auto eventTime = convertEventToTimeSeconds(midiFile, event);
            std::cout << "midi playback: play event : offset = " <<  event.offset << ",  index = " << j << ", time: " << eventTime << ", time = " << currentTime << std::endl;
            if (currentTime > eventTime){
                std::cout << "midi playback: play event: track: " << i << ", event index = " << j << std::endl;
                nextIdToPlayPerTrack.at(i) = j + 1;
            }else{
                break;
            }
        }
    }   
}