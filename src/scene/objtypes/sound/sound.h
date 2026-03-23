#ifndef MOD_SOUNDMANAGER
#define MOD_SOUNDMANAGER

#include <map>
#include <vector>
#include <assert.h>
#include <iostream>
#include <AL/alut.h>
#include <AL/alext.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <queue>
#include <glm/glm.hpp>
#include <vorbis/vorbisfile.h>

#include "../../../common/util.h"
// https://www.openal.org/documentation/OpenAL_Programmers_Guide.pdf

// Global stuff ////
void startSoundSystem();
void stopSoundSystem();
float getVolume();
void setVolume(float volume);
void setListenerPosition(float x, float y, float z, std::vector<float> forward, std::vector<float> up);


ALuint loadSoundState(std::string filepath);
void unloadSoundState(ALuint source, std::string filepath);
void playSource(ALuint source);
void stopSource(ALuint source);
std::vector<std::string> listSounds();
void onSoundFrame(glm::vec3 listenerPosition);


void setSoundPosition(ALuint source, float x, float y, float z);
void setSoundVolume(ALuint source, float newVolume);
void setSoundLooping(ALuint source, bool shouldLoop);
void setSoundPitch(ALuint source, float pitchMultiplier);

ALuint playSourceOneshot(ALuint buffer, std::optional<glm::vec3> position, std::optional<float> volume, bool loop, bool center);
ALuint getBufferFromSource(ALuint source);
bool isCurrentOneshot(ALuint sourceId);

struct OneShot {
  ALuint source;
};

// Video ////

struct BufferedAudio {
  ALuint source;
  std::vector<ALuint> buffers;
  std::queue<ALuint> freeBuffers;
};

BufferedAudio createBufferedAudio();
void freeBufferedAudio(BufferedAudio& buffer);
void playBufferedAudio(BufferedAudio& buffer, void* data, int datasize, int samplerate);

#endif