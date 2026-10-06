#include "./sound.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <limits>

std::string readFileOrPackage(std::string filepath); // i dont really like directly referencing this here, but...it's ok
unsigned int openFileOrPackage(std::string filepath);
int closeFileOrPackage(unsigned int handle);
size_t readFileOrPackage(unsigned int handle, void *ptr, size_t size, size_t nmemb);
int seekFileOrPackage(unsigned int handle, int offset, int whence);
size_t tellFileOrPackage(unsigned int handle);
static std::unordered_map<std::string, ALuint> soundBuffers;  
static std::unordered_map<std::string, int> soundUsages;

struct OneShotData {
  ALuint bufferId;
  bool center;
  objid ownerId;
  bool shouldErase = false;
};

static std::unordered_map<ALuint, OneShotData> soundOneshotsSourceToBuffer;

//// global sound stuff /////
void startSoundSystem(){
  alutInit(NULL, NULL);
}
void stopSoundSystem(){
  alutExit();
}

float getVolume(){
  ALfloat oldVolume;
  alGetListenerf(AL_GAIN, &oldVolume);
  return oldVolume;
}

void setVolume(float volume){
  modassert(volume >= 0 && volume <=1, "set listener volume invalid volume");
  alListenerf(AL_GAIN, volume);
}

void setListenerPosition(float x, float y, float z, std::vector<float> forward, std::vector<float> up){
  assert(forward.size() == 3);
  assert(up.size() == 3);
  alListener3f(AL_POSITION, x, y, z); 

  float orientation[6];
  orientation[0] = forward.at(0);
  orientation[1] = forward.at(1);
  orientation[2] = forward.at(2);
  orientation[3] = up.at(0);
  orientation[4] = up.at(1);
  orientation[5] = up.at(2);
  alListenerfv(AL_ORIENTATION, orientation);
}

///////////////////////
void playSource(ALuint source){
  alSourcePlay(source);
}

void stopSource(ALuint source){
  alSourceStop(source);
}


void setSoundPosition(ALuint source, float x, float y, float z){
  alSource3f(source, AL_POSITION, x, y, z);
}
void setSoundVolume(ALuint source, float newVolume){
  alSourcef(source, AL_GAIN, newVolume);
}
void setSoundLooping(ALuint source, bool shouldLoop){
  alSourcei(source, AL_LOOPING, shouldLoop ? AL_TRUE : AL_FALSE);
}
void setSoundPitch(ALuint source, float pitchMultiplier){
  modassert(pitchMultiplier >= 0, "pitch multiplier negative");
  alSourcef(source, AL_PITCH, pitchMultiplier);
}


size_t my_read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {
  unsigned int* handle = static_cast<unsigned int*>(datasource);
  modassert(handle, "handle is null");
  modlog("vorbis", "read");
  return readFileOrPackage(*handle, ptr, size, nmemb);
}

int my_seek_func(void *datasource, ogg_int64_t offset, int whence) {
  unsigned int* handle = static_cast<unsigned int*>(datasource);
  modassert(handle, "handle is null");
  modlog("vorbis", "seek");
  return seekFileOrPackage(*handle, offset, whence);
}

int my_close_func(void *datasource) {
  unsigned int* handle = static_cast<unsigned int*>(datasource);
  modassert(handle, "handle is null");
  modlog("vorbis", "close");
  return closeFileOrPackage(*handle);
}

long my_tell_func(void *datasource) {
  unsigned int* handle = static_cast<unsigned int*>(datasource);
  modassert(handle, "handle is null");
  modlog("vorbis", "tell");
  return tellFileOrPackage(*handle);
}

ov_callbacks vorbisFileFns = {
    .read_func = my_read_func,
    .seek_func = my_seek_func,
    .close_func = my_close_func,
    .tell_func = my_tell_func
};

struct DecodedVorbisAudio {
  std::vector<char> pcmData;
  int channels = 0;
  int sampleRate = 0;
};

bool decodeVorbisAudio(std::string filepath, DecodedVorbisAudio& audio, std::string& error){
  unsigned int fileHandle = openFileOrPackage(filepath.c_str());

  OggVorbis_File vf;
  auto result = ov_open_callbacks(&fileHandle, &vf, NULL, 0, vorbisFileFns); 
  if (result < 0) {
    error = std::string("Error opening Ogg/Vorbis audio: ") + std::to_string(result);
    return false;
  }

  vorbis_info* vi = ov_info(&vf, -1);
  if (vi == NULL || vi->channels <= 0 || vi->rate <= 0){
    ov_clear(&vf);
    error = "Ogg/Vorbis audio has invalid stream metadata.";
    return false;
  }

  audio.channels = vi->channels;
  audio.sampleRate = vi->rate;
  int bitstream = 0;
  char chunk[8192];
  while (true){
    const long bytesRead = ov_read(&vf, chunk, sizeof(chunk), 0, 2, 1, &bitstream);
    if (bytesRead == 0){
      break;
    }
    if (bytesRead < 0){
      ov_clear(&vf);
      error = "An error occurred while decoding Ogg/Vorbis audio.";
      return false;
    }
    audio.pcmData.insert(audio.pcmData.end(), chunk, chunk + bytesRead);
  }
  ov_clear(&vf);

  const size_t bytesPerFrame = static_cast<size_t>(audio.channels) * sizeof(int16_t);
  if (audio.pcmData.empty() || audio.pcmData.size() % bytesPerFrame != 0){
    error = "Ogg/Vorbis audio contains no complete sample frames.";
    return false;
  }
  return true;
}

void readVorbisFile(std::string& filepath, ALuint* _soundBuffer){
  modlog("vorbis", filepath);
  modlog("vorbis", "open");
  DecodedVorbisAudio audio;
  std::string error;
  if (!decodeVorbisAudio(filepath, audio, error)){
    modassert(false, error);
    throw std::runtime_error(error);
  }

  auto format = audio.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
  alGenBuffers(1, _soundBuffer);
  alBufferData(*_soundBuffer, format, audio.pcmData.data(), audio.pcmData.size(), audio.sampleRate);
}

uint16_t readWaveUint16(const std::string& data, size_t offset){
  return static_cast<uint16_t>(static_cast<uint8_t>(data[offset])) |
    (static_cast<uint16_t>(static_cast<uint8_t>(data[offset + 1])) << 8);
}

uint32_t readWaveUint32(const std::string& data, size_t offset){
  return static_cast<uint32_t>(static_cast<uint8_t>(data[offset])) |
    (static_cast<uint32_t>(static_cast<uint8_t>(data[offset + 1])) << 8) |
    (static_cast<uint32_t>(static_cast<uint8_t>(data[offset + 2])) << 16) |
    (static_cast<uint32_t>(static_cast<uint8_t>(data[offset + 3])) << 24);
}

bool decodeWaveAudio(const std::string& data, SoundAnalysisAudio& audio, std::string& error){
  if (data.size() < 12 || data.compare(0, 4, "RIFF") != 0 || data.compare(8, 4, "WAVE") != 0){
    error = "Expected a RIFF/WAVE audio file.";
    return false;
  }

  uint16_t format = 0;
  uint16_t channels = 0;
  uint16_t bitsPerSample = 0;
  uint16_t blockAlign = 0;
  uint32_t sampleRate = 0;
  size_t audioOffset = 0;
  size_t audioSize = 0;
  for (size_t offset = 12; offset + 8 <= data.size();){
    const uint32_t chunkSize = readWaveUint32(data, offset + 4);
    const size_t chunkOffset = offset + 8;
    if (chunkSize > data.size() - chunkOffset){
      error = "The WAV file contains a truncated chunk.";
      return false;
    }
    if (data.compare(offset, 4, "fmt ") == 0){
      if (chunkSize < 16){
        error = "The WAV format chunk is incomplete.";
        return false;
      }
      format = readWaveUint16(data, chunkOffset);
      channels = readWaveUint16(data, chunkOffset + 2);
      sampleRate = readWaveUint32(data, chunkOffset + 4);
      blockAlign = readWaveUint16(data, chunkOffset + 12);
      bitsPerSample = readWaveUint16(data, chunkOffset + 14);
    } else if (data.compare(offset, 4, "data") == 0){
      audioOffset = chunkOffset;
      audioSize = chunkSize;
    }

    const size_t paddedChunkSize = static_cast<size_t>(chunkSize) + (chunkSize & 1);
    if (paddedChunkSize > data.size() - chunkOffset){
      break;
    }
    offset = chunkOffset + paddedChunkSize;
  }

  const size_t bytesPerSample = (bitsPerSample + 7) / 8;
  if ((format != 1 && format != 3) || channels == 0 || sampleRate == 0 ||
      sampleRate > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
      audioSize == 0 || bytesPerSample == 0 || blockAlign < channels * bytesPerSample ||
      (format == 1 && bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32) ||
      (format == 3 && bitsPerSample != 32)){
    error = "WAV analysis supports PCM (8/16/24/32-bit) and 32-bit float audio.";
    return false;
  }

  const size_t frameCount = audioSize / blockAlign;
  if (frameCount == 0){
    error = "The WAV file contains no audio samples.";
    return false;
  }
  audio.monoSamples.reserve(frameCount);
  for (size_t frame = 0; frame < frameCount; frame++){
    float monoSample = 0.f;
    const size_t frameOffset = audioOffset + frame * blockAlign;
    for (size_t channel = 0; channel < channels; channel++){
      const size_t sampleOffset = frameOffset + channel * bytesPerSample;
      float sample = 0.f;
      if (format == 3){
        std::memcpy(&sample, data.data() + sampleOffset, sizeof(sample));
        if (!std::isfinite(sample)){
          sample = 0.f;
        }
      } else if (bitsPerSample == 8){
        sample = (static_cast<int>(static_cast<uint8_t>(data[sampleOffset])) - 128) / 128.f;
      } else if (bitsPerSample == 16){
        sample = static_cast<int16_t>(readWaveUint16(data, sampleOffset)) / 32768.f;
      } else if (bitsPerSample == 24){
        int32_t value =
          static_cast<uint8_t>(data[sampleOffset]) |
          (static_cast<int32_t>(static_cast<uint8_t>(data[sampleOffset + 1])) << 8) |
          (static_cast<int32_t>(static_cast<uint8_t>(data[sampleOffset + 2])) << 16);
        if ((value & 0x800000) != 0){
          value |= static_cast<int32_t>(0xff000000);
        }
        sample = value / 8388608.f;
      } else {
        const int32_t value = static_cast<int32_t>(readWaveUint32(data, sampleOffset));
        sample = value / 2147483648.f;
      }
      monoSample += std::clamp(sample, -1.f, 1.f);
    }
    audio.monoSamples.push_back(monoSample / channels);
  }
  audio.sampleRate = static_cast<int>(sampleRate);
  return true;
}

bool decodeSoundFileForAnalysis(std::string filepath, SoundAnalysisAudio& audio, std::string& error){
  audio = {};
  error.clear();
  auto extension = getExtension(filepath);
  if (!extension.has_value()){
    error = "The selected sound file has no extension.";
    return false;
  }
  std::transform(extension->begin(), extension->end(), extension->begin(), [](unsigned char value){
    return static_cast<char>(std::tolower(value));
  });

  if (extension.value() == "ogg"){
    DecodedVorbisAudio decoded;
    if (!decodeVorbisAudio(filepath, decoded, error)){
      return false;
    }
    const size_t bytesPerFrame = static_cast<size_t>(decoded.channels) * sizeof(int16_t);
    const size_t frameCount = decoded.pcmData.size() / bytesPerFrame;
    audio.monoSamples.reserve(frameCount);
    for (size_t frame = 0; frame < frameCount; frame++){
      float monoSample = 0.f;
      for (int channel = 0; channel < decoded.channels; channel++){
        const size_t offset = frame * bytesPerFrame + static_cast<size_t>(channel) * sizeof(int16_t);
        const uint16_t sampleBits = static_cast<uint8_t>(decoded.pcmData[offset]) |
          (static_cast<uint16_t>(static_cast<uint8_t>(decoded.pcmData[offset + 1])) << 8);
        const auto sample = static_cast<int16_t>(sampleBits);
        monoSample += sample / 32768.f;
      }
      audio.monoSamples.push_back(monoSample / decoded.channels);
    }
    audio.sampleRate = decoded.sampleRate;
    return true;
  }

  if (extension.value() == "wav"){
    const std::string data = readFileOrPackage(filepath);
    if (data.empty()){
      error = "The selected WAV file could not be read.";
      return false;
    }
    return decodeWaveAudio(data, audio, error);
  }

  error = "Audio analysis supports WAV and Ogg/Vorbis files.";
  return false;
}

ALuint findOrLoadBuffer(std::string filepath){
  if (soundBuffers.find(filepath) != soundBuffers.end()){
    return soundBuffers.at(filepath);
  }

  ALuint soundBuffer = 0;
  auto extension = getExtension(filepath);
  if (extension.value() == "ogg"){
    readVorbisFile(filepath, &soundBuffer);
  }else{
    auto soundFileData = readFileOrPackage(filepath);
    soundBuffer = alutCreateBufferFromFileImage(soundFileData.c_str(), soundFileData.size());    
  }

  ALenum error = alutGetError();
  if (error != ALUT_ERROR_NO_ERROR){
    std::cerr << "ERROR: ALUT Error: " << alutGetErrorString(error) <<  ": " << std::strerror(errno) << std::endl;
    throw std::runtime_error("error loading buffer");
  }
  soundBuffers[filepath] = soundBuffer;
  return soundBuffer;  
}

ALuint createSource(ALuint soundBuffer){
  ALuint soundSource;
  alGenSources(1, &soundSource);
  alSourcei(soundSource, AL_BUFFER, soundBuffer);  
  return soundSource;
}

ALuint loadSoundState(std::string filepath){
  std::cout << "EVENT: loading sound:" << filepath <<  std::endl; 

  ALuint soundBuffer = findOrLoadBuffer(filepath);
  ALuint soundSource = createSource(soundBuffer);
  
  if (soundUsages.find(filepath) == soundUsages.end()){
    soundUsages[filepath] = 0;
  }
  soundUsages[filepath] = soundUsages[filepath] + 1;
  return soundSource;
}

ALuint playSourceOneshot(ALuint buffer, std::optional<glm::vec3> position, std::optional<float> volume, std::optional<float> pitch, bool loop, bool center, objid ownerId){
  ALuint source = createSource(buffer);

  std::cout << "playSourceOneshot: " << print(volume) << std::endl;
  if (volume.has_value()){
    std::cout << "set volume: " << volume.value() << std::endl;
    setSoundVolume(source, volume.value());
  }
  setSoundPitch(source, pitch.has_value() ? pitch.value() : 1.f);
  if (position.has_value()){
    setSoundPosition(source, position.value().x, position.value().y, position.value().z);
  }

  if (loop){
    setSoundLooping(source, loop);
  }

  alSourcePlay(source);
  modassert(soundOneshotsSourceToBuffer.find(source) == soundOneshotsSourceToBuffer.end(), "duplicate source");
  soundOneshotsSourceToBuffer[source] = OneShotData {
    .bufferId = buffer,
    .center = center,
    .ownerId = ownerId,
  };
  return source;
}

void maybeRemoveOwnerId(objid ownerId){
  for (auto &[source, oneshotData] : soundOneshotsSourceToBuffer){
    if (oneshotData.ownerId == ownerId){
      oneshotData.shouldErase = true;
    }
  }
}

ALuint getBufferFromSource(ALuint source){
  ALuint bufferId;
  alGetSourcei(source, AL_BUFFER, (ALint*)&bufferId);
  ALenum err = alGetError();
  if (err != AL_NO_ERROR) {
    std::cerr << "OpenAL error querying buffer: " << err << std::endl;
    modassert(false, "error querying buffer");
  }
  return bufferId;
}


int getUsages(std::string filepath){
  if (soundUsages.find(filepath) == soundUsages.end()){
    return 0;
  }
  return soundUsages.at(filepath);
}

std::vector<std::string> listSounds(){
  std::vector<std::string> sounds;
  for (auto [soundname, _ ] : soundBuffers){
    sounds.push_back(soundname);
  }
  return sounds;
}

void unloadSoundState(ALuint source,  std::string filepath){
  int usages = getUsages(filepath);
  assert(usages > 0);
  alDeleteSources(1, &source);
  if (usages == 1){
    soundUsages.erase(filepath);
    ALuint buffer = soundBuffers.at(filepath);

    std::vector<ALuint> sourceToRemove;
    for (auto& [oneshotSource, oneshotData] : soundOneshotsSourceToBuffer){
      if(oneshotData.bufferId == buffer){
        sourceToRemove.push_back(oneshotSource);
      }
    }
    for (auto oneshotSource : sourceToRemove){
      soundOneshotsSourceToBuffer.erase(oneshotSource);
      alDeleteSources(1, &oneshotSource);
    }

    alDeleteBuffers(1, &buffer);
    soundBuffers.erase(filepath);

  }else{
    soundUsages[filepath] = soundUsages[filepath] - 1;
  }
}

bool isSoundFinished(ALuint sourceId){
  ALint state;
  alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
  return state == AL_STOPPED;
}

bool isCurrentOneshot(ALuint sourceId){
  return soundOneshotsSourceToBuffer.find(sourceId) != soundOneshotsSourceToBuffer.end();
}

void onSoundFrame(glm::vec3 listenerPosition){
  for (auto& [sourceId, oneshotData] : soundOneshotsSourceToBuffer){
    if (oneshotData.center){
      setSoundPosition(sourceId, listenerPosition.x, listenerPosition.y, listenerPosition.z);
    }
  }

  std::vector<ALuint> sourceIds;
  for (auto& [sourceId, bufferId] : soundOneshotsSourceToBuffer){
    bool isFinished = isSoundFinished(sourceId) || bufferId.shouldErase;
    if (isFinished){
      sourceIds.push_back(sourceId);
    }
  }
  for (auto sourceId : sourceIds){
    modlog("onSoundFrame remove oneshot", std::to_string(sourceId));
    alDeleteSources(1, &sourceId);
    soundOneshotsSourceToBuffer.erase(sourceId);
  }
}

//////// AUDIO STUFF FOR VIDEO CODE //////////////
const int NUM_AUDIO_BUFFERS = 5; // increasing this can add latency 
BufferedAudio createBufferedAudio(){
  ALuint buffers[NUM_AUDIO_BUFFERS];
  ALuint source;

  alGenSources(1, &source);
  assert(alGetError() == AL_NO_ERROR);

  alGenBuffers(NUM_AUDIO_BUFFERS, buffers);
  assert(alGetError() == AL_NO_ERROR);

  std::vector<ALuint> buffersv;
  for (int i = 0; i < NUM_AUDIO_BUFFERS; i++){
    buffersv.push_back(buffers[i]);
  }
  std::queue<ALuint> freeBuffers;
  for (int i = 0; i < NUM_AUDIO_BUFFERS; i++){
    freeBuffers.push(buffers[i]);
  }

  BufferedAudio audio {
    .source = source,
    .buffers = buffersv,
    .freeBuffers = freeBuffers,
  };
  return audio;
}

void freeBufferedAudio(BufferedAudio& buffer){
  ALuint buffers[NUM_AUDIO_BUFFERS];
  for (int i = 0; i < NUM_AUDIO_BUFFERS; i++){
    buffers[i] = buffer.buffers.at(i);
  }

  alDeleteSources(1, &buffer.source);
  assert(alGetError() == AL_NO_ERROR);
  alDeleteBuffers(NUM_AUDIO_BUFFERS, buffers);
  assert(alGetError() == AL_NO_ERROR);
}

void playBufferedAudio(BufferedAudio& buffer, void* data, int datasize, int samplerate){
  auto alutError = alGetError();
  if (alutError  != AL_NO_ERROR){
    std::cout << "alut error: " << alutError << std::endl;
  }

  auto alError = alGetError();
  ALuint freeBuffer = -1;


  ALint processed = 0;
  alGetSourcei(buffer.source, AL_BUFFERS_PROCESSED, &processed);
  while (processed-- > 0) {
      alSourceUnqueueBuffers(buffer.source, 1, &freeBuffer);
      if (alGetError() == AL_NO_ERROR) {
          buffer.freeBuffers.push(freeBuffer);
      }
  }


  if(!buffer.freeBuffers.empty()){
    ALuint bufferId = buffer.freeBuffers.front();
    buffer.freeBuffers.pop();
    alBufferData(bufferId,  AL_FORMAT_MONO_FLOAT32, data, datasize, samplerate);
    alError = alGetError();
    if (alError != AL_NO_ERROR){
      modlog("buffered audio", "error bufferData");
    }

    alSourceQueueBuffers(buffer.source, 1, &bufferId);
   // assert(alError == AL_NO_ERROR || alError == AL_INVALID_VALUE);
  }else{
    modlog("buffered audio", "no buffers, dropping sample");
  }

  int state;
  alGetSourcei(buffer.source, AL_SOURCE_STATE, &state);
  if (state == AL_STOPPED){
    std::cout << "buffered audio BUFFER STREAM WAS STOPPED!!!" << std::endl;
    alSourcePlay(buffer.source);
  }else if (state == AL_INITIAL){
    std::cout << "buffered audio BUFFER STREAM INITIAL START!!!" << std::endl;
    alSourcePlay(buffer.source);
  }else if (state == AL_PLAYING){

  }else{
    std::cout << "unexpected state" << std::endl;
    assert(false);
  }
}