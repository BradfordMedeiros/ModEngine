#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include <GLFW/glfw3.h>
#include "./scene/scene.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scheme/scriptmanager.h"
#include "./scene/sprites/sprites.h"
#include "./scene/animation/playback.h"
#include "./scene/animation/timeplayback.h"
#include "./sounds/soundmanager.h"

// This file is really just an extension of main.cpp (notice heavy usage of external scope) but organizes the business logic of the api functions
// These functions represent the functionality that individual language bindings use, but should not be coupled to any language in particular. 
// Should trim down when it becomes clear what the core api should.

void setActiveCamera(short cameraId);
void nextCamera();
void moveCamera(glm::vec3 offset);  
void rotateCamera(float xoffset, float yoffset);

void applyImpulse(short index, glm::vec3 impulse);
void applyImpulseRel(short index, glm::vec3 impulse);
void clearImpulse(short index);

short loadScene(std::string sceneFile);
void unloadScene(short sceneId);
void saveScene();
std::vector<short> listScenes();

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

short getGameObjectByName(std::string name);
std::vector<short> getObjectsByType(std::string type);
std::string getGameObjectName(short index);
std::map<std::string, std::string> getGameObjectAttr(short id);
void setGameObjectAttr(short id, std::map<std::string, std::string> attr);

glm::vec3 getGameObjectPosition(short index);
void setGameObjectPosition(short index, glm::vec3 pos);
void setGameObjectPositionRelative(short index, float x, float y, float z, bool xzPlaneOnly);

glm::vec3 getGameObjectScale(short index);
void setGameObjectScale(short index, glm::vec3 scale);

glm::quat getGameObjectRotation(short index);
void setGameObjectRotation(short index, glm::quat rotation);
void setSelectionMode(bool enabled);

void makeObject(std::string name, std::string meshName, float x, float y, float z);
void removeObjectById(short id);

void drawText(std::string word, float left, float top, unsigned int fontSize);

struct AnimationState {
  std::map<short, TimePlayback> playbacks;
};
std::vector<std::string> listAnimations(short id);
void playAnimation(short id, std::string animationToPlay);

std::vector<std::string> listSounds(short id);
void playSound(std::string sound);

std::vector<std::string> listModels();
void sendEventMessage(std::string message);

void attachToRail(short id, std::string rail);
void unattachFromRail(short id);
double timeSeconds();

#endif