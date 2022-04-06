#include "./cscript_scheme.h"

void* createSchemeScript(){
  return NULL;
}

CustomObjBinding cscriptSchemeBinding(CustomApiBindings& api){
  auto binding = createCustomBinding("./res/scripts/color.scm", api);

  createStaticSchemeBindings(
    api.listSceneId,
    api.loadScene,  
    api.unloadScene,  
    api.unloadAllScenes,
    api.listScenes,  
    api.listSceneFiles,
    api.sendLoadScene,
    api.createScene,
    api.moveCamera,  
    api.rotateCamera,
    api.removeObjectById,
    api.getObjectsByType,
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
    api.enforceLayout,
    {}
  );

  binding.create = createSchemeScript;
  binding.remove = [&api] (void* data) -> void {
    
  };
  return binding;
}