#include "./object_types.h"

std::map<objid, GameObjectObj> getObjectMapping() {
	std::map<objid, GameObjectObj> objectMapping;
	return objectMapping;
}

std::size_t getVariantIndex(GameObjectObj gameobj){
  return gameobj.index();
}

void nothingObjAttr(GameObjectObj& obj, GameobjAttributes& _attributes){ }// do nothing 
void nothingSetObjAttr(GameObjectObj& obj, GameobjAttributes& _attributes, ObjectSetAttribUtil& util){ }// do nothing 


template<typename T>
std::function<void(GameObjectObj& obj, GameobjAttributes& attr)> convertElementValue(std::function<void(T&, GameobjAttributes&)> getAttr) {   
  return [getAttr](GameObjectObj& obj, GameobjAttributes& attr) -> void {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    getAttr(*objInstance, attr);
  };
}

template<typename T>
std::function<void(GameObjectObj& obj, GameobjAttributes& attr, ObjectSetAttribUtil&)> convertElementSetValue(std::function<void(T&, GameobjAttributes&, ObjectSetAttribUtil&)> setAttr) {   
  return [setAttr](GameObjectObj& obj, GameobjAttributes& attr, ObjectSetAttribUtil& util) -> void {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    setAttr(*objInstance, attr, util);
  };
}


//  std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj&)> serialize;
template<typename T>
std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj& obj, ObjectSerializeUtil& util)> convertSerialize(std::function<void(T&, ObjectSerializeUtil&)> serialize) {   
  return [](GameObjectObj& obj, ObjectSerializeUtil& util) -> std::vector<std::pair<std::string, std::string>> {
    return {};
  };
}

template<typename T>
std::function<void(GameObjectObj& obj, ObjectRemoveUtil&)> convertRemove(std::function<void(T&, ObjectRemoveUtil&)> rmObject) {   
  return [rmObject](GameObjectObj& obj, ObjectRemoveUtil& util) -> void {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    rmObject(*objInstance, util);
  };
}

std::vector<std::pair<std::string, std::string>> serializeNotImplemented(GameObjectObj& obj, ObjectSerializeUtil& util){
  std::cout << "ERROR: SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
  assert(false);
  return {};    
}

void removeDoNothing(GameObjectObj& obj, ObjectRemoveUtil& util){}

GameObjectObj createRoot(GameobjAttributes& attr, ObjectTypeUtil& util){
  return GameObjectRoot{};
}

std::vector<ObjectType> objTypes = {
  ObjectType {
    .name = "root",
    .variantType = getVariantIndex(GameObjectRoot{}),
    .createObj = createRoot,
    .objectAttributes = nothingObjAttr,
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "geo",
    .variantType = getVariantIndex(GameObjectGeo{}),
    .createObj = createGeo,
    .objectAttributes = convertElementValue<GameObjectGeo>(geoObjAttr),
    .setAttributes = convertElementSetValue<GameObjectGeo>(setGeoObjAttributes),
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "camera",
    .variantType = getVariantIndex(GameObjectCamera{}),
    .createObj = createCamera,
    .objectAttributes = convertElementValue<GameObjectCamera>(cameraObjAttr),
    .setAttributes = convertElementSetValue<GameObjectCamera>(setCameraAttributes),
    .serialize = convertSerialize<GameObjectCamera>(serializeCamera),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "portal",
    .variantType = getVariantIndex(GameObjectPortal{}),
    .createObj = createPortal,
    .objectAttributes = nothingObjAttr,
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "light",
    .variantType = getVariantIndex(GameObjectLight{}),
    .createObj = createLight,
    .objectAttributes = convertElementValue<GameObjectLight>(lightObjAttr),
    .setAttributes = convertElementSetValue<GameObjectLight>(setLightAttributes),
    .serialize = convertSerialize<GameObjectLight>(serializeLight),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "sound",
    .variantType = getVariantIndex(GameObjectSound{}),
    .createObj = createSound,
    .objectAttributes = convertElementValue<GameObjectSound>(soundObjAttr),
    .setAttributes = nothingSetObjAttr,
    .serialize = convertSerialize<GameObjectSound>(serializeSound),
    .removeObject = convertRemove<GameObjectSound>(removeSound),
  },
  ObjectType {
    .name = "text",
    .variantType = getVariantIndex(GameObjectUIText{}),
    .createObj = createUIText,
    .objectAttributes = convertElementValue<GameObjectUIText>(textObjAttributes),
    .setAttributes = convertElementSetValue<GameObjectUIText>(setUITextAttributes),
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "layout",
    .variantType = getVariantIndex(GameObjectUILayout{}),
    .createObj = createUILayout,
    .objectAttributes = nothingObjAttr,
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "navconnection",
    .variantType = getVariantIndex(GameObjectNavConns{}),
    .createObj = createNavConns,
    .objectAttributes = nothingObjAttr,
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "ui",
    .variantType = getVariantIndex(GameObjectUIButton{}),
    .createObj = createUIButton,
    .objectAttributes = nothingObjAttr,
    .setAttributes = nothingSetObjAttr,
    .serialize = convertSerialize<GameObjectUIButton>(serializeButton),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "slider",
    .variantType = getVariantIndex(GameObjectUISlider{}),
    .createObj = createUISlider,
    .objectAttributes = nothingObjAttr, 
    .setAttributes = nothingSetObjAttr,
    .serialize = convertSerialize<GameObjectUISlider>(serializeSlider),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "heightmap",
    .variantType = getVariantIndex(GameObjectHeightmap{}),
    .createObj = createHeightmap,
    .objectAttributes = nothingObjAttr, 
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = convertRemove<GameObjectHeightmap>(removeHeightmap),
  },
  ObjectType {
    .name = "navmesh",
    .variantType = getVariantIndex(GameObjectNavmesh{}),
    .createObj = createNavmesh,
    .objectAttributes = nothingObjAttr, 
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "emitter",
    .variantType = getVariantIndex(GameObjectEmitter{}),
    .createObj = createEmitter,
    .objectAttributes = nothingObjAttr, 
    .setAttributes = nothingSetObjAttr,
    .serialize = serializeNotImplemented,
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "default",
    .variantType = getVariantIndex(GameObjectMesh{}),
    .createObj = createMesh,
    .objectAttributes = convertElementValue<GameObjectMesh>(meshObjAttr),
    .setAttributes = convertElementSetValue<GameObjectMesh>(setMeshAttributes),
    .serialize = convertSerialize<GameObjectMesh>(serializeMesh),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "voxel", 
    .variantType = getVariantIndex(GameObjectVoxel{}),
    .createObj = createVoxel, 
    .objectAttributes = nothingObjAttr,
    .setAttributes = nothingSetObjAttr,
    .serialize = convertSerialize<GameObjectVoxel>(serializeVoxel),
    .removeObject  = removeDoNothing,
  },
};


void addObject(objid id, std::string objectType, GameobjAttributes& attr, std::map<objid, GameObjectObj>& mapping, ObjectTypeUtil util){
  for (auto &objType : objTypes){
    if (objectType == objType.name){
      mapping[id] = objType.createObj(attr, util);
      return;
    }
  }
  std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  assert(false);
}

void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void()> rmEmitter
){
  auto Object = mapping.at(id); 
  auto variantIndex = Object.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      std::cout << "type is: " << objType.name << std::endl;
      ObjectRemoveUtil util { .rmEmitter = rmEmitter };
      objType.removeObject(Object, util);
      mapping.erase(id);
      return;
    }
  }
  std::cout << "object type not implemented" << std::endl;
  assert(false);
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
    // todo instance these
    for (int i = 0; i < voxelBodies.size(); i++){
      auto voxelBody = voxelBodies.at(i);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, voxelBody.position + glm::vec3(0.5f, 0.5f, 0.5f))));
      drawMesh(*defaultMeshes.voxelCubeMesh, shaderProgram, voxelBody.textureId);   
      numTriangles = numTriangles + defaultMeshes.voxelCubeMesh -> numTriangles; 
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

void objectAttributes(GameObjectObj& toRender, GameobjAttributes& _attributes){
  auto variantIndex = toRender.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      objType.objectAttributes(toRender, _attributes);
      return;
    }
  }
  assert(false);
}

// TODO -> this needs updating hard.  
void setObjectAttributes(std::map<objid, GameObjectObj>& mapping, objid id, GameobjAttributes& attributes, std::function<void(bool)> setEmitterEnabled){
  GameObjectObj& toRender = mapping.at(id);
  auto variantIndex = toRender.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      ObjectSetAttribUtil util {
        .setEmitterEnabled = setEmitterEnabled,
      };
      objType.setAttributes(toRender, attributes, util);
      return;
    }
  }
  std::cout << "obj type not supported" << std::endl;
  assert(false);
}
  
std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName){
  GameObjectObj objectToSerialize = mapping.at(id);
  auto variantIndex = objectToSerialize.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      ObjectSerializeUtil serializeUtil {
        .textureName = getTextureName,
      };
      return objType.serialize(objectToSerialize, serializeUtil);
    }
  }

  std::cout << "obj type not supported" << std::endl;
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