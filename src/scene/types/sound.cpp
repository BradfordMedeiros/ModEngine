#include "./sound.h"

static std::map<std::string, ALuint> soundBuffers;  
static std::map<std::string, int> soundUsages;

void startSoundSystem(){
  alutInit(NULL, NULL);
}
void stopSoundSystem(){
  alutExit();
}

void playSource(ALuint source){
  alSourcePlay(source);
}
std::vector<std::string> listSounds(){
  std::vector<std::string> sounds;
  for (auto [soundname, _ ] : soundBuffers){
    sounds.push_back(soundname);
  }
  return sounds;
}
void setSoundPosition(ALuint source, float x, float y, float z){
  alSource3f(source, AL_POSITION, x, y, z);
}

void setListenerPosition(float x, float y, float z){
  alListener3f(AL_POSITION, x, y, z); 
}


int getUsages(std::string filepath){
  if (soundUsages.find(filepath) == soundUsages.end()){
    return 0;
  }
  return soundUsages.at(filepath);
}

ALuint findOrLoadBuffer(std::string filepath){
  if (soundBuffers.find(filepath) != soundBuffers.end()){
    return soundBuffers.at(filepath);
  }
  ALuint soundBuffer = alutCreateBufferFromFile(filepath.c_str());
  ALenum error = alutGetError();
  if (error != ALUT_ERROR_NO_ERROR){
    std::cerr << "ERROR: " << alutGetErrorString(error) <<  ": " << std::strerror(errno) << std::endl;
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

// @todo support ogg file format.
// This call should support: .wav, .snd, .au , but have only tested .wav
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

void unloadSoundState(ALuint source,  std::string filepath){
  int usages = getUsages(filepath);
  assert(usages > 0);
  alDeleteSources(1, &source);
  if (usages == 1){
    soundUsages.erase(filepath);
    ALuint buffer = soundBuffers.at(filepath);
    alDeleteBuffers(1, &buffer);
    soundBuffers.erase(filepath);
  }else{
    soundUsages[filepath] = soundUsages[filepath] - 1;
  }
}

const int NUM_AUDIO_BUFFERS = 100;
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
    freeBuffers.push(i);
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

bool isPlaying = false;
void playBufferedAudio(BufferedAudio& buffer, uint8_t* data, int datasize, int samplerate){
  auto alutError = alGetError();
  if (alutError  != AL_NO_ERROR){
    std::cout << "alut error: " << alutError << std::endl;
  }

  auto alError = alGetError();
  ALuint freeBuffer = -1;
  while(alError == AL_NO_ERROR){
    alSourceUnqueueBuffers(buffer.source, 1, &freeBuffer);
    alError = alGetError();
    assert(alError == AL_NO_ERROR || alError == AL_INVALID_VALUE);
    if (alError != AL_INVALID_VALUE){
      assert(freeBuffer != -1);
      buffer.freeBuffers.push(freeBuffer);
    }
  }

  if(!buffer.freeBuffers.empty()){
    ALuint bufferId = buffer.freeBuffers.front();
    buffer.freeBuffers.pop();
    alBufferData(bufferId,  AL_FORMAT_STEREO16, data, datasize, samplerate);
    alSourceQueueBuffers(buffer.source, 1, &bufferId);
    assert(alError == AL_NO_ERROR || alError == AL_INVALID_VALUE);
  }

  int state;
  alGetSourcei(buffer.source, AL_SOURCE_STATE, &state);
  if (state == AL_STOPPED){
    std::cout << "BUFFER STREAM WAS STOPPED!!!" << std::endl;
    alSourcePlay(buffer.source);
  }else if (state == AL_INITIAL){
    std::cout << "BUFFER STREAM INITIAL START!!!" << std::endl;
    alSourcePlay(buffer.source);
  }else if (state == AL_PLAYING){

  }else{
    std::cout << "unexpected state" << std::endl;
    assert(false);
  }
}