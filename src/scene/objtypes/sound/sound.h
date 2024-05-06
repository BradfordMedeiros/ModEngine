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
#include <glm/glm.hpp>
#include "../../../common/util.h"
// https://www.openal.org/documentation/OpenAL_Programmers_Guide.pdf

void startSoundSystem();
void stopSoundSystem();
ALuint loadSoundState(std::string filepath);
void unloadSoundState(ALuint source, std::string filepath);
void playSource(ALuint source, std::optional<float> volume, std::optional<glm::vec3> position);
void stopSource(ALuint source);

float getVolume();
void setVolume(float volume);

std::vector<std::string> listSounds();
void setSoundPosition(ALuint source, float x, float y, float z);
void setSoundVolume(ALuint source, float newVolume);
void setSoundLooping(ALuint source, bool shouldLoop);

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