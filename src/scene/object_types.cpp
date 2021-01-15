#include "./object_types.h"

std::map<objid, GameObjectObj> getObjectMapping() {
	std::map<objid, GameObjectObj> objectMapping;
	return objectMapping;
}

TextureInformation texinfoFromFields(std::map<std::string, std::string>& additionalFields, std::function<Texture(std::string)> ensureTextureLoaded){
  glm::vec2 textureoffset = additionalFields.find("textureoffset") == additionalFields.end() ? glm::vec2(0.f, 0.f) : parseVec2(additionalFields.at("textureoffset"));
  glm::vec2 texturetiling = additionalFields.find("texturetiling") == additionalFields.end() ? glm::vec2(1.f, 1.f) : parseVec2(additionalFields.at("texturetiling"));
  std::string textureOverloadName = additionalFields.find("texture") == additionalFields.end() ? "" : additionalFields.at("texture");
  int textureOverloadId = textureOverloadName == "" ? -1 : ensureTextureLoaded(textureOverloadName).textureId;

  TextureInformation info {
    .textureoffset = textureoffset,
    .texturetiling = texturetiling,
    .textureOverloadName = textureOverloadName,
    .textureOverloadId = textureOverloadId,
  };
  return info;
}

static std::vector<std::string> meshFieldsToCopy = { "textureoffset", "texturetiling", "texture", "discard", "emission", "tint" };
GameObjectMesh createMesh(
  std::map<std::string, std::string> additionalFields, 
  std::map<std::string, Mesh>& meshes, 
  std::string defaultMesh, 
  std::function<bool(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<Texture(std::string)> ensureTextureLoaded
){
  std::string rootMeshName = additionalFields.find("mesh") == additionalFields.end()  ? "" : additionalFields.at("mesh");
  bool usesMultipleMeshes = additionalFields.find("meshes") != additionalFields.end();


  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;

  if (usesMultipleMeshes){
    auto meshStrings = split(additionalFields.at("meshes"), ',');
    for (auto meshName : meshStrings){
      bool loadedMesh = ensureMeshLoaded(meshName, meshFieldsToCopy);
      if (loadedMesh){
        meshNames.push_back(meshName);
        meshesToRender.push_back(meshes.at(meshName));  
      }
    }
  }else{
    auto meshName = (additionalFields.find("mesh") != additionalFields.end()) ? additionalFields.at("mesh") : defaultMesh;
    meshName = (meshName == "") ? defaultMesh : meshName;
    bool loadedMesh = ensureMeshLoaded(meshName, meshFieldsToCopy);
    if (loadedMesh){
      meshNames.push_back(meshName);
      meshesToRender.push_back(meshes.at(meshName));   
    }
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .isDisabled = additionalFields.find("disabled") != additionalFields.end(),
    .nodeOnly = meshNames.size() == 0,
    .rootMesh = rootMeshName,
    .texture = texinfoFromFields(additionalFields, ensureTextureLoaded),
    .discardAmount = additionalFields.find("discard") == additionalFields.end() ? 0.f : std::atof(additionalFields.at("emission").c_str()),
    .emissionAmount = additionalFields.find("emission") == additionalFields.end() ? 0.f : std::atof(additionalFields.at("emission").c_str()),
    .tint = additionalFields.find("tint") == additionalFields.end() ? glm::vec3(1.f, 1.f, 1.f) : parseVec(additionalFields.at("tint").c_str()),
  };
  return obj;
}
GameObjectCamera createCamera(){
  GameObjectCamera obj {};
  return obj;
}
GameObjectPortal createPortal(std::map<std::string, std::string> additionalFields){
  bool hasCamera =  additionalFields.find("camera") != additionalFields.end();
  auto camera = hasCamera ? additionalFields.at("camera") : "";
  auto perspective = additionalFields.find("perspective") != additionalFields.end() ? additionalFields.at("perspective") == "true" : false;

  GameObjectPortal obj {
    .camera = camera,
    .perspective = perspective,
  };
  return obj;
}
GameObjectSound createSound(std::map<std::string, std::string> additionalFields){
  auto clip = additionalFields.at("clip");
  auto source = loadSoundState(clip);
  GameObjectSound obj {
    .clip = clip,
    .source = source,
  };
  return obj;
}

LightType getLightType(std::string type){
  if (type == "spotlight"){
    return LIGHT_SPOTLIGHT;
  }
  if (type == "directional"){
    return LIGHT_DIRECTIONAL;
  }
  return LIGHT_POINT;
}
GameObjectLight createLight(std::map<std::string, std::string> additionalFields){
  auto color = additionalFields.find("color") == additionalFields.end() ? glm::vec3(1.f, 1.f, 1.f) : parseVec(additionalFields.at("color"));
  auto lightType = additionalFields.find("type") == additionalFields.end() ? LIGHT_POINT : getLightType(additionalFields.at("type"));
  auto maxangle = (lightType != LIGHT_SPOTLIGHT || additionalFields.find("angle") == additionalFields.end()) ? -10.f : std::atof(additionalFields.at("angle").c_str());

  // constant, linear, quadratic
  // in shader =>  float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  
  auto attenuation = additionalFields.find("attenuation") == additionalFields.end() ? glm::vec3(1.0, 0.007, 0.0002) : parseVec(additionalFields.at("attenuation"));

  GameObjectLight obj {
    .color = color,
    .type = lightType,
    .maxangle = maxangle, 
    .attenuation = attenuation,
  };
  return obj;
}

GameObjectVoxel createVoxel(std::map<std::string, std::string> additionalFields, std::function<void()> onVoxelBoundInfoChanged, unsigned int defaultTexture, std::function<Texture(std::string)> ensureTextureLoaded){
  auto textureString = additionalFields.find("fromtextures") == additionalFields.end() ? "" : additionalFields.at("fromtextures");
  auto voxel = createVoxels(parseVoxelState(additionalFields.at("from"), textureString, defaultTexture, ensureTextureLoaded), onVoxelBoundInfoChanged, defaultTexture);
  GameObjectVoxel obj {
    .voxel = voxel,
  };
  return obj;
}

GameObjectChannel createChannel(std::map<std::string, std::string> additionalFields){
  bool hasFrom = additionalFields.find("from") != additionalFields.end();
  bool hasTo = additionalFields.find("to") != additionalFields.end();

  GameObjectChannel obj {
    .from = hasFrom ? additionalFields.at("from") : "",
    .to = hasTo ? additionalFields.at("to") : "",
    .complete = hasFrom && hasTo,
  };
  return obj;
}

GameObjectScene createScene(objid id, std::map<std::string, std::string> additionalFields, std::function<void(std::string)> loadScene){
  auto scenefile = additionalFields.at("scene");
  loadScene(scenefile);
  GameObjectScene obj {
    .scenefile = scenefile,
  };
  return obj;
}

std::map<std::string, std::string> particleFields(std::map<std::string, std::string> additionalFields){
  std::map<std::string, std::string> particleAttributes;
  for (auto [key, value] : additionalFields){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      particleAttributes[newKey] = value;
    }
  }
  return particleAttributes;
}

struct ValueVariance {
  glm::vec3 value;
  glm::vec3 variance;
  std::vector<float> lifetimeEffect;
};
std::vector<EmitterDelta> emitterDeltas(std::map<std::string, std::string> additionalFields){
  std::map<std::string, ValueVariance> values;
  for (auto [key, value] : additionalFields){
    if ((key.at(0) == '!' || key.at(0) == '?') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = ValueVariance {
        .value = glm::vec3(0.f, 0.f, 0.f),
        .variance = glm::vec3(0.f, 0.f, 0.f),
      };
    }
  }
  for (auto [key, value] : additionalFields){
    if (key.size() > 1){
      auto newKey = key.substr(1, key.size());
      if (key.at(0) == '!'){
        values.at(newKey).value = parseVec(value);
      }else if (key.at(0) == '?'){
        values.at(newKey).variance = parseVec(value);
      }else if (key.at(0) == '%'){
        values.at(newKey).lifetimeEffect = parseFloatVec(value);
      }
    }
  }
  std::vector<EmitterDelta> deltas;
  for (auto [key, value] : values){
    deltas.push_back(EmitterDelta{
      .attributeName = key,
      .value = value.value,
      .variance = value.variance,
      .lifetimeEffect = value.lifetimeEffect,
    });
  }
  return deltas;
}

GameObjectEmitter createEmitter(std::function<void(float, float, int, std::map<std::string, std::string>, std::vector<EmitterDelta>)> addEmitter, std::map<std::string, std::string> additionalFields){
  GameObjectEmitter obj {};
  float spawnrate = additionalFields.find("rate") != additionalFields.end() ? std::atof(additionalFields.at("rate").c_str()) : 1.f;
  float lifetime = additionalFields.find("duration") != additionalFields.end() ? std::atof(additionalFields.at("duration").c_str()) : 10.f;
  int limit = additionalFields.find("limit") != additionalFields.end() ? std::atoi(additionalFields.at("limit").c_str()) : 10;
  assert(limit >= 0);
  addEmitter(spawnrate, lifetime, limit, particleFields(additionalFields), emitterDeltas(additionalFields));
  return obj;
}

GameObjectHeightmap createHeightmap(std::map<std::string, std::string> additionalFields, std::function<Mesh(MeshData&)> loadMesh, std::function<Texture(std::string)> ensureTextureLoaded){
  auto mapName = additionalFields.find("map") != additionalFields.end() ? additionalFields.at("map") : "";
  auto dim = additionalFields.find("dim") != additionalFields.end() ? std::atoi(additionalFields.at("dim").c_str()) : -1;
  auto heightmap = loadAndAllocateHeightmap(mapName, dim);
  auto meshData = generateHeightmapMeshdata(heightmap);
  GameObjectHeightmap obj{
    .heightmap = heightmap,
    .mesh = loadMesh(meshData),
    .texture = texinfoFromFields(additionalFields, ensureTextureLoaded),
  };
  return obj;
}

GameObjectNavmesh createNavmesh(Mesh& navmesh){
  GameObjectNavmesh obj {
    .mesh = navmesh,
  };
  return obj;
}

GameObjectNavConns createNavConns(std::map<std::string, std::string>& additionalFields){
  GameObjectNavConns obj {
    .navgraph = createNavGraph(additionalFields),
  };
  return obj;
}

GameObjectUICommon parseCommon(std::map<std::string, std::string>& additionalFields, std::map<std::string, Mesh>& meshes){
  auto onFocus = additionalFields.find("focus") != additionalFields.end() ? additionalFields.at("focus") : "";
  auto onBlur = additionalFields.find("blur") != additionalFields.end() ? additionalFields.at("blur") : "";
  GameObjectUICommon common {
    .mesh = meshes.at("./res/models/controls/input.obj"),
    .isFocused = false,
    .onFocus = onFocus,
    .onBlur = onBlur,
  };
  return common;
}
GameObjectUIButton createUIButton(std::map<std::string, std::string> additionalFields, std::map<std::string, Mesh>& meshes, std::function<Texture(std::string)> ensureTextureLoaded){
  auto onTexture = additionalFields.find("ontexture") != additionalFields.end() ? additionalFields.at("ontexture") : "";
  auto offTexture = additionalFields.find("offtexture") != additionalFields.end() ? additionalFields.at("offtexture") : "";
  auto toggleOn = additionalFields.find("state") != additionalFields.end() && additionalFields.at("state") == "on";
  auto canToggle = additionalFields.find("cantoggle") == additionalFields.end() || !(additionalFields.at("cantoggle") == "false");
  auto onToggleOn = additionalFields.find("on") != additionalFields.end() ? additionalFields.at("on") : "";
  auto onToggleOff = additionalFields.find("off") != additionalFields.end() ? additionalFields.at("off") : "";
  auto hasOnTint  = additionalFields.find("ontint") != additionalFields.end();
  auto onTint = hasOnTint ? parseVec(additionalFields.at("ontint")) : glm::vec3(1.f, 1.f, 1.f);

  GameObjectUIButton obj { 
    .common = parseCommon(additionalFields, meshes),
    .initialState = toggleOn,
    .toggleOn = toggleOn,
    .canToggle = canToggle,
    .onTextureString = onTexture,
    .onTexture = ensureTextureLoaded(onTexture == "" ? "./res/models/controls/on.png" : onTexture).textureId,
    .offTextureString = offTexture,
    .offTexture = ensureTextureLoaded(offTexture == "" ? "./res/models/controls/off.png" : offTexture).textureId,
    .onToggleOn = onToggleOn,
    .onToggleOff = onToggleOff,
    .hasOnTint = hasOnTint,
    .onTint = onTint,
  };
  return obj;
}

GameObjectUISlider createUISlider(std::map<std::string, std::string> additionalFields, std::map<std::string, Mesh>& meshes, std::function<Texture(std::string)> ensureTextureLoaded){
  auto onSlide = additionalFields.find("onslide") != additionalFields.end() ? additionalFields.at("onslide") : "";

  GameObjectUISlider obj {
    .common = parseCommon(additionalFields, meshes),
    .min = 0.f,
    .max = 100.f,
    .percentage = 100.f,
    .texture = ensureTextureLoaded("./res/models/controls/slider.png").textureId,
    .opacityTexture = ensureTextureLoaded("./res/models/controls/slider_opacity.png").textureId,
    .onSlide = onSlide,
  };
  return obj;
}

GameObjectVideo createVideo(
  std::map<std::string, std::string> additionalFields, 
  std::function<Texture(std::string filepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels)> ensureTextureDataLoaded
){
  auto videoPath = additionalFields.at("source");
  auto video = loadVideo(videoPath.c_str());
  std::cout << "INFO: OBJECT TYPE: CREATE VIDEO" << std::endl;

  ensureTextureDataLoaded(
    videoPath,
    video.avFrame2 -> data[0], 
    video.avFrame2 -> width, 
    video.avFrame2 -> height, 
    4
  );
  GameObjectVideo obj {
    .video = video,
    .source = videoPath,
    .sound = createBufferedAudio(),
  };
  return obj;
}

void addObject(
  objid id, 
  std::string objectType, 
  std::map<std::string, std::string> additionalFields,
  std::map<objid, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, 
  std::string defaultMesh, 
  std::function<bool(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<Texture(std::string)> ensureTextureLoaded,
  std::function<Texture(std::string filepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels)> ensureTextureDataLoaded,
  std::function<void()> onVoxelBoundInfoChanged,
  std::function<void(std::string)> loadScene,
  std::function<void(float, float, int, std::map<std::string, std::string>, std::vector<EmitterDelta>)> addEmitter,
  std::function<Mesh(MeshData&)> loadMesh
){
  if (objectType == "default"){
    mapping[id] = createMesh(additionalFields, meshes, defaultMesh, ensureMeshLoaded, ensureTextureLoaded);
  }else if(objectType == "camera"){
    mapping[id] = createCamera();
  }else if (objectType == "portal"){
    mapping[id] = createPortal(additionalFields);
  }else if(objectType == "sound"){
    mapping[id] = createSound(additionalFields);
  }else if(objectType == "light"){
    mapping[id] = createLight(additionalFields);
  }else if(objectType == "voxel"){
    auto defaultVoxelTexture = ensureTextureLoaded("./res/textures/wood.jpg");
    mapping[id] = createVoxel(additionalFields, onVoxelBoundInfoChanged, defaultVoxelTexture.textureId, ensureTextureLoaded);
  }else if(objectType == "channel"){
    mapping[id] = createChannel(additionalFields);
  }else if(objectType == "scene"){
    mapping[id] = createScene(id, additionalFields, loadScene);
  }else if (objectType == "root"){
    mapping[id] = GameObjectRoot{};
  }else if (objectType == "emitter"){
    mapping[id] = createEmitter(addEmitter, additionalFields);
  }else if (objectType == "heightmap"){
    mapping[id] = createHeightmap(additionalFields, loadMesh, ensureTextureLoaded);
  }else if (objectType == "navmesh"){
    mapping[id] = createNavmesh(meshes.at("./res/models/ui/node.obj"));
  }else if (objectType == "navconnection"){
    mapping[id] = createNavConns(additionalFields);
  }else if (objectType == "ui"){
    mapping[id] = createUIButton(additionalFields, meshes, ensureTextureLoaded);
  }else if (objectType == "slider"){
    mapping[id] = createUISlider(additionalFields, meshes, ensureTextureLoaded);
  }else if (objectType == "video"){
    mapping[id] = createVideo(additionalFields, ensureTextureDataLoaded);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
    assert(false);
  }
}
void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void()> rmEmitter
){
  // @TODO - handle resource cleanup better here eg unload meshes
  auto Object = mapping.at(id); 
  auto soundObj = std::get_if<GameObjectSound>(&Object);
  if (soundObj != NULL){
    unloadSoundState(soundObj -> source, soundObj -> clip); 
  }

  auto sceneObj = std::get_if<GameObjectScene>(&Object);
  if (sceneObj != NULL){
    std::cout << "ERROR: scene - remove scene obj not yet implemented" << std::endl;
    assert(false);
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&Object);
  if (emitterObj != NULL){
    rmEmitter();
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&Object);
  if (heightmapObj !=NULL){
    delete[] heightmapObj -> heightmap.data;
  }

  auto videoObj = std::get_if<GameObjectVideo>(&Object);
  if (videoObj != NULL){
    freeVideoContent(videoObj -> video);
    freeBufferedAudio(videoObj -> sound);
  }

  mapping.erase(id);
}

void renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping, 
  Mesh& nodeMesh,
  Mesh& cameraMesh,
  Mesh& portalMesh, 
  Mesh& voxelCubeMesh,
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform,
  unsigned int portalTexture,
  glm::mat4 model
){
  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);

  if (meshObj != NULL && !meshObj -> isDisabled && !meshObj ->nodeOnly){
    for (auto meshToRender : meshObj -> meshesToRender){
      bool hasBones = false;
      if (meshToRender.bones.size() > 0){
        for (int i = 0; i < 100; i++){
          auto boneUniformLocation = glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str());
          if (i >= meshToRender.bones.size()){
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
          }else{
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(meshToRender.bones.at(i).offsetMatrix));
          }
        }
        hasBones = true;
      }

      glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), showBoneWeight);
      glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), useBoneTransform);
      glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), hasBones);    

      glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(meshObj -> texture.textureoffset));
      glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(meshObj -> texture.texturetiling));
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(meshObj -> tint));
      drawMesh(meshToRender, shaderProgram, meshObj -> texture.textureOverloadId);    
    }
    return;
  }

  if (meshObj != NULL && meshObj -> nodeOnly && showDebug) {
    glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);   
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(meshObj -> texture.textureoffset));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(meshObj -> texture.texturetiling));
    drawMesh(nodeMesh, shaderProgram, meshObj -> texture.textureOverloadId);    
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), cameraMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(cameraMesh, shaderProgram);
    return;
  }

  auto soundObject = std::get_if<GameObjectSound>(&toRender);
  if (soundObject != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);    
    return;
  }

  auto portalObj = std::get_if<GameObjectPortal>(&toRender);
  if (portalObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(portalMesh, shaderProgram, portalTexture);
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showDebug){   
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);
    return;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), voxelCubeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));

    auto voxelBodies = getVoxelBodies(voxelObj -> voxel);
    for (int i = 0; i < voxelBodies.size(); i++){
      auto voxelBody = voxelBodies.at(i);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, voxelBody.position + glm::vec3(0.5f, 0.5f, 0.5f))));
      drawMesh(voxelCubeMesh, shaderProgram, voxelBody.textureId);    
    }
    return;
  }

  auto channelObj = std::get_if<GameObjectChannel>(&toRender);
  if (channelObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);
    return;
  }

  auto rootObj = std::get_if<GameObjectRoot>(&toRender);
  if (rootObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);
    return;
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);  
    return; 
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&toRender);
  if (heightmapObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(heightmapObj -> texture.textureoffset));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(heightmapObj -> texture.texturetiling));
    drawMesh(heightmapObj -> mesh, shaderProgram, heightmapObj -> texture.textureOverloadId);   
    return;
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&toRender);
  if (navmeshObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);
    return;
  }

  auto navconnObj = std::get_if<GameObjectNavConns>(&toRender);
  if (navconnObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram); 

    auto navPoints = aiAllPoints(navconnObj -> navgraph);

    for (auto navPoint : navPoints){
      std::cout << "points: " << print(navPoint.fromPoint) << ", " << print(navPoint.toPoint) << std::endl;
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
      glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(
          glm::scale(glm::translate(glm::mat4(1.0f), navPoint.fromPoint), glm::vec3(10.f, 1.f, 10.f))
        )
      );
      drawMesh(nodeMesh, shaderProgram);

      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
      glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(
          glm::scale(glm::translate(glm::mat4(1.0f), navPoint.toPoint), glm::vec3(5.f, 2.f, 10.f))
        )
      );
      drawMesh(nodeMesh, shaderProgram);
    }
    std::cout << std::endl;
    return;
  }

  auto uiObj = std::get_if<GameObjectUIButton>(&toRender);
  if (uiObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    if (uiObj -> hasOnTint && uiObj -> toggleOn){
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(uiObj -> onTint));
    }
    auto textureOverloadId = uiObj -> toggleOn ? uiObj -> onTexture : uiObj -> offTexture;
    drawMesh(uiObj -> common.mesh, shaderProgram, textureOverloadId); 
    return;   
  }

  auto uiSliderObj = std::get_if<GameObjectUISlider>(&toRender);
  if (uiSliderObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "discardTexAmount"), 1 - uiSliderObj -> percentage);  
    drawMesh(uiSliderObj -> common.mesh, shaderProgram, uiSliderObj -> texture, uiSliderObj -> opacityTexture);  
    return;  
  }

  auto videoObj = std::get_if<GameObjectVideo>(&toRender);
  if (videoObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram);   
    return;
  }
}

std::map<std::string, std::string> objectAttributes(std::map<objid, GameObjectObj>& mapping, objid id){
  std::map<std::string, std::string> attributes;

  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (meshObj -> meshNames.size() > 0){
      attributes["mesh"] = meshObj -> meshNames.at(0);
    }
    attributes["isDisabled"] = meshObj -> isDisabled ? "true": "false";
    return attributes;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL){
    return attributes;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL){   
    attributes["color"] = print(lightObj -> color);
    return attributes;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    // not yet implemented
    assert(false);
    return attributes;
  }

  auto soundObj = std::get_if<GameObjectSound>(&toRender);
  if (soundObj != NULL){
    attributes["clip"] = soundObj -> clip;
    return attributes;
  }

  auto channelObj = std::get_if<GameObjectChannel>(&toRender);
  if (channelObj != NULL){
    attributes["from"] = channelObj -> from;
    attributes["to"] = channelObj -> to;
    return attributes;
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL){
    assert(false);
    return attributes;
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&toRender);
  if (heightmapObj != NULL){
    assert(false);
    return attributes;
  }

  assert(false);
  return attributes;
}

void setObjectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, std::map<std::string, std::string> attributes){
 GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (attributes.find("isDisabled") != attributes.end()){
      meshObj -> isDisabled = attributes.at("isDisabled") == "true";;
    }
    return;
  }

  auto cameraObj = std::get_if<GameObjectChannel>(&toRender);
  if (cameraObj != NULL){
    if (attributes.find("to") != attributes.end()){
      cameraObj -> to = attributes.at("to");
    }
    if (attributes.find("from") != attributes.end()){
      cameraObj -> from = attributes.at("from");
    }
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL){   
    lightObj -> color = parseVec(attributes.at("color"));
    return;
  }
  assert(false);
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.rootMesh != ""){
    pairs.push_back(std::pair<std::string, std::string>("mesh", obj.rootMesh));
  }
  if (obj.isDisabled){
    pairs.push_back(std::pair<std::string, std::string>("disabled", "true"));
  }
  if (obj.texture.textureoffset.x != 0.f && obj.texture.textureoffset.y != 0.f){
    pairs.push_back(std::pair<std::string, std::string>("textureoffset", serializeVec(obj.texture.textureoffset)));
  }
  if (obj.texture.textureOverloadName != ""){
    pairs.push_back(std::pair<std::string, std::string>("texture", obj.texture.textureOverloadName));
  }
  if (!isIdentityVec(obj.tint)){
    pairs.push_back(std::pair<std::string, std::string>("tint", serializeVec(obj.tint)));
  }

  return pairs;  
}
std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj){
  return {}; 
}  
std::vector<std::pair<std::string, std::string>> serializeSound(GameObjectSound obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.clip != ""){
    pairs.push_back(std::pair<std::string, std::string>("clip", obj.clip));
  }
  return pairs;
}   
std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back(std::pair<std::string, std::string>("color", serializeVec(obj.color)));
  return pairs;
}  
std::vector<std::pair<std::string, std::string>> serializeVoxel(GameObjectVoxel obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back(std::pair<std::string, std::string>("from", serializeVoxelState(obj.voxel)));
  return pairs;
}  
std::vector<std::pair<std::string, std::string>> serializeChannel(GameObjectChannel obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.from != ""){
    pairs.push_back(std::pair<std::string, std::string>("from", obj.from));
  }
  if (obj.to != ""){
    pairs.push_back(std::pair<std::string, std::string>("to", obj.to));
  }
  return pairs;
}

void addSerializeCommon(std::vector<std::pair<std::string, std::string>>& pairs, GameObjectUICommon& common){
  if (common.onFocus != ""){
    pairs.push_back(std::pair<std::string, std::string>("focus", common.onFocus));
  }
  if (common.onBlur != ""){
    pairs.push_back(std::pair<std::string, std::string>("blur", common.onBlur));
  }
}
std::vector<std::pair<std::string, std::string>> serializeButton(GameObjectUIButton obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  addSerializeCommon(pairs, obj.common);
  if (obj.canToggle != true){
    pairs.push_back(std::pair<std::string, std::string>("cantoggle", "false"));
  }
  if (obj.onTextureString != ""){
    pairs.push_back(std::pair<std::string, std::string>("ontexture", obj.onTextureString));
  }
  if (obj.offTextureString != ""){
    pairs.push_back(std::pair<std::string, std::string>("offtexture", obj.offTextureString));
  }
  if (obj.onToggleOn != ""){
    pairs.push_back(std::pair<std::string, std::string>("on", obj.onToggleOn));
  }
  if (obj.onToggleOff != ""){
    pairs.push_back(std::pair<std::string, std::string>("off", obj.onToggleOff));
  }
  if (obj.initialState == true){
    pairs.push_back(std::pair<std::string, std::string>("state", "on"));
  }
  return pairs;
}
std::vector<std::pair<std::string, std::string>> serializeSlider(GameObjectUISlider obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  addSerializeCommon(pairs, obj.common);
  if (obj.onSlide != ""){
    pairs.push_back(std::pair<std::string, std::string>("onslide", obj.onSlide));
  }
  return pairs;
}

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping){
  GameObjectObj objectToSerialize = mapping.at(id);
  auto meshObject = std::get_if<GameObjectMesh>(&objectToSerialize);
  if (meshObject != NULL){
    return serializeMesh(*meshObject);
  }
  auto cameraObject = std::get_if<GameObjectCamera>(&objectToSerialize);
  if (cameraObject != NULL){
    return serializeCamera(*cameraObject);
  }
  auto soundObject = std::get_if<GameObjectSound>(&objectToSerialize);
  if (soundObject != NULL){
    return serializeSound(*soundObject);
  }
  auto lightObject = std::get_if<GameObjectLight>(&objectToSerialize);
  if (lightObject != NULL){
    return serializeLight(*lightObject);
  }
  auto voxelObject = std::get_if<GameObjectVoxel>(&objectToSerialize);
  if (voxelObject != NULL){
    return serializeVoxel(*voxelObject);
  }
  auto channelObject = std::get_if<GameObjectChannel>(&objectToSerialize);
  if (channelObject != NULL){
    return serializeChannel(*channelObject);
  }

  auto rootObject = std::get_if<GameObjectRoot>(&objectToSerialize);
  if (rootObject != NULL){
    return {};
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&objectToSerialize);
  if (emitterObj != NULL){
    std::cout << "ERROR: EMITTER SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&objectToSerialize);
  if (heightmapObj != NULL){
    std::cout << "ERROR: HEIGHTMAP SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&objectToSerialize);
  if (navmeshObj != NULL){
    std::cout << "ERROR: NAVMESH SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto navconnObj = std::get_if<GameObjectNavConns>(&objectToSerialize);
  if (navconnObj != NULL){
    std::cout << "ERROR: NAVCONN SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto uiControlObj = std::get_if<GameObjectUIButton>(&objectToSerialize);
  if (uiControlObj != NULL){
    return serializeButton(*uiControlObj);
  }
  
  auto uiControlSliderObj = std::get_if<GameObjectUISlider>(&objectToSerialize);
  if (uiControlSliderObj != NULL){
    std::cout << "ERROR: UI SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return serializeSlider(*uiControlSliderObj);    
  }

  auto uiVideoObj = std::get_if<GameObjectVideo>(&objectToSerialize);
  if (uiVideoObj != NULL){
    std::cout << "ERROR: VIDEO SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto rootObj = std::get_if<GameObjectRoot>(&objectToSerialize);
  if (rootObj != NULL){
    return {};
  }

  //assert(false);  
  return {};
}


std::vector<objid> getGameObjectsIndex(std::map<objid, GameObjectObj>& mapping){
  std::vector<objid> indicies;
  for (auto [id, _]: mapping){    
      indicies.push_back(id);
  }
  return indicies;
}

NameAndMesh getMeshesForId(std::map<objid, GameObjectObj>& mapping, objid id){  
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;

  GameObjectObj& gameObj = mapping.at(id);
  auto meshObject = std::get_if<GameObjectMesh>(&gameObj);

  if (meshObject != NULL){
    for (int i = 0; i < meshObject -> meshesToRender.size(); i++){
      meshNames.push_back(meshObject -> meshNames.at(i));
      meshes.push_back(meshObject -> meshesToRender.at(i));
    }
  }

  NameAndMesh meshData {
    .meshNames = meshNames,
    .meshes = meshes
  };

  assert(meshNames.size() == meshes.size());
  return meshData;
}

std::vector<std::string> getMeshNames(std::map<objid, GameObjectObj>& mapping, objid id){
  std::vector<std::string> names;
  if (id == -1){
    return names;
  }
  for (auto name : getMeshesForId(mapping, id).meshNames){
    names.push_back(name);
  }

  return names;
}

std::map<std::string, std::vector<std::string>> getChannelMapping(std::map<objid, GameObjectObj>& mapping){
  std::map<std::string, std::vector<std::string>> channelMapping;
  for (auto &[_, obj] : mapping){
    auto channelObj = std::get_if<GameObjectChannel>(&obj);
    if (channelObj != NULL && channelObj -> complete){
      std::vector<std::string> toChannels;
      if (channelMapping.find(channelObj -> from) == channelMapping.end()){
        channelMapping[channelObj -> from] = toChannels;
      }
      channelMapping[channelObj -> from].push_back(channelObj -> to);   
    } 
  }
  return channelMapping;
}

std::map<objid, GameObjectHeightmap*> getHeightmaps(std::map<objid, GameObjectObj>& mapping){
  std::map<objid, GameObjectHeightmap*> maps;
  for (auto &[id, obj] : mapping){
    auto heightmap = std::get_if<GameObjectHeightmap>(&obj);
    if (heightmap != NULL){
      maps[id] = heightmap;
    }
  }
  return maps;  
}

bool isNavmesh(std::map<objid, GameObjectObj>& mapping, objid id){
  auto object = mapping.at(id); 
  auto navmesh = std::get_if<GameObjectNavmesh>(&object);
  return navmesh != NULL;
}

std::optional<Texture> textureForId(std::map<objid, GameObjectObj>& mapping, objid id){
  auto Object = mapping.at(id); 
  auto heightmapObj = std::get_if<GameObjectHeightmap>(&Object);
  if (heightmapObj !=NULL){
    return heightmapObj -> mesh.texture;
  }

  auto meshObj = std::get_if<GameObjectMesh>(&Object);
  if (meshObj != NULL){
    for (int i = 0; i < meshObj -> meshNames.size(); i++){
      if (meshObj -> meshNames.at(i) == meshObj -> rootMesh){
        return meshObj -> meshesToRender.at(i).texture;
      }
    }
    std::cout << "WARNING: " << id << " is mesh obj and does not have a mesh" << std::endl;
  }
  return std::nullopt;
}

void applyFocusUI(std::map<objid, GameObjectObj>& mapping, objid id, std::function<void(std::string)> sendNotify){
  for (auto &[uiId, obj] : mapping){
    auto uiControl = std::get_if<GameObjectUIButton>(&obj);
    if (uiControl != NULL){
      if (id == uiId && uiControl -> canToggle){
        uiControl -> toggleOn = !uiControl -> toggleOn;
        if (uiControl -> toggleOn && uiControl -> onToggleOn != ""){
          sendNotify(uiControl -> onToggleOn);
        }else if (uiControl -> onToggleOff != ""){
          sendNotify(uiControl -> onToggleOff);
        }
      }

      if (uiControl -> common.isFocused && id != uiId){

        std::cout << "id: " << id << " is now not focused" << std::endl;
        uiControl -> common.isFocused = false;
        if (uiControl -> common.onBlur != ""){
          sendNotify(uiControl -> common.onBlur);
        }
      }

      if (!uiControl -> common.isFocused && id == uiId){
        std::cout << "id: " << id << " is now focused" << std::endl;
        uiControl -> common.isFocused = true;
        if (uiControl -> common.onFocus != ""){
          sendNotify(uiControl -> common.onFocus);
        }
      }
    }
  }
}
void applyKey(std::map<objid, GameObjectObj>& mapping, char key, std::function<void(std::string)> applyText){
  /*for (auto &[uiId, obj] : mapping){
    auto uiControl = std::get_if<GameObjectUI>(&obj);
    if (uiControl != NULL && uiControl -> isFocused){
      auto oldText = uiControl -> text;
      uiControl -> text = oldText + key;
      applyText(uiControl -> text);
    }
  }*/
}

void applyUICoord(std::map<objid, GameObjectObj>& mapping, std::function<void(std::string, float)> onSliderPercentage, objid id, float uvx, float uvy){
  for (auto &[uiId, obj] : mapping){
    auto uiControl = std::get_if<GameObjectUISlider>(&obj);
    if (uiControl != NULL && uiId == id){
      uiControl -> percentage = uvx;
      if (uiControl -> onSlide != ""){
        onSliderPercentage(uiControl -> onSlide, uiControl -> percentage);
      }
    }
  }
}

void updatePosition(std::map<objid, GameObjectObj>& mapping, objid id, glm::vec3 position){
  auto object = mapping.at(id); 
  auto soundObj = std::get_if<GameObjectSound>(&object);
  if (soundObj != NULL){
    setSoundPosition(soundObj -> source, position.x, position.y, position.z);
  }
}

void playSoundState(std::map<objid, GameObjectObj>& mapping, objid id){
  auto object = mapping.at(id);
  auto soundObj = std::get_if<GameObjectSound>(&object);
  if (soundObj != NULL){
    playSource(soundObj -> source);
  }else{
    std::cout << "WARNING: " << id << " is not a sound object" << std::endl;
  }
}

void processVideoFrame(GameObjectVideo* videoObj, std::function<void(std::string texturepath, unsigned char* data, int textureWidth, int textureHeight)> updateTextureData){
  int stream = nextFrame(videoObj -> video);
  if (stream == videoObj -> video.streamIndexs.video){
    updateTextureData( 
      videoObj -> source,
      videoObj -> video.avFrame2 -> data[0], 
      videoObj -> video.avFrame2 -> width, 
      videoObj -> video.avFrame2 -> height
    );
  }else if (stream == videoObj -> video.streamIndexs.audio){
    auto audioCodec = videoObj -> video.codecs.audioCodec;
    auto bufferSize = av_samples_get_buffer_size(NULL, audioCodec -> channels, videoObj -> video.avFrame -> nb_samples, audioCodec -> sample_fmt, 0);
    auto numChannels = audioCodec -> channels;

    /*uint8_t* bufferData = new uint8_t[bufferSize];
    //https://stackoverflow.com/questions/21386135/ffmpeg-openal-playback-streaming-sound-from-video-wont-work

    std::cout << "num channels: " << numChannels << std::endl;
    // @TODO process all channels
    // @TODO chandle more formats to eliminate assertion below 
    std::cout << "fmt name: " << av_get_sample_fmt_name(audioCodec -> sample_fmt) << std::endl;;
    assert(audioCodec -> sample_fmt == AV_SAMPLE_FMT_S16);
   


    playBufferedAudio(videoObj -> sound, videoObj -> video.avFrame -> data[0], bufferSize, audioCodec -> sample_rate);*/
  }
}

void onObjectFrame(std::map<objid, GameObjectObj>& mapping, std::function<void(std::string texturepath, unsigned char* data, int textureWidth, int textureHeight)> updateTextureData, float timestamp){
  for (auto &[_, obj] : mapping){
    auto videoObj = std::get_if<GameObjectVideo>(&obj);
    if (videoObj != NULL){
      while (true){
        bool processedToCurrentTime =  videoObj -> video.videoTimestamp > (timestamp + 10);
        if (processedToCurrentTime){
          break;
        }
        processVideoFrame(videoObj, updateTextureData);
      }
    }
  }
}