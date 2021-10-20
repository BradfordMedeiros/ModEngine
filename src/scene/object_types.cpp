#include "./object_types.h"

std::map<objid, GameObjectObj> getObjectMapping() {
	std::map<objid, GameObjectObj> objectMapping;
	return objectMapping;
}

TextureInformation texinfoFromFields(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded){
  glm::vec2 textureoffset = attr.stringAttributes.find("textureoffset") == attr.stringAttributes.end() ? glm::vec2(0.f, 0.f) : parseVec2(attr.stringAttributes.at("textureoffset"));
  glm::vec2 texturetiling = attr.stringAttributes.find("texturetiling") == attr.stringAttributes.end() ? glm::vec2(1.f, 1.f) : parseVec2(attr.stringAttributes.at("texturetiling"));
  glm::vec2 texturesize = attr.stringAttributes.find("texturesize") == attr.stringAttributes.end() ? glm::vec2(1.f, 1.f) : parseVec2(attr.stringAttributes.at("texturesize"));
  std::string textureOverloadName = attr.stringAttributes.find("texture") == attr.stringAttributes.end() ? "" : attr.stringAttributes.at("texture");
  int textureOverloadId = textureOverloadName == "" ? -1 : ensureTextureLoaded(textureOverloadName).textureId;

  TextureInformation info {
    .textureoffset = textureoffset,
    .texturetiling = texturetiling,
    .texturesize = texturesize,
    .textureOverloadName = textureOverloadName,
    .textureOverloadId = textureOverloadId,
  };
  return info;
}

static std::vector<std::string> meshFieldsToCopy = { "textureoffset", "texturetiling", "texturesize", "texture", "discard", "emission", "tint" };
GameObjectMesh createMesh(
  GameobjAttributes& attr, 
  std::map<std::string, MeshRef>& meshes, 
  std::function<std::vector<std::string>(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<Texture(std::string)> ensureTextureLoaded
){
  // get rid of meshes attribute completely, make ensuremeshloaded return the meshes you're actually responsible for
  // basically top level ensureMesh(attr("mesh") => your nodes, then the child ones can be logic'd in via being smart about ensureMeshLoaded :) 
  std::string rootMeshName = attr.stringAttributes.find("mesh") == attr.stringAttributes.end()  ? "" : attr.stringAttributes.at("mesh");
  auto meshNamesForObj = ensureMeshLoaded(rootMeshName, meshFieldsToCopy);
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;
  for (auto meshName : meshNamesForObj){
    std::cout << "trying to get mesh name: " << meshName << std::endl;
    meshNames.push_back(meshName);
    meshesToRender.push_back(meshes.at(meshName).mesh);  
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .isDisabled = attr.stringAttributes.find("disabled") != attr.stringAttributes.end(),
    .nodeOnly = meshNames.size() == 0,
    .rootMesh = rootMeshName,
    .texture = texinfoFromFields(attr, ensureTextureLoaded),
    .discardAmount = attr.numAttributes.find("discard") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("discard"),
    .emissionAmount = attr.numAttributes.find("emission") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("emission"),
    .tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint"),
  };
  return obj;
}

GameObjectVoxel createVoxel(GameobjAttributes& attr, std::function<void()> onVoxelBoundInfoChanged, unsigned int defaultTexture, std::function<Texture(std::string)> ensureTextureLoaded){
  auto textureString = attr.stringAttributes.find("fromtextures") == attr.stringAttributes.end() ? "" : attr.stringAttributes.at("fromtextures");
  auto voxel = createVoxels(parseVoxelState(attr.stringAttributes.at("from"), textureString, defaultTexture, ensureTextureLoaded), onVoxelBoundInfoChanged, defaultTexture);
  GameObjectVoxel obj {
    .voxel = voxel,
  };
  return obj;
}

GameobjAttributes particleFields(GameobjAttributes& attributes){
  GameobjAttributes attr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttributes = {},
  };
  for (auto [key, value] : attributes.stringAttributes){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.stringAttributes[newKey] = value;
    }
  }
  for (auto [key, value] : attributes.numAttributes){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.numAttributes[newKey] = value;
    }
  }
  for (auto [key, value] : attributes.vecAttributes){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.vecAttributes[newKey] = value;
    }
  }
  return attr;
}

struct ValueVariance {
  glm::vec3 value;
  glm::vec3 variance;
  std::vector<float> lifetimeEffect;
};
std::vector<EmitterDelta> emitterDeltas(GameobjAttributes& attributes){
  std::map<std::string, ValueVariance> values;
  for (auto [key, _] : attributes.vecAttributes){
    if ((key.at(0) == '!' || key.at(0) == '?' || key.at(0) == '%') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = ValueVariance {
        .value = glm::vec3(0.f, 0.f, 0.f),
        .variance = glm::vec3(0.f, 0.f, 0.f),
        .lifetimeEffect = {},
      };
    }
  }
  
  for (auto [key, value] : attributes.vecAttributes){
    if (key.size() > 1){
      auto newKey = key.substr(1, key.size());
      if (key.at(0) == '!'){
        values.at(newKey).value = value;
      }else if (key.at(0) == '?'){
        values.at(newKey).variance = value;
      }
      /*else if (key.at(0) == '%'){
        values.at(newKey).lifetimeEffect = parseFloatVec(value);
      }*/
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

GameObjectEmitter createEmitter(std::function<void(float, float, int, GameobjAttributes& attributes, std::vector<EmitterDelta>, bool, EmitterDeleteBehavior)> addEmitter, GameobjAttributes& attributes){
  GameObjectEmitter obj {};
  float spawnrate = attributes.numAttributes.find("rate") != attributes.numAttributes.end() ? attributes.numAttributes.at("rate") : 1.f;
  float lifetime = attributes.numAttributes.find("duration") != attributes.numAttributes.end() ? attributes.numAttributes.at("duration") : 10.f;
  int limit = attributes.numAttributes.find("limit") != attributes.numAttributes.end() ? attributes.numAttributes.at("limit") : 10;
  auto enabled = attributes.stringAttributes.find("state") != attributes.stringAttributes.end() ? !(attributes.stringAttributes.at("state") == "disabled") : true;
  assert(limit >= 0);
  
  auto deleteValueStr = attributes.stringAttributes.find("onremove") != attributes.stringAttributes.end() ? attributes.stringAttributes.at("onremove") : "delete";
  auto deleteType = EMITTER_DELETE;
  if (deleteValueStr == "orphan"){
    deleteType = EMITTER_ORPHAN;
  }
  if (deleteValueStr == "finish"){
    deleteType = EMITTER_FINISH;
  }

  auto emitterAttr = particleFields(attributes);
  addEmitter(spawnrate, lifetime, limit, emitterAttr, emitterDeltas(attributes), enabled, deleteType);
  return obj;
}

GameObjectHeightmap createHeightmap(GameobjAttributes& attr, std::function<Mesh(MeshData&)> loadMesh, std::function<Texture(std::string)> ensureTextureLoaded){
  auto mapName = attr.stringAttributes.find("map") != attr.stringAttributes.end() ? attr.stringAttributes.at("map") : "";
  auto dim = attr.numAttributes.find("dim") != attr.numAttributes.end() ? attr.numAttributes.at("dim") : -1;
  auto heightmap = loadAndAllocateHeightmap(mapName, dim);
  auto meshData = generateHeightmapMeshdata(heightmap);
  GameObjectHeightmap obj{
    .heightmap = heightmap,
    .mesh = loadMesh(meshData),
    .texture = texinfoFromFields(attr, ensureTextureLoaded),
  };
  return obj;
}

GameObjectNavmesh createNavmesh(Mesh& navmesh){
  GameObjectNavmesh obj {
    .mesh = navmesh,
  };
  return obj;
}

std::size_t getVariantIndex(GameObjectObj gameobj){
  return gameobj.index();
}

void nothingObjAttr(GameObjectObj& obj, GameobjAttributes& _attributes){ }// do nothing 


template<typename T>
std::function<void(GameObjectObj& obj, GameobjAttributes& attr)> convertElementValue(std::function<void(T&, GameobjAttributes&)> getAttr) {   
  return [&getAttr](GameObjectObj& obj, GameobjAttributes& attr) -> void {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    getAttr(*objInstance, attr);
  };
}

//  std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj&)> serialize;
template<typename T>
std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj& obj)> convertSerialize(std::function<void(T&)> serialize) {   
  return [](GameObjectObj& obj) -> std::vector<std::pair<std::string, std::string>> {
    return {};
  };
}

std::vector<std::pair<std::string, std::string>> serializeNotImplemented(GameObjectObj& obj){
  std::cout << "ERROR: GEO SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
  assert(false);
  return {};    
}

std::vector<ObjectType> objTypes = {
  ObjectType {
    .name = "geo",
    .variantType = getVariantIndex(GameObjectGeo{}),
    .createObj = createGeo,
    .objectAttributes = convertElementValue<GameObjectGeo>(geoObjAttr),
    .serialize = serializeNotImplemented,
  },
  ObjectType {
    .name = "camera",
    .variantType = getVariantIndex(GameObjectCamera{}),
    .createObj = createCamera,
    .objectAttributes = nothingObjAttr,
    .serialize = serializeNotImplemented,
  },
  ObjectType {
    .name = "portal",
    .variantType = getVariantIndex(GameObjectPortal{}),
    .createObj = createPortal,
    .objectAttributes = nothingObjAttr,
    .serialize = serializeNotImplemented,
  },
  ObjectType {
    .name = "light",
    .variantType = getVariantIndex(GameObjectLight{}),
    .createObj = createLight,
    .objectAttributes = convertElementValue<GameObjectLight>(lightObjAttr),
    .serialize = convertSerialize<GameObjectLight>(serializeLight),
  },
  ObjectType {
    .name = "sound",
    .variantType = getVariantIndex(GameObjectSound{}),
    .createObj = createSound,
    .objectAttributes = convertElementValue<GameObjectSound>(soundObjAttr),
    .serialize = convertSerialize<GameObjectSound>(serializeSound),
  },
  ObjectType {
    .name = "text",
    .variantType = getVariantIndex(GameObjectUIText{}),
    .createObj = createUIText,
    .objectAttributes = convertElementValue<GameObjectUIText>(textObjAttributes),
    .serialize = serializeNotImplemented,
  },
  ObjectType {
    .name = "layout",
    .variantType = getVariantIndex(GameObjectUILayout{}),
    .createObj = createUILayout,
    .objectAttributes = nothingObjAttr,
    .serialize = serializeNotImplemented,
  },
  ObjectType {
    .name = "navconnection",
    .variantType = getVariantIndex(GameObjectNavConns{}),
    .createObj = createNavConns,
    .objectAttributes = nothingObjAttr,
    .serialize = serializeNotImplemented,
  },
  ObjectType {
    .name = "ui",
    .variantType = getVariantIndex(GameObjectUIButton{}),
    .createObj = createUIButton,
    .objectAttributes = nothingObjAttr,
    .serialize = convertSerialize<GameObjectUIButton>(serializeButton),
  },
  ObjectType {
    .name = "slider",
    .variantType = getVariantIndex(GameObjectUISlider{}),
    .createObj = createUISlider,
    .objectAttributes = nothingObjAttr, 
    .serialize = convertSerialize<GameObjectUISlider>(serializeSlider),
  },
};


void addObject(
  objid id, 
  std::string objectType, 
  GameobjAttributes& attr,
  std::map<objid, GameObjectObj>& mapping, 
  std::map<std::string, MeshRef>& meshes, 
  std::function<std::vector<std::string>(std::string, std::vector<std::string>)> ensureMeshLoaded,
  std::function<Texture(std::string)> ensureTextureLoaded,
  std::function<Texture(std::string filepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels)> ensureTextureDataLoaded,
  std::function<void()> onCollisionChange,
  std::function<void(float, float, int, GameobjAttributes&, std::vector<EmitterDelta>, bool, EmitterDeleteBehavior)> addEmitter,
  std::function<Mesh(MeshData&)> loadMesh
){
  ObjectTypeUtil util {
    .meshes = meshes,
    .ensureTextureLoaded = ensureTextureLoaded,
  };
  for (auto &objType : objTypes){
    if (objectType == objType.name){
      mapping[id] = objType.createObj(attr, util);
      return;
    }
  }

  if (objectType == "default"){
    mapping[id] = createMesh(attr, meshes, ensureMeshLoaded, ensureTextureLoaded);
  }else if(objectType == "voxel"){
    auto defaultVoxelTexture = ensureTextureLoaded("./res/textures/wood.jpg");
    mapping[id] = createVoxel(attr, onCollisionChange, defaultVoxelTexture.textureId, ensureTextureLoaded);
  }else if (objectType == "root"){
    mapping[id] = GameObjectRoot{};
  }else if (objectType == "emitter"){
    mapping[id] = createEmitter(addEmitter, attr);
  }else if (objectType == "heightmap"){
    mapping[id] = createHeightmap(attr, loadMesh, ensureTextureLoaded);
  }else if (objectType == "navmesh"){
    mapping[id] = createNavmesh(meshes.at("./res/models/ui/node.obj").mesh);
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

  auto emitterObj = std::get_if<GameObjectEmitter>(&Object);
  if (emitterObj != NULL){
    rmEmitter();
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&Object);
  if (heightmapObj !=NULL){
    delete[] heightmapObj -> heightmap.data;
  }

  mapping.erase(id);
}

int renderDefaultNode(GLint shaderProgram, Mesh& mesh){
  // Transformation getTransformationFromMatrix(glm::mat4 matrix){
  // unscale this model matrix
  glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), mesh.bones.size() > 0);
  glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
  glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
  glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
  drawMesh(mesh, shaderProgram);
  return mesh.numTriangles;
}

int renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping, 
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform,
  unsigned int portalTexture,
  glm::mat4 model,
  bool drawPoints,
  std::function<void(GLint, objid, std::string, unsigned int, float)> drawWord,
  std::function<int(glm::vec3)> drawSphere,
  DefaultMeshes& defaultMeshes
){
  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);

  if (meshObj != NULL && !meshObj -> isDisabled && !meshObj ->nodeOnly){
    int numTriangles = 0;
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
      glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(meshObj -> texture.texturesize));
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(meshObj -> tint));
      drawMesh(meshToRender, shaderProgram, meshObj -> texture.textureOverloadId, -1, drawPoints);   
      numTriangles = numTriangles + meshToRender.numTriangles; 
    }
    return numTriangles;
  }

  if (meshObj != NULL && meshObj -> nodeOnly && showDebug) {
    glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);   
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(meshObj -> texture.textureoffset));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(meshObj -> texture.texturetiling));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(meshObj -> texture.texturesize));
    drawMesh(*defaultMeshes.nodeMesh, shaderProgram, meshObj -> texture.textureOverloadId);    
    return defaultMeshes.nodeMesh -> numTriangles;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, *defaultMeshes.cameraMesh);
  }

  auto soundObject = std::get_if<GameObjectSound>(&toRender);
  if (soundObject != NULL && showDebug){
    return renderDefaultNode(shaderProgram, *defaultMeshes.soundMesh);
  }

  auto portalObj = std::get_if<GameObjectPortal>(&toRender);
  if (portalObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), defaultMeshes.nodeMesh -> bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(*defaultMeshes.portalMesh, shaderProgram, portalTexture);
    return defaultMeshes.portalMesh -> numTriangles;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showDebug){   
    return renderDefaultNode(shaderProgram, *defaultMeshes.lightMesh);
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), defaultMeshes.voxelCubeMesh -> bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));

    auto voxelBodies = getVoxelBodies(voxelObj -> voxel);

    int numTriangles = 0;
    for (int i = 0; i < voxelBodies.size(); i++){
      auto voxelBody = voxelBodies.at(i);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, voxelBody.position + glm::vec3(0.5f, 0.5f, 0.5f))));
      drawMesh(*defaultMeshes.voxelCubeMesh, shaderProgram, voxelBody.textureId);   
      numTriangles = numTriangles = defaultMeshes.voxelCubeMesh -> numTriangles; 
    }
    return numTriangles;
  }

  auto rootObj = std::get_if<GameObjectRoot>(&toRender);
  if (rootObj != NULL){
    return 0;
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL && showDebug){
    std::cout << "rendering emitter" << std::endl;
    return renderDefaultNode(shaderProgram, *defaultMeshes.emitter);
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&toRender);
  if (heightmapObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(heightmapObj -> texture.textureoffset));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(heightmapObj -> texture.texturetiling));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(heightmapObj -> texture.texturesize));
    drawMesh(heightmapObj -> mesh, shaderProgram, heightmapObj -> texture.textureOverloadId);   
    return heightmapObj -> mesh.numTriangles;
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&toRender);
  if (navmeshObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, *defaultMeshes.nav);
  }

  auto navconnObj = std::get_if<GameObjectNavConns>(&toRender);
  if (navconnObj != NULL && showDebug){
    int numTriangles = 0;

    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(*defaultMeshes.nav, shaderProgram); 
    numTriangles = numTriangles + defaultMeshes.nav -> numTriangles;

    auto navPoints = aiAllPoints(navconnObj -> navgraph);

    for (auto navPoint : navPoints){
      std::cout << "points: " << print(navPoint.fromPoint) << ", " << print(navPoint.toPoint) << std::endl;
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
      glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(
          glm::scale(glm::translate(glm::mat4(1.0f), navPoint.fromPoint), glm::vec3(10.f, 1.f, 10.f))
        )
      );
      drawMesh(*defaultMeshes.nav, shaderProgram);
      numTriangles = numTriangles + defaultMeshes.nav -> numTriangles;

      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
      glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(
          glm::scale(glm::translate(glm::mat4(1.0f), navPoint.toPoint), glm::vec3(5.f, 2.f, 10.f))
        )
      );
      drawMesh(*defaultMeshes.nav, shaderProgram);
      numTriangles = numTriangles + defaultMeshes.nav -> numTriangles;
    }
    std::cout << std::endl;
    return numTriangles;
  }

  auto uiObj = std::get_if<GameObjectUIButton>(&toRender);
  if (uiObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    if (uiObj -> hasOnTint && uiObj -> toggleOn){
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(uiObj -> onTint));
    }
    auto textureOverloadId = uiObj -> toggleOn ? uiObj -> onTexture : uiObj -> offTexture;
    drawMesh(uiObj -> common.mesh, shaderProgram, textureOverloadId); 
    return uiObj -> common.mesh.numTriangles;   
  }

  auto uiSliderObj = std::get_if<GameObjectUISlider>(&toRender);
  if (uiSliderObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform1f(glGetUniformLocation(shaderProgram, "discardTexAmount"), 1 - uiSliderObj -> percentage);  
    drawMesh(uiSliderObj -> common.mesh, shaderProgram, uiSliderObj -> texture, uiSliderObj -> opacityTexture);  
    return uiSliderObj -> common.mesh.numTriangles;  
  }

  auto textObj = std::get_if<GameObjectUIText>(&toRender);
  if (textObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);   
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawWord(shaderProgram, id, textObj -> value, 1, textObj -> deltaOffset);
    return 0;
  }

  auto layoutObj = std::get_if<GameObjectUILayout>(&toRender);
  if (layoutObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);   
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));     
    int layoutVertexCount = 0;
    if (showDebug){
      layoutVertexCount += renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh);
    }
    if (layoutObj -> showBackpanel){
      auto boundWidth = layoutObj -> boundInfo.xMax - layoutObj  -> boundInfo.xMin;
      auto boundheight = layoutObj -> boundInfo.yMax - layoutObj -> boundInfo.yMin;
      auto zFightingBias = glm::vec3(0.f, 0.f, -0.001f);  
      auto rectModel = glm::scale(glm::translate(glm::mat4(1.0f), layoutObj -> boundOrigin + zFightingBias), glm::vec3(boundWidth, boundheight, 1.f));
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(rectModel));
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(layoutObj -> tint));
      drawMesh(*defaultMeshes.unitXYRect, shaderProgram);
      layoutVertexCount += defaultMeshes.unitXYRect -> numTriangles;
    }
    return layoutVertexCount;
  }

  auto geoObj = std::get_if<GameObjectGeo>(&toRender);
  if (geoObj != NULL){
    auto sphereVertexCount = 0;
    if (showDebug){
      for (auto point : geoObj -> points){
        sphereVertexCount += drawSphere(point);
      }
    }
    auto defaultNodeVertexCount = geoObj -> type == GEOSPHERE ? drawSphere(glm::vec3(0.f, 0.f, 0.f)) : renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh);
    return defaultNodeVertexCount + sphereVertexCount;
  }
  return 0;
}

void objectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& _attributes){
  GameObjectObj& toRender = mapping.at(id);
  auto variantIndex = toRender.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      objType.objectAttributes(toRender, _attributes);
      return;
    }
  }

  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (meshObj -> meshNames.size() > 0){
      _attributes.stringAttributes["mesh"] = meshObj -> meshNames.at(0);
    }
    _attributes.stringAttributes["isDisabled"] = meshObj -> isDisabled ? "true" : "false";
    _attributes.vecAttributes["tint"] = meshObj -> tint;
    return;
  }  


  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    // not yet implemented
    assert(false);
    return;
  } 

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL){
    assert(false);
    return;
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&toRender);
  if (heightmapObj != NULL){
    assert(false);
    return;
  }

  assert(false);
}

// TODO -> this needs updating hard.  
void setObjectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& attributes, std::function<void(bool)> setEmitterEnabled){
 GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (attributes.stringAttributes.find("isDisabled") != attributes.stringAttributes.end()){
      meshObj -> isDisabled = attributes.stringAttributes.at("isDisabled") == "true";;
    }
    if (attributes.stringAttributes.find("textureoffset") != attributes.stringAttributes.end()){
      meshObj -> texture.textureoffset = parseVec2(attributes.stringAttributes.at("textureoffset"));
    }
    if (attributes.vecAttributes.find("tint") != attributes.vecAttributes.end()){
      meshObj -> tint = attributes.vecAttributes.at("tint");
    }
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL){   
    lightObj -> color = attributes.vecAttributes.at("color");
    return;
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL){
    auto enabled = attributes.stringAttributes.find("state") != attributes.stringAttributes.end() ? !(attributes.stringAttributes.at("state") == "disabled") : true;
    setEmitterEnabled(enabled);
    return;
  }

  auto geoObj = std::get_if<GameObjectGeo>(&toRender);
  if (geoObj != NULL){
    if (attributes.stringAttributes.find("points") != attributes.stringAttributes.end()){
      geoObj -> points = parsePoints(attributes.stringAttributes.at("points"));
    }
    return;
  }

  auto textObj = std::get_if<GameObjectUIText>(&toRender);
  if (textObj != NULL){
    if (attributes.stringAttributes.find("value") != attributes.stringAttributes.end()){
      textObj -> value = attributes.stringAttributes.at("value");
    }
    if (attributes.numAttributes.find("spacing") != attributes.numAttributes.end()){
      textObj -> deltaOffset = attributes.numAttributes.at("spacing");
    }
    return;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL){
    return; 
  }
  assert(false);
}


void addSerializedTextureInformation(std::vector<std::pair<std::string, std::string>>& pairs, TextureInformation& texture){
  if (texture.textureoffset.x != 0.f && texture.textureoffset.y != 0.f){
    pairs.push_back(std::pair<std::string, std::string>("textureoffset", serializeVec(texture.textureoffset)));
  }
  if (texture.textureOverloadName != ""){
    pairs.push_back(std::pair<std::string, std::string>("texture", texture.textureOverloadName));
  }
  if (texture.texturesize.x != 1.f && texture.texturesize.y != 1.f){
    pairs.push_back(std::pair<std::string, std::string>("texturesize", serializeVec(texture.texturesize)));
  }
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.rootMesh != ""){
    pairs.push_back(std::pair<std::string, std::string>("mesh", obj.rootMesh));
  }
  if (obj.isDisabled){
    pairs.push_back(std::pair<std::string, std::string>("disabled", "true"));
  }
  addSerializedTextureInformation(pairs, obj.texture);
  if (!isIdentityVec(obj.tint)){
    pairs.push_back(std::pair<std::string, std::string>("tint", serializeVec(obj.tint)));
  }

  return pairs;  
}
  
std::vector<std::pair<std::string, std::string>> serializeVoxel(GameObjectVoxel obj, std::function<std::string(int)> textureName){
  std::vector<std::pair<std::string, std::string>> pairs;
  auto serializedData = serializeVoxelState(obj.voxel, textureName);

  pairs.push_back(std::pair<std::string, std::string>("from", serializedData.voxelState));
  if (serializedData.textureState != ""){
    pairs.push_back(std::pair<std::string, std::string>("fromtextures", serializedData.textureState));
  }
  return pairs;
}  

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName){
  GameObjectObj objectToSerialize = mapping.at(id);
  auto variantIndex = objectToSerialize.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      return objType.serialize(objectToSerialize);
    }
  }

  auto meshObject = std::get_if<GameObjectMesh>(&objectToSerialize);
  if (meshObject != NULL){
    return serializeMesh(*meshObject);
  }

  auto voxelObject = std::get_if<GameObjectVoxel>(&objectToSerialize);
  if (voxelObject != NULL){
    return serializeVoxel(*voxelObject, getTextureName);
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

  auto rootObj = std::get_if<GameObjectRoot>(&objectToSerialize);
  if (rootObj != NULL){
    return {};
  }
  assert(false);  
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

void applyFocusUI(std::map<objid, GameObjectObj>& mapping, objid id, std::function<void(std::string, std::string)> sendNotify){
  for (auto &[uiId, obj] : mapping){
    auto uiControl = std::get_if<GameObjectUIButton>(&obj);
    if (uiControl != NULL){
      if (id == uiId && uiControl -> canToggle){
        uiControl -> toggleOn = !uiControl -> toggleOn;
        if (uiControl -> toggleOn && uiControl -> onToggleOn != ""){
          sendNotify(uiControl -> onToggleOn, "");
        }else if (uiControl -> onToggleOff != ""){
          sendNotify(uiControl -> onToggleOff, "");
        }
      }

      if (uiControl -> common.isFocused && id != uiId){

        std::cout << "id: " << id << " is now not focused" << std::endl;
        uiControl -> common.isFocused = false;
        if (uiControl -> common.onBlur != ""){
          sendNotify(uiControl -> common.onBlur, "");
        }
      }

      if (!uiControl -> common.isFocused && id == uiId){
        std::cout << "id: " << id << " is now focused" << std::endl;
        uiControl -> common.isFocused = true;
        if (uiControl -> common.onFocus != ""){
          sendNotify(uiControl -> common.onFocus, "");
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

void onObjectFrame(std::map<objid, GameObjectObj>& mapping, std::function<void(std::string texturepath, unsigned char* data, int textureWidth, int textureHeight)> updateTextureData, float timestamp){
  // placeholder unused for now
}