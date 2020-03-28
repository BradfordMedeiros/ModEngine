#include "./object_types.h"

std::map<short, GameObjectObj> getObjectMapping() {
	std::map<short, GameObjectObj> objectMapping;
	return objectMapping;
}

GameObjectMesh createMesh(std::map<std::string, std::string> additionalFields, std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<void(std::string)> ensureMeshLoaded){
  bool usesMultipleMeshes = additionalFields.find("meshes") != additionalFields.end();
  bool isDisabled = additionalFields.find("disabled") != additionalFields.end() ; 

  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;

  if (usesMultipleMeshes){
    auto meshStrings = split(additionalFields.at("meshes"), ',');
    for (auto meshName : meshStrings){
      ensureMeshLoaded(meshName);
      meshNames.push_back(meshName);
      meshesToRender.push_back(meshes.at(meshName));
    }
  }else{
    auto meshName = (additionalFields.find("mesh") != additionalFields.end()) ? additionalFields.at("mesh") : defaultMesh;
    meshName = (meshName == "") ? defaultMesh : meshName;
    ensureMeshLoaded(meshName);
    meshNames.push_back(meshName);
    meshesToRender.push_back(meshes.at(meshName));
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .isDisabled = isDisabled,
  };

  return obj;
}
GameObjectCamera createCamera(){
  GameObjectCamera obj {};
  return obj;
}
GameObjectLight createLight(){
  GameObjectLight obj {};
  return obj;
}

GameObjectVoxel createVoxel(std::map<std::string, std::string> additionalFields, std::function<void()> onVoxelBoundInfoChanged){
  auto voxel = createVoxels(parseVoxelState(additionalFields.at("from")), onVoxelBoundInfoChanged);
  GameObjectVoxel obj {
    .voxel = voxel,
  };
  return obj;
}

void addObject(
  short id, 
  std::string objectType, 
  std::map<std::string, std::string> additionalFields,
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, 
  std::string defaultMesh, 
  std::function<void(std::string)> ensureMeshLoaded,
  std::function<void()> onVoxelBoundInfoChanged
){
  if (objectType == "default"){
    mapping[id] = createMesh(additionalFields, meshes, defaultMesh, ensureMeshLoaded);
  }else if(objectType == "camera"){
    mapping[id] = createCamera();
  }else if (objectType == "light"){
    mapping[id] = createLight();
  }else if (objectType == "voxel"){
    mapping[id] = createVoxel(additionalFields, onVoxelBoundInfoChanged);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
    assert(false);
  }
}
void removeObject(std::map<short, GameObjectObj>& mapping, short id){
  mapping.erase(id);
}

void drawBones(GLint shaderProgram, glm::mat4 model, std::vector<Bone> bones){
  int index  = 0;
  for (auto bone : bones){
    std::vector<Line> lines;
    Line line1 = {
      .fromPos = glm::vec3(0.5f, 0.f, 0.f),
      .toPos = glm::vec3(0.5f, 1.f, 0.f)
    };
    Line line2 = {
      .fromPos = glm::vec3(0.f, 0.5f, 0.f),
      .toPos = glm::vec3(1.f, 0.5f, 0.f)
    };
    lines.push_back(line1);
    lines.push_back(line2);

    glm::mat4 newModel = bone.offsetMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(newModel));
    drawLines(lines);
    index++;
  }
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

}


void renderObject(
  GLint shaderProgram, 
  short id, 
  std::map<short, GameObjectObj>& mapping, 
  Mesh& cameraMesh, 
  bool showBoundingBoxForMesh, 
  Mesh& boundingBoxMesh, 
  bool showCameras, 
  glm::mat4 model,
  bool showBoneWeight,
  bool useBoneTransform
){
  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL && !meshObj->isDisabled){
    for (auto meshToRender : meshObj -> meshesToRender){

      bool hasBones = false;
      if (meshToRender.bones.size() > 0){
        int activeProgramId; 
        glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgramId);

        //std::cout << "!!!: BONE LOCATION START -------------------: (" << activeProgramId << ")" << std::endl;
        auto modelUniformLocation = glGetUniformLocation(shaderProgram, "model");
        //std::cout << "model uniform location: " << modelUniformLocation << std::endl;

        auto hasBonesLocation = glGetUniformLocation(shaderProgram, "hasBones");
        //std::cout << "has bones location: " << hasBonesLocation << std::endl;

        for (int i = 0; i < 100; i++){
           auto boneUniformLocation = glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str());
          //std::cout << "!!!: BONE LOCATION: " << boneUniformLocation << std::endl;

          if (i >= meshToRender.bones.size()){
   
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
          }else{
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(meshToRender.bones.at(i).offsetMatrix));
          }

        }
        hasBones = true;
      }else{
        hasBones = false;
      }

      glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), showBoneWeight);
      glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), useBoneTransform);
      
      glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);
      //drawBones(shaderProgram, model, meshToRender.bones);

      glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), hasBones);
      if (hasBones){
        drawMesh(meshToRender);    
      }
    }
    return;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showCameras){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), cameraMesh.bones.size() > 0);
    drawMesh(cameraMesh);
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showCameras){   // @TODO SH0W CAMERAS SHOULD BE SHOW DEBUG, AND WE SHOULD HAVE SEPERATE MESH TYPE FOR LIGHTS AND NOT REUSE THE CAMERA
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), cameraMesh.bones.size() > 0);
    drawMesh(cameraMesh);
    return;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), voxelObj -> voxel.mesh.bones.size() > 0);
    drawMesh(voxelObj -> voxel.mesh);
    return;
  }
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj){
  // @TODO fix scene serialization, only serializing first mesh right now, which is wrong
  return { std::pair<std::string, std::string>("mesh", obj.meshNames.at(0)) };
}
std::vector<std::pair<std::string, std::string>> defaultSerialization(){
  return {}; 
}   

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, GameObjectObj>& mapping){
  GameObjectObj objectToSerialize = mapping.at(id);
  auto meshObject = std::get_if<GameObjectMesh>(&objectToSerialize);
  if (meshObject != NULL){
    return serializeMesh(*meshObject);
  }
  return defaultSerialization();
}

std::vector<short> getGameObjectsIndex(std::map<short, GameObjectObj>& mapping){
  std::vector<short> indicies;
  for (auto [id, _]: mapping){    
      indicies.push_back(id);
  }
  return indicies;
}

NameAndMesh getMeshesForId(std::map<short, GameObjectObj>& mapping, short id){  
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

std::vector<std::string> getMeshNames(std::map<short, GameObjectObj>& mapping, short id){
  std::vector<std::string> names;
  if (id == -1){
    return names;
  }
  for (auto name : getMeshesForId(mapping, id).meshNames){
    names.push_back(name);
  }

  return names;
}