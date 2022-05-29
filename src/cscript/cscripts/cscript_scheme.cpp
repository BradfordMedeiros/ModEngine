#include "./cscript_scheme.h"

auto schemeCallbacks = getSchemeCallbacks();

void* createSchemeScript(std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript){
  loadScript(scriptname, id, sceneId, isServer, isFreeScript);
  return NULL;
}

void unloadSchemeScript(std::string scriptname, objid id, void* data) {
  unloadScript(scriptname, id);
}

CScriptBinding cscriptSchemeBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding(".*\\.scm", api);

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
  binding.onKeyCallback = schemeCallbacks.onKeyCallback;
  binding.onKeyCharCallback = schemeCallbacks.onKeyCharCallback;
  binding.onCameraSystemChange = schemeCallbacks.onCameraSystemChange;
  binding.onMessage = schemeCallbacks.onMessage;
  binding.onTcpMessage = schemeCallbacks.onTcpMessage;
  binding.onUdpMessage = schemeCallbacks.onUdpMessage;
  binding.onPlayerJoined = schemeCallbacks.onPlayerJoined;
  binding.onPlayerLeave = schemeCallbacks.onPlayerLeave;
  // notice binding.render not implemented

  createStaticSchemeBindings(
    api.listSceneId,
    api.loadScene,  
    api.unloadScene,  
    api.unloadAllScenes,
    api.listScenes,  
    api.listSceneFiles,
    api.parentScene,
    api.childScenes,
    api.sceneIdByName,
    api.rootIdForScene,
    api.sendLoadScene,
    api.createScene,
    api.moveCamera,  
    api.rotateCamera,
    api.removeObjectById,
    api.getObjectsByType,
    api.getObjectsByAttr,
    api.setActiveCamera,
    api.drawText,
    api.drawLine,
    api.freeLine,
    api.getGameObjectNameForId,
    api.getGameObjectAttr,
    api.setGameObjectAttr,
    api.getGameObjectPos,
    api.setGameObjectPos,
    api.setGameObjectPosRelative,
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
    api.listModels,
    api.sendNotifyMessage,
    api.timeSeconds,
    api.timeElapsed,
    api.saveScene, 
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
    api.setLayerState,
    api.enforceLayout,
    {}
  );

  binding.create = createSchemeScript;
  binding.remove = unloadSchemeScript;

  return binding;
}