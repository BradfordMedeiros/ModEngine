#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include "./scene/scene.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scheme_bindings.h"

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

#endif