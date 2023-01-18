#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include <GLFW/glfw3.h>
#include <queue>          

#include "./scene/scene.h"
#include "./scene/scene_object.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scene/sprites/sprites.h"
#include "./scene/types/sound.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./scene/sysinterface.h"
#include "./drawing.h"
#include "./worldloader.h"
#include "./netscene.h"
#include "./worldtiming.h"
#include "./renderstages.h"
#include "./cscript/cscript.h"
#include "./layers.h"
#include "./benchstats.h"
#include "./lines.h"
#include "./colorselection.h"
#include "./sql/sql.h"

float getTotalTime();

NetworkPacket toNetworkPacket(UdpPacket& packet);

// This file is really just an extension of main.cpp (notice heavy usage of external scope) but organizes the business logic of the api functions
// These functions represent the functionality that individual language bindings use, but should not be coupled to any language in particular. 
// Should trim down when it becomes clear what the core api should.

objid listSceneId(int32_t id);

Transformation getCameraTransform();
void maybeResetCamera(int32_t id);
void setActiveCamera(int32_t cameraId, float interpolationTime);
void setActiveCamera(std::string name, objid sceneId);
void nextCamera();
void moveCamera(glm::vec3 offset, std::optional<bool> relative);  
void moveCamera(glm::vec3 offset);
void rotateCamera(float xoffset, float yoffset);
void setCameraRotation(glm::quat orientation);

void applyImpulse(int32_t index, glm::vec3 impulse);
void applyImpulseRel(int32_t index, glm::vec3 impulse);
void clearImpulse(int32_t index);

std::vector<std::string> listSceneFiles(std::optional<objid> sceneId);
bool parentScene(objid sceneId, objid* _parentSceneId);
std::vector<objid> childScenes(objid sceneId);

int32_t loadScene(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags);
int32_t loadSceneParentOffset(std::string sceneFile, glm::vec3 offset, std::string parentNodeName);
std::optional<objid> sceneIdByName(std::string name);
objid rootIdForScene(objid sceneId);

void unloadScene(int32_t sceneId);
void unloadAllScenes();
void resetScene(std::optional<objid> sceneId);
void saveScene(bool includeIds, objid sceneId, std::optional<std::string> filename);
void saveHeightmap(objid id, std::optional<std::string> filename);

std::vector<int32_t> listScenes(std::optional<std::vector<std::string>> tags);
std::vector<StringPairVec2> scenegraph();

void sendLoadScene(int32_t sceneId);
void createScene(std::string scenename);
void deleteScene(std::string scenename);

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

std::optional<objid> getGameObjectByName(std::string name, objid sceneId, bool sceneIdExplicit);
std::vector<int32_t> getObjectsByType(std::string type);
std::vector<int32_t> getObjectsByAttr(std::string type, std::optional<AttributeValue> value, std::optional<int32_t> sceneId);
std::optional<std::string> getGameObjectName(int32_t index);
GameobjAttributes getGameObjectAttr(int32_t id);
void setGameObjectAttr(int32_t id, GameobjAttributes& attr);

glm::vec3 getGameObjectPosition(int32_t index, bool isWorld);
glm::vec3 getGameObjectPos(int32_t index);
void setGameObjectPosition(int32_t index, glm::vec3 pos);
void setGameObjectPositionRelative(int32_t index, glm::vec3 pos);

glm::vec3 getGameObjectScale(int32_t index);
void setGameObjectScale(int32_t index, glm::vec3 scale);

glm::quat getGameObjectRotation(int32_t index, bool isWorld);
glm::quat getGameObjectRotationRelative(int32_t index); // relative
void setGameObjectRotation(int32_t index, glm::quat rotation);
void setGameObjectRotationRelative(int32_t index, glm::quat rotation);

std::optional<objid> makeObjectAttr(objid sceneId, std::string name, GameobjAttributes& attributes, std::map<std::string, GameobjAttributes>& submodelAttributes);
void removeObjectById(int32_t id);
void copyObject(int32_t id);
void handleCopy();
void handleClipboardSelect();

void drawText(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId);
void drawText(std::string word, float left, float top, unsigned int fontSize);
void drawTextNdi(std::string word, float left, float top, unsigned int fontSize);

FontFamily& fontFamilyByName(std::string name);
int drawWord(GLint shaderProgram, objid id, std::string word, unsigned int fontSize, float offsetDelta, AlignType align, TextWrap wrap, TextVirtualization virtualization, UiTextCursor cursor, std::string fontFamilyName, bool drawBoundingOnly);

std::vector<std::string> listAnimations(int32_t id);
void playAnimation(int32_t id, std::string animationToPlay);
void stopAnimation(int32_t id);

std::vector<std::string> listSounds(int32_t id);
void playSound(std::string sound);

std::vector<std::string> listResources(std::string resourceType);

void sendNotifyMessage(std::string message, std::string value);

double timeSeconds(bool realtime);
double timeElapsed();

void playRecording(objid id, std::string recordingPath, std::optional<RecordingPlaybackType> type);
void stopRecording(objid id);

void tickRecording(float time, GameObject& gameobject);
void tickRecordings(float time);

objid createRecording(objid id);
void saveRecording(objid recordingId, std::string filepath);

std::vector<HitObject> raycastW(glm::vec3 pos, glm::quat direction, float maxDistance);
glm::vec3 moveRelative(glm::vec3, glm::quat orientation, float distance);
glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec);

void nextTexture();
void previousTexture();
void maybeChangeTexture(int index);
void setTexture(objid index, std::string textureName);

void setState(std::string stateName);
void setCrosshairSprite();

void windowPositionCallback(GLFWwindow* window, int xpos, int ypos);
void windowSizeCallback(GLFWwindow* window, int width, int height);
void toggleFullScreen(bool fullscreen);

void setCulling(bool cullEnabled);
void setWorldState(std::vector<ObjectValue> values);
std::vector<ObjectValue> getWorldState();
void setLayerState(std::vector<StrValues> values);
void setFloatState(std::string stateName, float value);
void setIntState(std::string stateName, int value);

void playSoundState(std::string source, objid sceneId);

unsigned int activeTextureId();

struct VoxelQueryData {
  int index;
  GameObjectVoxel* voxelPtr;
};
std::vector<VoxelQueryData> getSelectedVoxels();

void emit(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity);

objid addLoadingAround(objid id);
void removeLoadingAround(objid id);

void makeParent(objid child, objid parent);

void createGeneratedMesh(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string destMesh);

glm::vec3 navPosition(objid id, glm::vec3 target);
bool lock(std::string key, objid owner);
bool unlock(std::string key, objid owner);
void removeLocks(objid owner);

void enforceLayout(objid layoutId);

void takeScreenshot(std::string filepath);

struct UserTexture {
  unsigned int id;
  unsigned int selectionTextureId;
  bool autoclear;
  bool shouldClear;
  std::optional<unsigned int> clearTextureId;
  glm::vec4 clearColor;
};
std::vector<UserTexture> textureIdsToRender();
unsigned int  createTexture(std::string name, unsigned int width, unsigned int height, objid ownerId);
void freeTexture(std::string name, objid ownerId);
void clearTexture(unsigned int textureId, std::optional<bool> autoclear, std::optional<glm::vec4> color, std::optional<std::string> texture);
void markUserTexturesCleared();

std::vector<std::vector<std::string>> executeSqlQuery(sql::SqlQuery& query, bool* valid);
std::map<std::string, std::string> getArgs();

void schedule(objid id, float delayTimeMs, void* data, std::function<void(void*)> fn);
void tickScheduledTasks();
void removeScheduledTask(std::set<objid> ids);
void removeScheduledTaskByOwner(std::set<objid> ids);

#endif