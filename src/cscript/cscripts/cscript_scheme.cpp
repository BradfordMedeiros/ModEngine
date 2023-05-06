#include "./cscript_scheme.h"

auto schemeCallbacks = getSchemeCallbacks();

std::function<std::string(std::string)> pathForModLayer;

void* createSchemeScript(std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript){
  loadScript(scriptname, id, sceneId, isServer, isFreeScript, pathForModLayer);
  return NULL;
}

void unloadSchemeScript(std::string scriptname, objid id, void* data) {
  unloadScript(scriptname, id);
}

CScriptBinding cscriptSchemeBinding(CustomApiBindings& api, std::function<std::string(std::string)> modlayerPath){
  auto binding = createCScriptBinding(".*\\.scm", api);
  pathForModLayer =  modlayerPath;

  binding.onFrame = schemeCallbacks.onFrame;

  // todo convert these callbacks to be parameterized by id 
  binding.onCollisionEnter = schemeCallbacks.onCollisionEnter;
  binding.onCollisionExit = schemeCallbacks.onCollisionExit;
  binding.onMouseCallback = schemeCallbacks.onMouseCallback;
  binding.onMouseMoveCallback = schemeCallbacks.onMouseMoveCallback;
  binding.onScrollCallback = schemeCallbacks.onScrollCallback;
  binding.onObjectSelected = schemeCallbacks.onObjectSelected;
  binding.onObjectUnselected = schemeCallbacks.onObjectUnselected;
  binding.onObjectHover = schemeCallbacks.onObjectHover;
  binding.onMapping = schemeCallbacks.onMapping;
  binding.onKeyCallback = schemeCallbacks.onKeyCallback;
  binding.onKeyCharCallback = schemeCallbacks.onKeyCharCallback;
  binding.onCameraSystemChange = schemeCallbacks.onCameraSystemChange;
  binding.onMessage = schemeCallbacks.onMessage;
  binding.onTcpMessage = schemeCallbacks.onTcpMessage;
  binding.onUdpMessage = schemeCallbacks.onUdpMessage;
  binding.onPlayerJoined = schemeCallbacks.onPlayerJoined;
  binding.onPlayerLeave = schemeCallbacks.onPlayerLeave;
  binding.onObjectAdded = schemeCallbacks.onObjectAdded;
  binding.onObjectRemoved = schemeCallbacks.onObjectRemoved;
  // notice binding.render not implemented

  createStaticSchemeBindings(
    api.listSceneId,
    api.loadScene,  
    api.unloadScene,  
    api.unloadAllScenes,
    api.resetScene,
    api.listScenes,  
    api.listSceneFiles,
    api.parentScene,
    api.childScenes,
    api.sceneIdByName,
    api.rootIdForScene,
    api.scenegraph,
    api.sendLoadScene,
    api.createScene,
    api.deleteScene,
    api.moveCamera,  
    api.rotateCamera,
    api.removeObjectById,
    api.getObjectsByType,
    api.getObjectsByAttr,
    api.setActiveCamera,
    api.drawText,
    api.drawRect,
    api.drawLine,
    api.freeLine,
    api.getGameObjNameForId,
    api.getGameObjectAttr,
    api.setGameObjectAttr,
    api.getGameObjectPos,
    api.setGameObjectPosition,
    api.getGameObjectRotation,
    api.setGameObjectRot,
    api.setFrontDelta,
    api.moveRelative,
    api.moveRelativeVec,
    api.orientationFromPos,
    api.getGameObjectByName,
    api.applyImpulse,
    api.applyImpulseRel,
    api.clearImpulse,
    api.listAnimations,
    api.playAnimation,
    api.listClips,
    api.playClip,
    api.listResources,
    api.sendNotifyMessage,
    api.timeSeconds,
    api.timeElapsed,
    api.saveScene,
    api.saveHeightmap,
    api.listServers,
    api.connectServer,
    api.disconnectServer,
    api.sendMessageTcp,
    api.sendMessageUdp,
    api.playRecording,
    api.stopRecording,
    api.createRecording,
    api.saveRecording,
    api.makeObjectAttr,
    api.makeParent,
    api.raycast,
    api.saveScreenshot,
    api.setState,
    api.setFloatState,
    api.setIntState,
    api.navPosition,
    api.emit,
    api.loadAround,
    api.rmLoadAround,
    api.generateMesh,
    api.getArgs,
    api.lock,
    api.unlock,
    api.debugInfo,
    api.setWorldState,
    api.getWorldState,
    api.setLayerState,
    api.enforceLayout,
    api.createTexture,
    api.freeTexture,
    api.clearTexture,
    api.runStats,
    api.stat,
    api.logStat,
    api.installMod,
    api.uninstallMod,
    api.listMods,
    api.compileSqlQuery,
    api.executeSqlQuery,
    api.selected,
    api.click,
    api.moveMouse,
    {}
  );

  binding.create = createSchemeScript;
  binding.remove = unloadSchemeScript;

  return binding;
}