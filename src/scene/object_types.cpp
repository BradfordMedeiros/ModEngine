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
GameObjectCamera createCamera(){
  GameObjectCamera obj {};
  return obj;
}
GameObjectPortal createPortal(GameobjAttributes& attr){
  bool hasCamera = attr.stringAttributes.find("camera") != attr.stringAttributes.end();
  auto camera = hasCamera ? attr.stringAttributes.at("camera") : "";
  auto perspective = attr.stringAttributes.find("perspective") != attr.stringAttributes.end() ? attr.stringAttributes.at("perspective") == "true" : false;

  GameObjectPortal obj {
    .camera = camera,
    .perspective = perspective,
  };
  return obj;
}
GameObjectSound createSound(GameobjAttributes& attr){
  auto clip = attr.stringAttributes.at("clip");
  auto loop = (attr.stringAttributes.find("loop") != attr.stringAttributes.end()) && (attr.stringAttributes.at("loop") == "true");
  auto source = loadSoundState(clip, loop);
  GameObjectSound obj {
    .clip = clip,
    .source = source,
    .loop = loop,
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
GameObjectLight createLight(GameobjAttributes& attr){
  auto color = attr.vecAttributes.find("color") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("color");
  auto lightType = attr.stringAttributes.find("type") == attr.stringAttributes.end() ? LIGHT_POINT : getLightType(attr.stringAttributes.at("type"));
  auto maxangle = (lightType != LIGHT_SPOTLIGHT || attr.numAttributes.find("angle") == attr.numAttributes.end()) ? -10.f : attr.numAttributes.at("angle");

  // constant, linear, quadratic
  // in shader =>  float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  
  auto attenuation = attr.vecAttributes.find("attenuation") == attr.vecAttributes.end() ? glm::vec3(1.0, 0.007, 0.0002) : attr.vecAttributes.at("attenuation");

  GameObjectLight obj {
    .color = color,
    .type = lightType,
    .maxangle = maxangle, 
    .attenuation = attenuation,
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

GameObjectChannel createChannel(GameobjAttributes& attr){
  bool hasFrom = attr.stringAttributes.find("from") != attr.stringAttributes.end();
  bool hasTo = attr.stringAttributes.find("to") != attr.stringAttributes.end();

  GameObjectChannel obj {
    .from = hasFrom ? attr.stringAttributes.at("from") : "",
    .to = hasTo ? attr.stringAttributes.at("to") : "",
    .complete = hasFrom && hasTo,
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

GameObjectEmitter createEmitter(std::function<void(float, float, int, std::map<std::string, std::string>, std::vector<EmitterDelta>, bool)> addEmitter, GameobjAttributes& attributes){
  GameObjectEmitter obj {};
  float spawnrate = attributes.numAttributes.find("rate") != attributes.numAttributes.end() ? attributes.numAttributes.at("rate") : 1.f;
  float lifetime = attributes.numAttributes.find("duration") != attributes.numAttributes.end() ? attributes.numAttributes.at("duration") : 10.f;
  int limit = attributes.numAttributes.find("limit") != attributes.numAttributes.end() ? attributes.numAttributes.at("limit") : 10;
  auto enabled = attributes.stringAttributes.find("state") != attributes.stringAttributes.end() ? !(attributes.stringAttributes.at("state") == "disabled") : true;
  assert(limit >= 0);
  addEmitter(spawnrate, lifetime, limit, particleFields(attributes.stringAttributes), emitterDeltas(attributes.stringAttributes), enabled);
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

GameObjectNavConns createNavConns(GameobjAttributes& attr){
  GameObjectNavConns obj {
    .navgraph = createNavGraph(attr.stringAttributes),
  };
  return obj;
}

GameObjectUICommon parseCommon(GameobjAttributes& attr, std::map<std::string, MeshRef>& meshes){
  auto onFocus = attr.stringAttributes.find("focus") != attr.stringAttributes.end() ? attr.stringAttributes.at("focus") : "";
  auto onBlur = attr.stringAttributes.find("blur") != attr.stringAttributes.end() ? attr.stringAttributes.at("blur") : "";
  GameObjectUICommon common {
    .mesh = meshes.at("./res/models/controls/input.obj").mesh,
    .isFocused = false,
    .onFocus = onFocus,
    .onBlur = onBlur,
  };
  return common;
}
GameObjectUIButton createUIButton(GameobjAttributes& attr, std::map<std::string, MeshRef>& meshes, std::function<Texture(std::string)> ensureTextureLoaded){
  auto onTexture = attr.stringAttributes.find("ontexture") != attr.stringAttributes.end() ? attr.stringAttributes.at("ontexture") : "";
  auto offTexture = attr.stringAttributes.find("offtexture") != attr.stringAttributes.end() ? attr.stringAttributes.at("offtexture") : "";
  auto toggleOn = attr.stringAttributes.find("state") != attr.stringAttributes.end() && attr.stringAttributes.at("state") == "on";
  auto canToggle = attr.stringAttributes.find("cantoggle") == attr.stringAttributes.end() || !(attr.stringAttributes.at("cantoggle") == "false");
  auto onToggleOn = attr.stringAttributes.find("on") != attr.stringAttributes.end() ? attr.stringAttributes.at("on") : "";
  auto onToggleOff = attr.stringAttributes.find("off") != attr.stringAttributes.end() ? attr.stringAttributes.at("off") : "";
  auto hasOnTint  = attr.vecAttributes.find("ontint") != attr.vecAttributes.end();
  auto onTint = hasOnTint ? attr.vecAttributes.at("ontint") : glm::vec3(1.f, 1.f, 1.f);

  GameObjectUIButton obj { 
    .common = parseCommon(attr, meshes),
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

GameObjectUISlider createUISlider(GameobjAttributes& attr, std::map<std::string, MeshRef>& meshes, std::function<Texture(std::string)> ensureTextureLoaded){
  auto onSlide = attr.stringAttributes.find("onslide") != attr.stringAttributes.end() ? attr.stringAttributes.at("onslide") : "";

  GameObjectUISlider obj {
    .common = parseCommon(attr, meshes),
    .min = 0.f,
    .max = 100.f,
    .percentage = 100.f,
    .texture = ensureTextureLoaded("./res/models/controls/slider.png").textureId,
    .opacityTexture = ensureTextureLoaded("./res/models/controls/slider_opacity.png").textureId,
    .onSlide = onSlide,
  };
  return obj;
}

GameObjectUIText createUIText(GameobjAttributes& attr){
  auto value = attr.stringAttributes.find("value") != attr.stringAttributes.end() ? attr.stringAttributes.at("value") : "";
  auto deltaOffset = attr.numAttributes.find("spacing") != attr.numAttributes.end() ? attr.numAttributes.at("spacing") : 2;
  GameObjectUIText obj {
    .value = value,
    .deltaOffset = deltaOffset,
  };
  return obj;
}

GameObjectUILayout createUILayout(GameobjAttributes& attr){
  auto spacing = attr.numAttributes.find("spacing") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("spacing");
  auto type = attr.stringAttributes.find("type") != attr.stringAttributes.end() && (attr.stringAttributes.at("type") == "vertical") ? LAYOUT_VERTICAL : LAYOUT_HORIZONTAL;
  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  auto order = (attr.numAttributes.find("order") == attr.numAttributes.end()) ? 0 : attr.numAttributes.at("order");
  auto showBackpanel = (attr.stringAttributes.find("backpanel") != attr.stringAttributes.end() && attr.stringAttributes.at("backpanel") == "true");
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  auto margin = attr.numAttributes.find("margin") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("margin");

  BoundInfo boundInfo {
    .xMin = 0, .xMax = 0,
    .yMin = 0, .yMax = 0,
    .zMin = 0, .zMax = 0,
  };
  GameObjectUILayout obj{
    .type = type,
    .spacing = spacing,
    .elements = elements,
    .order = order,
    .boundInfo = boundInfo,
    .boundOrigin = glm::vec3(0.f, 0.f, 0.f),
    .showBackpanel = showBackpanel,
    .tint = tint,
    .margin = margin,
  };
  return obj;
}

GameObjectVideo createVideo(
  GameobjAttributes& attr, 
  std::function<Texture(std::string filepath, unsigned char* data, int textureWidth, int textureHeight, int numChannels)> ensureTextureDataLoaded
){
  auto videoPath = attr.stringAttributes.at("source");
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

std::vector<glm::vec3> parsePoints(std::string value){
  std::vector<glm::vec3> points;
  auto pointsArr = filterWhitespace(split(value, '|'));
  for (auto point : pointsArr){
    glm::vec3 pointValue(0.f, 0.f, 0.f);
    auto isVec = maybeParseVec(point, pointValue);
    assert(isVec);
    points.push_back(pointValue);
  }
  return points;
}
std::string pointsToString(std::vector<glm::vec3>& points){
  std::string value = "";
  for (int i = 0; i < points.size(); i++){
    auto point = points.at(i);
    value = value + print(point);
    if (i != (points.size() - 1)){
      value = value + "|";
    }
  }
  return value;
}
GameObjectGeo createGeo(GameobjAttributes& attr){
  auto points = parsePoints(
    attr.stringAttributes.find("points") != attr.stringAttributes.end() ? 
    attr.stringAttributes.at("points") : 
    ""
  );

  auto type = attr.stringAttributes.find("shape") != attr.stringAttributes.end() ? 
  (attr.stringAttributes.at("shape") == "sphere" ? GEOSPHERE : GEODEFAULT) : 
  GEODEFAULT;

  GameObjectGeo geo{
    .points = points,
    .type = type,
  };
  return geo;
}

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
  std::function<void(float, float, int, std::map<std::string, std::string>, std::vector<EmitterDelta>, bool)> addEmitter,
  std::function<Mesh(MeshData&)> loadMesh
){
  if (objectType == "default"){
    mapping[id] = createMesh(attr, meshes, ensureMeshLoaded, ensureTextureLoaded);
  }else if(objectType == "camera"){
    mapping[id] = createCamera();
  }else if (objectType == "portal"){
    mapping[id] = createPortal(attr);
  }else if(objectType == "sound"){
    mapping[id] = createSound(attr);
  }else if(objectType == "light"){
    mapping[id] = createLight(attr);
  }else if(objectType == "voxel"){
    auto defaultVoxelTexture = ensureTextureLoaded("./res/textures/wood.jpg");
    mapping[id] = createVoxel(attr, onCollisionChange, defaultVoxelTexture.textureId, ensureTextureLoaded);
  }else if(objectType == "channel"){
    mapping[id] = createChannel(attr);
  }else if (objectType == "root"){
    mapping[id] = GameObjectRoot{};
  }else if (objectType == "emitter"){
    mapping[id] = createEmitter(addEmitter, attr);
  }else if (objectType == "heightmap"){
    mapping[id] = createHeightmap(attr, loadMesh, ensureTextureLoaded);
  }else if (objectType == "navmesh"){
    mapping[id] = createNavmesh(meshes.at("./res/models/ui/node.obj").mesh);
  }else if (objectType == "navconnection"){
    mapping[id] = createNavConns(attr);
  }else if (objectType == "ui"){
    mapping[id] = createUIButton(attr, meshes, ensureTextureLoaded);
  }else if (objectType == "slider"){
    mapping[id] = createUISlider(attr, meshes, ensureTextureLoaded);
  }else if (objectType == "text"){
    mapping[id] = createUIText(attr);
  }else if (objectType == "layout"){
    mapping[id] = createUILayout(attr);
  }else if (objectType == "video"){
    mapping[id] = createVideo(attr, ensureTextureDataLoaded);
  }else if (objectType == "geo"){
    mapping[id] = createGeo(attr);
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

  auto videoObj = std::get_if<GameObjectVideo>(&Object);
  if (videoObj != NULL){
    freeVideoContent(videoObj -> video);
    freeBufferedAudio(videoObj -> sound);
  }

  mapping.erase(id);
}

int renderDefaultNode(GLint shaderProgram, Mesh& nodeMesh){
  glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
  glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
  glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
  glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
  drawMesh(nodeMesh, shaderProgram);
  return nodeMesh.numTriangles;
}

int renderObject(
  GLint shaderProgram, 
  objid id, 
  std::map<objid, GameObjectObj>& mapping, 
  Mesh& nodeMesh,
  Mesh& cameraMesh,
  Mesh& portalMesh, 
  Mesh& voxelCubeMesh,
  Mesh& unitXYRect, // unit xy rect is a 1x1 2d plane along the xy axis, centered at the origin
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform,
  unsigned int portalTexture,
  glm::mat4 model,
  bool drawPoints,
  std::function<void(GLint, objid, std::string, unsigned int, float)> drawWord,
  std::function<int(glm::vec3)> drawSphere
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
    drawMesh(nodeMesh, shaderProgram, meshObj -> texture.textureOverloadId);    
    return nodeMesh.numTriangles;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, cameraMesh);
  }

  auto soundObject = std::get_if<GameObjectSound>(&toRender);
  if (soundObject != NULL && showDebug){
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto portalObj = std::get_if<GameObjectPortal>(&toRender);
  if (portalObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(portalMesh, shaderProgram, portalTexture);
    return portalMesh.numTriangles;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showDebug){   
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), voxelCubeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));

    auto voxelBodies = getVoxelBodies(voxelObj -> voxel);

    int numTriangles = 0;
    for (int i = 0; i < voxelBodies.size(); i++){
      auto voxelBody = voxelBodies.at(i);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, voxelBody.position + glm::vec3(0.5f, 0.5f, 0.5f))));
      drawMesh(voxelCubeMesh, shaderProgram, voxelBody.textureId);   
      numTriangles = numTriangles = voxelCubeMesh.numTriangles; 
    }
    return numTriangles;
  }

  auto channelObj = std::get_if<GameObjectChannel>(&toRender);
  if (channelObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto rootObj = std::get_if<GameObjectRoot>(&toRender);
  if (rootObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto heightmapObj = std::get_if<GameObjectHeightmap>(&toRender);
  if (heightmapObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(heightmapObj -> texture.textureoffset));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(heightmapObj -> texture.texturetiling));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(heightmapObj -> texture.texturesize));
    drawMesh(heightmapObj -> mesh, shaderProgram, heightmapObj -> texture.textureOverloadId);   
    return heightmapObj -> mesh.numTriangles;
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&toRender);
  if (navmeshObj != NULL && showDebug){
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto navconnObj = std::get_if<GameObjectNavConns>(&toRender);
  if (navconnObj != NULL && showDebug){
    int numTriangles = 0;

    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(0.f, 0.f)));  
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    glUniform2fv(glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
    drawMesh(nodeMesh, shaderProgram); 
    numTriangles = numTriangles + nodeMesh.numTriangles;

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
      numTriangles = numTriangles + nodeMesh.numTriangles;

      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
      glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(
          glm::scale(glm::translate(glm::mat4(1.0f), navPoint.toPoint), glm::vec3(5.f, 2.f, 10.f))
        )
      );
      drawMesh(nodeMesh, shaderProgram);
      numTriangles = numTriangles + nodeMesh.numTriangles;
    }
    std::cout << std::endl;
    return numTriangles;
  }

  auto uiObj = std::get_if<GameObjectUIButton>(&toRender);
  if (uiObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
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
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
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
      layoutVertexCount += renderDefaultNode(shaderProgram, nodeMesh);
    }
    if (layoutObj -> showBackpanel){
      auto boundWidth = layoutObj -> boundInfo.xMax - layoutObj  -> boundInfo.xMin;
      auto boundheight = layoutObj -> boundInfo.yMax - layoutObj -> boundInfo.yMin;
      auto zFightingBias = glm::vec3(0.f, 0.f, -0.001f);  
      auto rectModel = glm::scale(glm::translate(glm::mat4(1.0f), layoutObj -> boundOrigin + zFightingBias), glm::vec3(boundWidth, boundheight, 1.f));
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(rectModel));
      glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(layoutObj -> tint));
      drawMesh(unitXYRect, shaderProgram);
      layoutVertexCount += unitXYRect.numTriangles;
    }
    return layoutVertexCount;
  }

  auto videoObj = std::get_if<GameObjectVideo>(&toRender);
  if (videoObj != NULL){
    return renderDefaultNode(shaderProgram, nodeMesh);
  }

  auto geoObj = std::get_if<GameObjectGeo>(&toRender);
  if (geoObj != NULL){
    auto sphereVertexCount = 0;
    if (showDebug){
      for (auto point : geoObj -> points){
        sphereVertexCount += drawSphere(point);
      }
    }
    auto defaultNodeVertexCount = geoObj -> type == GEOSPHERE ? drawSphere(glm::vec3(0.f, 0.f, 0.f)) : renderDefaultNode(shaderProgram, nodeMesh);
    return defaultNodeVertexCount + sphereVertexCount;
  }
  return 0;
}

void objectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& _attributes){
  GameObjectObj& toRender = mapping.at(id);

  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (meshObj -> meshNames.size() > 0){
      _attributes.stringAttributes["mesh"] = meshObj -> meshNames.at(0);
    }
    _attributes.stringAttributes["isDisabled"] = meshObj -> isDisabled ? "true" : "false";
    _attributes.vecAttributes["tint"] = meshObj -> tint;
    return;
  }  

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL){
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL){   
    _attributes.vecAttributes["color"] = lightObj -> color;
    return;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    // not yet implemented
    assert(false);
    return;
  } 

  auto soundObj = std::get_if<GameObjectSound>(&toRender);
  if (soundObj != NULL){
    _attributes.stringAttributes["clip"] = soundObj -> clip;
    _attributes.stringAttributes["loop"] = soundObj -> loop ? "true" : "false";
    return;
  }

  auto channelObj = std::get_if<GameObjectChannel>(&toRender);
  if (channelObj != NULL){
    _attributes.stringAttributes["from"] = channelObj -> from;
    _attributes.stringAttributes["to"] = channelObj -> to;
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

  auto textObj = std::get_if<GameObjectUIText>(&toRender);
  if (textObj != NULL){
    _attributes.stringAttributes["value"] = textObj -> value;
    _attributes.stringAttributes["spacing"] = std::to_string(textObj -> deltaOffset);
    return;
  }


  auto geoObj = std::get_if<GameObjectGeo>(&toRender);
  if (geoObj != NULL){
    _attributes.stringAttributes["points"] = pointsToString(geoObj -> points);
    if (geoObj -> type == GEOSPHERE){   // should show for any shape
      _attributes.stringAttributes["shape"] = "sphere";
    }
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
      std::cout << "updating texture offset" << std::endl;
      meshObj -> texture.textureoffset = parseVec2(attributes.stringAttributes.at("textureoffset"));
    }
    if (attributes.vecAttributes.find("tint") != attributes.vecAttributes.end()){
      meshObj -> tint = attributes.vecAttributes.at("tint");
    }
    return;
  }

  auto cameraObj = std::get_if<GameObjectChannel>(&toRender);
  if (cameraObj != NULL){
    if (attributes.stringAttributes.find("to") != attributes.stringAttributes.end()){
      cameraObj -> to = attributes.stringAttributes.at("to");
    }
    if (attributes.stringAttributes.find("from") != attributes.stringAttributes.end()){
      cameraObj -> from = attributes.stringAttributes.at("from");
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

  auto videoObj = std::get_if<GameObjectVideo>(&toRender);
  if (videoObj != NULL){
    assert(false);
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
std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj){
  return {}; 
}  
std::vector<std::pair<std::string, std::string>> serializeSound(GameObjectSound obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.clip != ""){
    pairs.push_back(std::pair<std::string, std::string>("clip", obj.clip));
  }
  if (obj.loop){
    pairs.push_back(std::pair<std::string, std::string>("loop", "true"));
  }
  return pairs;
}   
std::vector<std::pair<std::string, std::string>> serializeLight(GameObjectLight obj){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back(std::pair<std::string, std::string>("color", serializeVec(obj.color)));
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

std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName){
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
    return serializeVoxel(*voxelObject, getTextureName);
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

  auto uiTextObj = std::get_if<GameObjectUIText>(&objectToSerialize);
  if (uiTextObj != NULL){
    std::cout << "ERROR: UI SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto uiLayoutObj = std::get_if<GameObjectUILayout>(&objectToSerialize);
  if (uiLayoutObj != NULL){
    std::cout << "ERROR: UI SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto uiVideoObj = std::get_if<GameObjectVideo>(&objectToSerialize);
  if (uiVideoObj != NULL){
    std::cout << "ERROR: VIDEO SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
    assert(false);
    return {};
  }

  auto geoObj = std::get_if<GameObjectGeo>(&objectToSerialize);
  if (geoObj != NULL){
    std::cout << "ERROR: GEO SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
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