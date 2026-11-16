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



double timeSeconds(bool realtime);

std::optional<MidiFile> playingMidiFile;
std::optional<double> midiStartTime;
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

    auto currentTime = timeSeconds(true) - midiStartTime.value();
    for (int i = 0; i < midiFile.tracks.size(); i++){
        int nextIdToPlay = nextIdToPlayPerTrack.at(i);
        for (int j = nextIdToPlay; j < midiFile.tracks.at(i).events.size(); j++){
            auto& event = midiFile.tracks.at(i).events.at(j);

            auto eventTime = convertEventToTimeSeconds(midiFile, event);
            //std::cout << "midi playback: play event : offset = " <<  event.offset << ",  index = " << j << ", time: " << eventTime << ", time = " << currentTime << std::endl;
            if (currentTime >= eventTime){
                std::cout << "midi playback: play event: track: " << i << ", event index = " << j << std::endl;
                nextIdToPlayPerTrack.at(i) = j + 1;

                MidiNoteOn* midiNoteOn = std::get_if<MidiNoteOn>(&event.event);
                MidiNoteOff* midiNoteOff = std::get_if<MidiNoteOff>(&event.event);

                if (midiNoteOn){
                    noteOn(midiNoteOn -> note, midiNoteOn -> velocity);
                }
                if (midiNoteOff){
                    noteOff(midiNoteOff -> note);
                }

            }else{
                break;
            }
        }
    }   
}