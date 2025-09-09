#ifndef MOD_MAINAPI
#define MOD_MAINAPI

#include <GLFW/glfw3.h>
#include <queue>          

#include "./scene/scene_object.h"
#include "./state.h"
#include "./scene/physics.h"
#include "./scene/sprites/sprites.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./scene/sysinterface.h"
#include "./worldloader.h"
#include "./netscene.h"
#include "./worldtiming.h"
#include "./renderstages.h"
#include "./cscript/cscript.h"
#include "./layers.h"
#include "./perf/benchstats.h"
#include "./lines.h"
#include "./sql/sql.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/scene_sandbox.h"
#include "./package.h"

struct DefaultResources {
  GameObject defaultCamera;
  unsigned int quadVAO;  
  unsigned int quadVAO3D; 
  std::vector<FontFamily> fontFamily;
  DefaultMeshes defaultMeshes;
};

float getTotalTimeGame();
float getTotalTime();

NetworkPacket toNetworkPacket(UdpPacket& packet);

// This file is really just an extension of main.cpp (notice heavy usage of external scope) but organizes the business logic of the api functions
// These functions represent the functionality that individual language bindings use, but should not be coupled to any language in particular. 
// Should trim down when it becomes clear what the core api should.

objid listSceneId(int32_t id);

Transformation getCameraTransform();
Transformation getCullingTransform();
void maybeResetCamera(int32_t id);
void setActiveCamera(std::optional<int32_t> cameraId);
std::optional<objid> getActiveCamera();
Transformation getView();

void nextCamera();
void moveCamera(glm::vec3 offset);
void rotateCamera(float xoffset, float yoffset);
void setCameraRotation(glm::quat orientation);

void applyImpulse(int32_t index, glm::vec3 impulse);
void applyImpulseRel(int32_t index, glm::vec3 impulse);
void clearImpulse(int32_t index);
void applyForce(int32_t index, glm::vec3 force);
void applyTorque(int32_t index, glm::vec3 torque);
std::optional<ModAABB> getModAABB(int32_t index);
std::optional<ModAABB2> getModAABBModel(int32_t index);
std::optional<PhysicsInfo> getPhysicsInfo(int32_t index, bool group);

std::vector<std::string> listSceneFiles(std::optional<objid> sceneId);

int32_t loadScene(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags);
std::optional<objid> sceneIdByName(std::string name);
std::optional<std::string> sceneNameById(objid id);
objid rootSceneId();

void unloadScene(int32_t sceneId);
void doUnloadScenes();

void resetScene(std::optional<objid> sceneId);
bool saveScene(bool includeIds, objid sceneId, std::optional<std::string> filename);

std::vector<int32_t> listScenes(std::optional<std::vector<std::string>> tags);
std::set<objid> listObjAndDescInScene(objid sceneId);
std::set<objid> getChildrenIdsAndParent(objid id);
std::vector<ScenegraphDebug> scenegraph();

void sendLoadScene(int32_t sceneId);
void createScene(std::string scenename);
void deleteScene(std::string scenename);

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, float force);
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2);

bool gameobjExists(objid id);
std::optional<objid> getGameObjectByName(std::string name, objid sceneId);
std::vector<int32_t> getObjectsByAttr(std::string type, std::optional<AttributeValue> value, std::optional<int32_t> sceneId);
std::optional<std::string> getGameObjectName(int32_t index);
std::optional<AttributeValuePtr> getObjectAttributePtr(int32_t id, const char* field);
std::optional<AttributeValue> getObjectAttribute(int32_t id, const char* field);
void setGameObjectAttr(int32_t id, std::vector<GameobjAttribute> attrs);
void setSingleGameObjectAttr(int32_t id, const char* field, AttributeValue value);

glm::vec3 getPhysicsVelocity(int32_t id);
void setPhysicsVelocity(int32_t id, glm::vec3 velocity);

glm::vec3 getGameObjectPosition(int32_t index, bool isWorld, const char* hint);
void setGameObjectPosition(int32_t index, glm::vec3 pos, bool isWorld, Hint hint);

glm::vec3 getGameObjectScale(int32_t index);
glm::vec3 getGameObjectScale2(int32_t index, bool isWorld);
void setGameObjectScale(int32_t index, glm::vec3 scale, bool isWorld);

glm::quat getGameObjectRotation(int32_t index, bool isWorld, const char* hint);
void setGameObjectRotation(int32_t index, glm::quat rotation, bool isWorld, Hint hint);

std::optional<objid> makeObjectAttr(objid sceneId, std::string name, GameobjAttributes& attributes, std::unordered_map<std::string, GameobjAttributes>& submodelAttributes);
std::vector<objid> idsInGroupById(objid);
objid groupId(objid);

void removeObjectById(int32_t id);
void removeByGroupId(int32_t idInGroup);
void doRemoveQueuedRemovals();

std::optional<objid> prefabId(objid id);

void handleCopy();
void handleClipboardSelect();

void drawText(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId, std::optional<float> maxWidthNdi, std::optional<ShapeOptions> shaderId);
void drawText(std::string word, float left, float top, unsigned int fontSize);
void drawTextNdi(std::string word, float left, float top, unsigned int fontSize);
void getTextDimensionsNdi(std::string word, float fontSizeNdi, bool ndi, std::optional<std::string> fontFamily, float* _width, float* _height);

FontFamily& fontFamilyByName(std::optional<std::string> name);
int drawWord(GLint shaderProgram, objid id, std::string word, unsigned int fontSize, AlignType align, TextWrap wrap, TextVirtualization virtualization, UiTextCursor cursor, std::string fontFamilyName, bool drawBoundingOnly);

void drawRect(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<ShapeOptions> shaderId);
void drawLine2D(glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture, std::optional<ShapeOptions> shaderId);

std::set<std::string> listAnimations(int32_t id);
void playAnimation(int32_t id, std::string animationToPlay, AnimationType animationType, std::optional<std::set<objid>> mask, int zIndex, bool invertMask, std::optional<float> holdTime);
void stopAnimation(int32_t id);
void disableAnimationIds(std::set<objid>& ids);
void setAnimationPose(int32_t id, std::string animationToPlay, float time);
void clearAnimationPose(int32_t id);
std::optional<float> animationLength(int32_t, std::string animationToPlay);

std::vector<std::string> listSounds(int32_t id);
void playSound(std::string sound);
void stopSoundState(std::string source, objid sceneId);
void stopSoundStateById(objid id);

std::vector<std::string> listResources(std::string resourceType);

void sendNotifyMessage(std::string message, std::any value);
void sendAlert(std::string message);

double timeSeconds(bool realtime);
double timeElapsed();
int currentFrame();

std::vector<HitObject> raycastW(glm::vec3 pos, glm::quat direction, float maxDistance);
std::vector<HitObject> contactTest(objid id);
std::vector<HitObject> contactTestShape(glm::vec3 pos, glm::quat orientation, glm::vec3 scale);
glm::vec3 moveRelative(glm::vec3, glm::quat orientation, float distance);
glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec);

void previousTexture();
void maybeChangeTexture(int index);

void setNavmeshTexture(unsigned int);
void setTexture(objid index, std::string textureName);

void setState(std::string stateName);

void windowPositionCallback(GLFWwindow* window, int xpos, int ypos);
void windowSizeCallback(GLFWwindow* window, int width, int height);
void toggleFullScreen(bool fullscreen);

void setCulling(bool cullEnabled);
void setWorldState(std::vector<ObjectValue> values);
std::vector<ObjectValue> getWorldState();
void setLayerState(std::vector<StrValues> values);

void playSoundState(std::string source, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position);
void playSoundState(objid id, std::optional<float> volume, std::optional<glm::vec3> position);

unsigned int activeTextureId();

void emit(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity, std::optional<objid> parentId);

objid addLoadingAround(objid id);
void removeLoadingAround(objid id);

void makeParent(objid child, objid parent);
std::optional<objid> getParent(objid id);

void createGeneratedMesh(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string destMesh);
void createGeneratedMeshRaw(std::vector<glm::vec3>& verts, std::vector<glm::vec2>& uvCoords, std::vector<unsigned int>& indexs, std::string destMesh);

objid addLineNextCycle(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> color, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth);
objid addLineNextCyclePhysicsDebug(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner);

glm::vec3 navPosition(objid id, glm::vec3 target);

void takeScreenshot(std::string filepath);

struct UserTexture {
  unsigned int id;
  unsigned int ownerId;
  unsigned int selectionTextureId;
  bool autoclear;
  bool shouldClear;
  std::optional<unsigned int> clearTextureId;
  glm::vec4 clearColor;
};
std::vector<UserTexture> textureIdsToRender();
unsigned int  createTexture(std::string name, unsigned int width, unsigned int height, objid ownerId);
void freeTexture(std::string name, objid ownerId);
void freeTexture(objid ownerId);
void clearTexture(unsigned int textureId, std::optional<bool> autoclear, std::optional<glm::vec4> color, std::optional<std::string> texture);
void markUserTexturesCleared();

std::vector<std::vector<std::string>> executeSqlQuery(sql::SqlQuery& query, bool* valid);
std::unordered_map<std::string, std::string> getArgs();

void schedule(objid id, bool realtime, float delayTimeMs, void* data, std::function<void(void*)> fn);
void tickScheduledTasks();
void removeScheduledTask(std::set<objid> ids);
void removeScheduledTaskByOwner(std::set<objid> ids);

RotationDirection getCursorInfoWorld();

void setLogEndpoint(std::optional<std::function<void(std::string&)>> fn);
const char* getClipboardString();
void setClipboardString(const char* string);

void sendManipulatorEvent(MANIPULATOR_EVENT event);


bool saveState(std::string);
bool loadState(std::string);

void setSelected(std::optional<std::set<objid>> ids);
std::optional<unsigned int> getTextureSamplerId(std::string& texturepath);
void bindTexture(unsigned int program, unsigned int textureUnit, unsigned int textureId);
void unbindTexture(unsigned int program, unsigned int textureUnit);

struct IdAtCoords {
  float ndix;
  float ndiy;
  bool onlyGameObjId;
  std::optional<objid> result;
  glm::vec2 resultUv;
  std::optional<objid> textureId;
  std::function<void(std::optional<objid>, glm::vec2)> afterFrame;
};
void idAtCoordAsync(float ndix, float ndiy, bool onlyGameObjId, std::optional<objid> textureId, std::function<void(std::optional<objid>, glm::vec2)> afterFrame);

struct DepthAtCoord {
  float ndix;
  float ndiy;
  std::optional<float> resultDepth;
  std::function<void(float depth)> afterFrame;
};
void depthAtCoordAsync(float ndix, float ndiy, std::function<void(float)> afterFrame);

std::vector<TagInfo> getTag(int tag, glm::vec3 position);
std::vector<TagInfo> getAllTags(int tag);
std::optional<objid> getMainOctreeId();
std::optional<OctreeMaterial> getMaterial(glm::vec3 position);

void createPhysicsBody(objid id, ShapeCreateType option);
void setPhysicsOptions(objid id, rigidBodyOpts& opts);
void createFixedConstraint(objid idOne, objid idTwo);
void createPointConstraint(objid idOne, objid idTwo);
void createHingeConstraint(objid idOne, objid idTwo);
std::optional<int> physicsLayer(objid id);

#endif