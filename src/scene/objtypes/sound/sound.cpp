#include "./sound.h"

std::string readFileOrPackage(std::string filepath); // i dont really like directly referencing this here, but...it's ok
unsigned int openFileOrPackage(std::string filepath);
int closeFileOrPackage(unsigned int handle);
size_t readFileOrPackage(unsigned int handle, void *ptr, size_t size, size_t nmemb);
int seekFileOrPackage(unsigned int handle, int offset, int whence);
size_t tellFileOrPackage(unsigned int handle);
static std::unordered_map<std::string, ALuint> soundBuffers;  
static std::unordered_map<std::string, int> soundUsages;

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

void playSource(ALuint source, std::optional<float> volume, std::optional<glm::vec3> position){
  ALfloat oldVolume;
  ALfloat x;
  ALfloat y;
  ALfloat z;

  if (volume.has_value()){
    alGetSourcef(source, AL_GAIN, &oldVolume);
    setSoundVolume(source, volume.value());
  }
  if (position.has_value()){
    alGetSource3f(source, AL_POSITION, &x, &y, &z);
    setSoundPosition(source, position.value().x, position.value().y, position.value().z);
  }

  alSourcePlay(source);

  if (volume.has_value()){
    //setSoundVolume(source, oldVolume);
  }
  if (position.has_value()){
    setSoundPosition(source, x, y, z);
  }
}

void stopSource(ALuint source){
  alSourceStop(source);
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

void setSoundVolume(ALuint source, float newVolume){
  alSourcef(source, AL_GAIN, newVolume);
}
void setSoundLooping(ALuint source, bool shouldLoop){
  alSourcei(source, AL_LOOPING, shouldLoop ? AL_TRUE : AL_FALSE);
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



int getUsages(std::string filepath){
  if (soundUsages.find(filepath) == soundUsages.end()){
    return 0;
  }
  return soundUsages.at(filepath);
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

void readVorbisFile(std::string& filepath, ALuint* _soundBuffer){
  modlog("vorbis", filepath);
  modlog("vorbis", "open");
  unsigned int fileHandle = openFileOrPackage(filepath.c_str());

  OggVorbis_File vf;
  auto result = ov_open_callbacks(&fileHandle, &vf, NULL, 0, vorbisFileFns); 
  if (result < 0) {
    modassert(false, std::string("Error setting up callbacks: ") + std::to_string(result));
  }

  vorbis_info* vi = ov_info(&vf, -1);

  std::cout << "bitrate is: " << vi -> bitrate_nominal << std::endl;

  size_t bufferSize = ov_pcm_total(&vf, -1) * vi -> channels * 2; // 2 bytes per sample, per channel 
  bufferSize = bufferSize + 1; // + 1 since need to get 0 bytes read on the last byte to detect finished

  char* buffer = (char*)malloc(bufferSize);

  int bitstream;
  int bufferOffset = 0;
  int sizePerRead = 4096;
  int expectedReadLimit = bufferSize / sizePerRead;

  int numReads = 0;
  while(true){
    if (numReads > (expectedReadLimit * 10)){
      modassert(false, "Took too long reading vorbis audio file");
    }
    numReads++;

    auto bytesRemaining = bufferSize - bufferOffset;
    if (bytesRemaining < sizePerRead){
      sizePerRead = bytesRemaining;
    }
    modassert(sizePerRead != 0, "cannot provide empty buffer to read");

    auto value = ov_read(&vf, buffer + bufferOffset, sizePerRead, 0, 2, 1, &bitstream);
    if (value == OV_HOLE || value == OV_EBADLINK || value == OV_EINVAL || value < 0){
      modassert(false, "bad file read");
    }
    bufferOffset = bufferOffset + value;
    if (value == 0){
      break;
    }
  }

  auto format = vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

  alGenBuffers(1, _soundBuffer);
  alBufferData(*_soundBuffer, format, buffer, bufferOffset, vi -> rate);

  free(buffer);
  ov_clear(&vf); // this closes the file handle  https://xiph.org/vorbis/doc/vorbisfile/ov_open.html
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

bool isPlaying = false;
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