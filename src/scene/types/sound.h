#ifndef MOD_SOUNDMANAGER
#define MOD_SOUNDMANAGER

#include <map>
#include <vector>
#include <assert.h>
#include <iostream>
#include <AL/alut.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <queue>

// https://www.openal.org/documentation/OpenAL_Programmers_Guide.pdf

void startSoundSystem();
void stopSoundSystem();
ALuint loadSoundState(std::string filepath, bool shouldLoop);
void unloadSoundState(ALuint source, std::string filepath);
void playSource(ALuint source);

std::vector<std::string> listSounds();
void setSoundPosition(ALuint source, float x, float y, float z);
void setListenerPosition(float x, float y, float z, std::vector<float> forward, std::vector<float> up);

struct BufferedAudio {
  ALuint source;
  std::vector<ALuint> buffers;
  std::queue<ALuint> freeBuffers;
};

BufferedAudio createBufferedAudio();
void freeBufferedAudio(BufferedAudio& buffer);
void playBufferedAudio(BufferedAudio& buffer, uint8_t* data, int datasize, int samplerate);

#endif