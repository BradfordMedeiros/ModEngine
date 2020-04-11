#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include "./scene/scene.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scheme_bindings.h"
#include "./scene/sprites/sprites.h"

// This file is really just an extension of main.cpp (notice heavy usage of external scope) but organizes the business logic of the api functions
// These functions represent the functionality that individual language bindings use, but should not be coupled to any language in particular. 
// Should trim down when it becomes clear what the core api should.

void setActiveCamera(short cameraId);
void nextCamera();
void moveCamera(glm::vec3 offset);  
void rotateCamera(float xoffset, float yoffset);

void applyImpulse(short index, glm::vec3 impulse);
void clearImpulse(short index);

short loadScene(std::string sceneFile);
void unloadScene(short sceneId);
std::vector<short> listScenes();

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

short getGameObjectByName(std::string name);
std::vector<short> getObjectsByType(std::string type);
std::string getGameObjectName(short index);
glm::vec3 getGameObjectPosition(short index);
void setGameObjectPosition(short index, glm::vec3 pos);
void setGameObjectPositionRelative(short index, float x, float y, float z, bool xzPlaneOnly);

glm::quat getGameObjectRotation(short index);
void setGameObjectRotation(short index, glm::quat rotation);
void setSelectionMode(bool enabled);

void makeObject(std::string name, std::string meshName, float x, float y, float z);
void removeObjectById(short id);

void drawText(std::string word, float left, float top, unsigned int fontSize);

std::vector<std::string> getAnimationsById(short id);
void playAnimation(short id, std::string animationToPlay);

#endif