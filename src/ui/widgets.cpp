#include "./widgets.h"

extern CustomApiBindings* mainApi;
std::vector<std::string> getAllShaders();
void sendManipulatorEvent(MANIPULATOR_EVENT event);

void renderDebug(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Debug Panel");
	}

  {
    auto currValue = isEditorDebug();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Debug", &currValue);
    if(oldValue != currValue){
      setEditorDebug(currValue);
    }
  }

  {
    auto currValue = isShowCamera();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Cameras", &currValue);
    if(oldValue != currValue){
      setShowCamera(currValue);
    }
  }

  {
    auto currValue = isShowSound();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Sounds", &currValue);
    if(oldValue != currValue){
      setShowSound(currValue);
    }
  }
  {
    auto currValue = isShowEmitters();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Emitters", &currValue);
    if(oldValue != currValue){
      setShowEmitters(currValue);
    }
  }

  {
    auto currValue = isShowLights();
    auto oldValue = currValue;
    ImGui::Checkbox("Show Lights", &currValue);
    if(oldValue != currValue){
      setShowLights(currValue);
    }
  }

  ImGui::Dummy(ImVec2(0, 10));

  {
    auto currValue = isMuted();
    auto oldValue = currValue;
    ImGui::Checkbox("Mute", &currValue);
    if(oldValue != currValue){
      setIsMuted(currValue);
    }
  }

  if (includePanel){
	  ImGui::End();
  }
}

void renderObjectCount(bool includePanel){
	struct ObjectCount {
		std::string field;
		std::string statName;
	};

	std::vector<ObjectCount> objects {
		ObjectCount {
			.field = "Object count",
			.statName = "object-count",
		},
		ObjectCount {
			.field = "Rigid bodies",
			.statName = "rigidbody-count",
		},
		ObjectCount {
			.field = "Scenes loaded",
			.statName = "scenes-loaded",
		},
		ObjectCount {
			.field = "Num Textures",
			.statName = "num-textures",
		},
		ObjectCount {
			.field = "Num Models",
			.statName = "num-models",
		},
		ObjectCount {
			.field = "Num Meshes",
			.statName = "num-meshes",
		},
		ObjectCount {
			.field = "Num Animations",
			.statName = "num-animations",
		},
	};


	if (includePanel){
		ImGui::Begin("Object Count");
	}

	for (auto& object : objects){
		auto statSymbol = mainApi -> stat(object.statName);
		auto statValue = mainApi -> statValue(statSymbol);
		auto objectCount = std::get_if<int>(&statValue);
		modassert(objectCount, "object count is NULL");

		ImGui::Text(object.field.c_str());
		ImGui::SameLine();
		ImGui::Text(std::to_string(*objectCount).c_str());
	}

	if (includePanel){
		ImGui::End();
	}
}

void renderActiveScene(bool includePanel, std::optional<objid> activeScene){
	if (includePanel){
	  ImGui::Begin("Active Scene");
	}

  if (activeScene.has_value()){
    ImGui::Text((std::string("Active Id = ") + std::to_string(activeScene.value())).c_str());
    if(ImGui::Button("Save Scene")){
      mainApi -> saveScene(false /*include ids */, activeScene.value(), std::nullopt /* filename */);
    }
    if(ImGui::Button("Reset Scene")){
      mainApi -> resetScene(activeScene.value());
    }
    if(ImGui::Button("New Scene")){
      modassert(false, "not yet implemented");
    }
  }

  if (includePanel){
	  ImGui::End();
  }
}

void renderCreateObj(bool includePanel, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Create Object");
  }

  if (sceneId.has_value()){
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };

    if(ImGui::Button("Create Mesh")){
      mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string("mesh-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
    if(ImGui::Button("Create Camera")){
      mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string(">camera-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
    if(ImGui::Button("Create Light")){
      mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string("!light-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
    if(ImGui::Button("Create Text")){
      mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string(")text-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
  }


/*
getUniqueObjectName
  .createCamera = []() -> void {
    makeObject(uiManagerContext.uiContext -> activeSceneId().value(), std::string(">camera-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .createLight = []() -> void {
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };
    makeObject(uiManagerContext.uiContext -> activeSceneId().value(), std::string("!light-") + uniqueNameSuffix(), attr, submodelAttributes);
  },
  .createNavmesh = []() -> void {
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };
    makeObject(uiManagerContext.uiContext -> activeSceneId().value(), std::string(";navmesh-") + uniqueNameSuffix(), attr, submodelAttributes);
  },*/


  if (includePanel){
    ImGui::End();
  }
}

void renderCameraPanel(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Cameras");
	}

  static bool doThing = false;
  ImGui::Checkbox("Depth of Field", &doThing);
  
 float speed = 5.0f;

  ImGui::SliderFloat("Min Blur", &speed, 0.0f, 10.0f);
  ImGui::SliderFloat("Max Blur", &speed, 0.0f, 10.0f);
  ImGui::SliderFloat("Blur Amount", &speed, 0.0f, 10.0f);

  if (includePanel){
	  ImGui::End();
  }
}

void renderLightPanel(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Light");
	}

  static bool showOption = false;
  ImGui::Text("Type");

  ImGui::Checkbox("Point", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Spotlight", &showOption);
  ImGui::SameLine();
  ImGui::Checkbox("Directional", &showOption);

  if (includePanel){
	  ImGui::End();
  }
}

void renderMeshPanel(bool includePanel, std::optional<objid> objectToDetail){
  if (includePanel){
    ImGui::Begin("Mesh");
  }

  ImGui::Text("Mesh");
  if (objectToDetail.has_value()){
    auto id = objectToDetail.value();
    auto tint = getGameObjectTint(id);

    ImGui::Text(std::to_string(id).c_str());

    float color[4] = {tint.r, tint.g, tint.b, tint.a};
    if (ImGui::ColorEdit4("Tint", color)){
      setGameObjectTint(id, glm::vec4(color[0], color[1], color[2], color[3]));
    }
  }

  if (includePanel){
    ImGui::End();
  }
}

void renderUnknownObjPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Unsupported Object Type");
  }

  ImGui::Text("Unsupported Object Type");
  ImGui::Text("Detail Not yet implemented");

  if (includePanel){
    ImGui::End();
  } 
}

void renderObjPanel(bool includePanel, std::optional<objid> objectToDetail){
  if (includePanel){
    ImGui::Begin("Object Type Panel");
  }

  if (objectToDetail.has_value()){
    auto id = objectToDetail.value();
    auto type = getObjectType(id);
    if (type == OBJ_MESH){
      renderMeshPanel(includePanel, objectToDetail);
      // enum ObjectType {, , , , , , ,  };
    }else if (type == OBJ_CAMERA){
      renderCameraPanel(includePanel);
    }else if (type == OBJ_PORTAL){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_SOUND){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_LIGHT){
      renderLightPanel(includePanel);
    }else if (type == OBJ_OCTREE){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_EMITTER){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_NAVMESH){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_TEXT){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_PREFAB){
      renderUnknownObjPanel(includePanel);
    }else if (type == OBJ_VIDEO){
      renderUnknownObjPanel(includePanel);
    }else{
      renderUnknownObjPanel(includePanel);
    }
  }

  if (includePanel){
    ImGui::End();
  } 
}

void renderObjectDetails(objid id, bool includePanel){
  if (id == 0){
    return;
  }

  auto name = mainApi -> getGameObjNameForId(id).value();

  if (includePanel){
      ImGui::Begin("Object Details");
  }

  std::string objectName = std::string("Name: ") + name;
  ImGui::Text(objectName.c_str());

  static std::string testname = name;
  static objid objectId = id;
  if (objectId != id){
  	testname = objectName;
  }

  ImGui::InputText("Rename Object", &testname);
  ImGui::Button("Rename");
  ImGui::Dummy(ImVec2(0, 10));


  bool showPhysics = isGameObjectPhysicsEnabled(id);
  bool newShowPhysics = showPhysics;
  ImGui::Checkbox("Enable Physics", &newShowPhysics);
  if (newShowPhysics != showPhysics){
    setGameObjectPhysicsEnable(id, newShowPhysics);
  }

  bool showIsDynamic = isGameObjectPhysicsDynamic(id);
  bool newShowIsDynamic = showIsDynamic;
  ImGui::Checkbox("Dynamic", &newShowIsDynamic);
  if (showIsDynamic != newShowIsDynamic){
    setGameObjectPhysicsDynamic(id, newShowIsDynamic);
  }

  bool showOption = false;
  /*
  .structOffset = offsetof(GameObject, physicsOptions.hasCollisions),
    .field = "physics_collision", 
    .onString = "collide",
    .offString = "nocollide",
    .defaultValue = true,*/

  auto showHasCollision = getGameObjectHasCollision(id);
  bool newShowHasCollision = showHasCollision;
  ImGui::Checkbox("Collision", &newShowHasCollision);
  if (showHasCollision != newShowHasCollision){
    setGameObjectHasCollision(id, newShowHasCollision);
  }

  {
    std::vector<std::string> items = {
      "shape_box", "shape_sphere", "shape_capsule", "shape_cylinder", "shape_hull", "shape_exact", "shape_auto"
    };
    std::string shape = getGameObjectPhysicsShape(id);
    int current = 0;
    for (int i = 0; i < items.size(); i++){
      if (items.at(i) == shape){
        current = i;
        break;
      }
    }
    int oldCurrent = current;

    if (ImGui::BeginCombo("Physics Shape", shape.c_str()))
    {
        for (int i = 0; i < items.size(); i++){
          bool selected = (current == i);
            if (ImGui::Selectable(items.at(i).c_str(), selected)){
               current = i;
            }
            if (selected){
              ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (current != oldCurrent){
      std::cout << "set physics shape: " << items.at(current) << std::endl;
      setGameObjectPhysicsShape(id, items.at(current));

    }

  }
  ImGui::Dummy(ImVec2(0, 10));

  // Maybe should protect to only set when it changes
  {
    auto objPosition = mainApi -> getGameObjectPos(id, true, "[ui] obj pos");
  
    static float position[3] = {0};
    position[0] = objPosition.x;
    position[1] = objPosition.y;
    position[2] = objPosition.z;

    ImGui::Text("Position");
    ImGui::PushItemWidth(70);
    ImGui::DragFloat("X", &position[0], 0.1f);
    ImGui::SameLine();
    ImGui::DragFloat("Y", &position[1], 0.1f);
    ImGui::SameLine();
    ImGui::DragFloat("Z", &position[2], 0.1f);
    ImGui::PopItemWidth();

    mainApi -> setGameObjectPosition(id, glm::vec3(position[0], position[1], position[2]), true, Hint { .hint = "[ui] - obj set pos" });
  }

  {
    auto objScale = mainApi -> getGameObjectScale(id, true);
  
    static float scale[3] = {0};
    scale[0] = objScale.x;
    scale[1] = objScale.y;
    scale[2] = objScale.z;

    ImGui::Text("Scale");
    ImGui::PushItemWidth(70);
    ImGui::DragFloat("#X", &scale[0], 0.1f);
    ImGui::SameLine();
    ImGui::DragFloat("#Y", &scale[1], 0.1f);
    ImGui::SameLine();
    ImGui::DragFloat("#Z", &scale[2], 0.1f);
    ImGui::PopItemWidth();

    mainApi -> setGameObjectScale(id, glm::vec3(scale[0], scale[1], scale[2]), true);
  }


  {
    auto layer = getGameObjectLayer(id);

    std::vector<std::string> items = {
        "default",
        "ui",
        "basicui",
        "noselect",
        "nolighting",
    };
    int current = -1;
    for (int i = 0; i < items.size(); i++){
      if (layer == items.at(i)){
        current = i;
      }
    }

    if (layer == ""){
      current = 0;
    }
    auto labelName = current == -1 ? "<unknown>" : items.at(current);

    int oldLayerIndex = current;
    if (ImGui::BeginCombo("Layer", labelName.c_str()))
    {
        for (int i = 0; i < items.size(); i++){
            bool selected = (current == i);
            if (ImGui::Selectable(items.at(i).c_str(), selected)){
               current = i;
            }
            if (selected){
              ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (oldLayerIndex != current){
      std::cout << "new layer is: " << items.at(current) << std::endl;
      setGameObjectLayer(id, current == 0 ? "" : items.at(current));
    }
  }



  {
    std::vector<std::string> items = getAllShaders();
    auto shader = getGameObjectShader(id);
    std::cout << "curr shader: " << shader << std::endl;
    int oldShader = 0;
    for (int i = 0; i < items.size(); i++){
      if (items.at(i) == shader || (items.at(i) == "default" && shader == "")){
        oldShader = i;
        break;
      }
    }

    int newShader = oldShader;
    if (ImGui::BeginCombo("Shader", items.at(oldShader).c_str()))
    {
        for (int i = 0; i < items.size(); i++){
            bool selected = (newShader == i);
            if (ImGui::Selectable(items.at(i).c_str(), selected)){
               newShader = i;
            }
            if (selected){
              ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (oldShader != newShader){
      auto& newShaderStr = items.at(newShader);
      std::cout << "setting curr shader: " << newShaderStr << std::endl;
      setGameObjectShader(id, newShaderStr == "default" ? "" : newShaderStr);
    }
  }


  auto objectDetailsSize = ImGui::GetWindowSize();
  std::cout << "object details size: " << objectDetailsSize.x << std::endl;

  if (includePanel){
      ImGui::End();
  }
}

void renderRenderPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Render Settings");
  }

  {
    auto currValue = isDiffuseEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("Diffuse", &currValue);
    if(oldValue != currValue){
      setDiffuseEnabled(currValue);
    }
  }
  {
    auto currValue = isSpecularEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("specular", &currValue);
    if(oldValue != currValue){
      setSpecularEnabled(currValue);
    }
  }
  {
    auto currValue = isBloomEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("bloom", &currValue);
    if(oldValue != currValue){
      setBloomEnabled(currValue);
    }
  }
  {
    auto currValue = isAttenuationEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("attenuation", &currValue);
    if(oldValue != currValue){
      setAttenuationEnabled(currValue);
    }
  }
  {
    auto currValue = isShadowsEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("shadows", &currValue);
    if(oldValue != currValue){
      setShadowsEnabled(currValue);
    }
  }
  {
    auto currValue = isExposureEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("exposure", &currValue);
    if(oldValue != currValue){
      setExposureEnabled(currValue);
    }
  }
  {
    auto currValue = isGammaEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("gamma", &currValue);
    if(oldValue != currValue){
      setGammaEnabled(currValue);
    }
  }
  {
    auto currValue = isSkyboxEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("skybox", &currValue);
    if(oldValue != currValue){
      setSkyboxEnabled(currValue);
    }
  }
  {

    auto tint = skyboxColor();
    float color[3] = {tint.r, tint.g, tint.b};
    if (ImGui::ColorEdit3("Skybox Color", color)){
      setSkyboxColor(glm::vec3(color[0], color[1], color[2]));
    }
  }



  {
    auto currValue = isCullEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("cull", &currValue);
    if(oldValue != currValue){
      setCullEnabled(currValue);
    }
  }
  {
    auto currValue = isFogEnabled();
    auto oldValue = currValue;
    ImGui::Checkbox("fog", &currValue);
    if(oldValue != currValue){
      setFogEnabled(currValue);
    }

    { 
      auto minFog = fogMinCutoff();
      auto maxFog = fogMaxCutoff();
      if(ImGui::SliderFloat("Min Fog", &minFog, 0.0f, 10.0f)){
        setFogMinCutoff(minFog);
      }
      if(ImGui::SliderFloat("Max Fog", &maxFog, 0.0f, 10.0f)){
        setFogMaxCutoff(maxFog);
      }
    }

    {
      auto tint = fogColor();
      float color[4] = {tint.r, tint.g, tint.b, tint.w};
      if (ImGui::ColorEdit4("fog color", color)){
        setFogColor(glm::vec4(color[0], color[1], color[2], color[3]));
      }
    }

  }

  {

    auto tint = ambientLight();
    float color[3] = {tint.r, tint.g, tint.b};
    if (ImGui::ColorEdit3("Ambient", color)){
      setAmbientLightColor(glm::vec3(color[0], color[1], color[2]));
    }
  }


  if (includePanel){
    ImGui::End();
  }
}

void renderTransformPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Transform Panel");
  }
 
  //enum ManipulatorMode { NONE, ROTATE, TRANSLATE, SCALE };
  auto mode = getManipulatorMode();

  bool isTranslateMode = mode == TRANSLATE;
  bool wasTranslateMode = isTranslateMode;
  ImGui::Checkbox("Translate", &isTranslateMode);
  ImGui::SameLine();

  bool isMirror = isTranslateMirror();
  ImGui::Checkbox("Mirror", &isMirror);
  setTranslateMirror(isMirror);

  bool isScaleMode = mode == SCALE;
  bool wasScaleMode = isScaleMode;
  ImGui::Checkbox("Scale", &isScaleMode);
  ImGui::SameLine();

  auto uniformScale = isUniformScale();
  ImGui::Checkbox("Uniform", &uniformScale);
  setUniformScale(uniformScale);

  bool isRotateMode = mode == ROTATE;
  bool wasRotateMode = isRotateMode;
  ImGui::Checkbox("Rotate", &isRotateMode);

  if (!wasTranslateMode && isTranslateMode){
    setManipulatorMode(TRANSLATE);
  }else if (!wasScaleMode && isScaleMode){
    setManipulatorMode(SCALE);
  }else if (!wasRotateMode && isRotateMode){
    setManipulatorMode(ROTATE);
  }

  auto axis = getManipulatorAxis();
  std::string axisString = "none";
  if (axis == XAXIS){
    axisString = "X";
  }else if (axis == YAXIS){
    axisString = "Y";
  }else if (axis == ZAXIS){
    axisString = "Z";
  }
  if (ImGui::BeginCombo("Axis", axisString.c_str())){
    auto isXAxis = axis == XAXIS;
    auto isYAxis = axis == YAXIS;
    auto isZAxis = axis == ZAXIS;
    if (ImGui::Selectable("X", isXAxis)){
      setManipulatorAxis(XAXIS);
    }
    if (ImGui::Selectable("Y", isYAxis)){
      setManipulatorAxis(YAXIS);
    }
    if (ImGui::Selectable("Z", isZAxis)){
      setManipulatorAxis(ZAXIS);
    }
    ImGui::EndCombo();
  }


  if(ImGui::Button("-X")){
    sendManipulatorEvent(OBJECT_ORIENT_LEFT);
  }
  ImGui::SameLine();
  if(ImGui::Button("+X")){
    sendManipulatorEvent(OBJECT_ORIENT_RIGHT);
  }
  ImGui::SameLine();
  if(ImGui::Button("-Y")){
    sendManipulatorEvent(OBJECT_ORIENT_DOWN);
  }
  ImGui::SameLine();
  if(ImGui::Button("+Y")){
    sendManipulatorEvent(OBJECT_ORIENT_UP);
  }
  ImGui::SameLine();
  if(ImGui::Button("-Z")){
    sendManipulatorEvent(OBJECT_ORIENT_FORWARD);
  }
  ImGui::SameLine();
  if(ImGui::Button("+Z")){
    sendManipulatorEvent(OBJECT_ORIENT_BACK);
  }

  ImGui::Dummy(ImVec2(0, 10));
  ImGui::Checkbox("Show Grid", &isTranslateMode);

  auto isGroup = isGroupSelection();
  ImGui::Checkbox("Group Selection", &isGroup);
  setGroupSelection(isGroup);

  ImGui::Dummy(ImVec2(0, 10));

  {
    auto isContinuousTranslate = getModeTranslate() == SNAP_CONTINUOUS;
    auto oldIsContinuousTranslate = isContinuousTranslate;
    auto isAbsoluteTranslate = getModeTranslate() == SNAP_ABSOLUTE;
    auto oldIsAbsoluteTranslate = isAbsoluteTranslate;
    auto isRelativeTranslate = getModeTranslate() == SNAP_RELATIVE;
    auto oldIsRelativeTranslate = isRelativeTranslate;

    ImGui::Checkbox("Continuous Translate", &isContinuousTranslate);
    ImGui::Checkbox("Absolute Translate", &isAbsoluteTranslate);
    ImGui::Checkbox("Relative Translate", &isRelativeTranslate);
      
    if (isContinuousTranslate && (isContinuousTranslate != oldIsContinuousTranslate)){
      setModeTranslate(SNAP_CONTINUOUS);
    }else if (isAbsoluteTranslate && (isAbsoluteTranslate != oldIsAbsoluteTranslate)){
      setModeTranslate(SNAP_ABSOLUTE);
    }else if (isRelativeTranslate && (isRelativeTranslate != oldIsRelativeTranslate)){
      setModeTranslate(SNAP_RELATIVE);
    }else if (!isContinuousTranslate && !isAbsoluteTranslate && !isRelativeTranslate){
      setModeTranslate(SNAP_CONTINUOUS);
    }
  }


  ImGui::Dummy(ImVec2(0, 10));

  auto isAbsoluteRotate = getModeRotate() == SNAP_ABSOLUTE;
  auto oldIsAbsoluteRotate = isAbsoluteRotate;
  ImGui::Checkbox("Absolute Rotation", &isAbsoluteRotate);
  if (isAbsoluteRotate != oldIsAbsoluteRotate){
    if (isAbsoluteRotate){
      setModeRotate(SNAP_ABSOLUTE);
    }else{
      setModeRotate(SNAP_CONTINUOUS);
    }
  }

/*
  put angles here
          .options = { "0.01", "0.1", "0.5", "1", "5" },
        .onClick = optionsOnClick("editor", "snaptranslate", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snaptranslate", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
      },
      DockOptionConfig {  // "Snap Scales",
        .options = { "0.01", "0.1", "0.5", "1", "5" },
        .onClick = optionsOnClick("editor", "snapscale", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snapscale", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
      DockOptionConfig { // Snap Rotation
        .options = { "1", "5", "15", "30", "45", "90", "180" },
        .onClick = optionsOnClick("editor", "snapangle", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snapangle", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }),
      },*/


  if (includePanel){
    ImGui::End();
  } 
}

void renderTextures(bool includePanel, std::optional<objid> objectToDetail){
  if (includePanel){
    ImGui::Begin("Textures Panel");
  }

  if (objectToDetail.has_value()){
    auto id = objectToDetail.value();
    auto objType = getObjectType(id);
    if (objType == OBJ_MESH){
      static float position[3] = {0};
      //position[0] = objPosition.x;
      //position[1] = objPosition.y;
      //position[2] = objPosition.z ;

      auto textureSize = getGameObjectTextureSize(id);
      auto textureTiling = getGameObjectTextureTiling(id);
      auto textureOffset = getGameObjectTextureOffset(id);

      ImGui::Text("Size");
      ImGui::PushItemWidth(70);
      ImGui::DragFloat("##X-size", &textureSize.x, 0.1f);
      ImGui::SameLine();
      ImGui::DragFloat("##Y-size", &textureSize.y, 0.1f);
      ImGui::PopItemWidth();

      ImGui::Text("Tiling");
      ImGui::PushItemWidth(70);
      ImGui::DragFloat("##X-tiling", &textureTiling.x, 0.1f);
      ImGui::SameLine();
      ImGui::DragFloat("##Y-tiling", &textureTiling.y, 0.1f);
      ImGui::PopItemWidth();

      ImGui::Text("Offset");
      ImGui::PushItemWidth(70);
      ImGui::DragFloat("##X-offset", &textureOffset.x, 0.1f);
      ImGui::SameLine();
      ImGui::DragFloat("##Y-offset", &textureOffset.y, 0.1f);
      ImGui::PopItemWidth();

      setGameObjectTextureSize(id, textureSize);
      setGameObjectTextureOffset(id, textureOffset);
      setGameObjectTextureTiling(id, textureTiling);

      auto textures = getTextures();

      float thumbnailSize = 64.0f;
      float spacing = ImGui::GetStyle().ItemSpacing.x;
      float panelWidth = ImGui::GetContentRegionAvail().x;
      int columns = (panelWidth + spacing) / (thumbnailSize + spacing);
      columns = std::max(columns, 1);

      for (int i = 0; i < textures.size(); i++){
        auto& texture = textures.at(i);
        if (ImGui::ImageButton(texture.name, texture.textureId, ImVec2(64, 64))){
          setGameObjectTexture(id, texture.name);
        }
        if ((i + 1) % columns != 0){
          ImGui::SameLine();
        }
      }
    }
  }

  if (includePanel){
    ImGui::End();
  } 
}



std::optional<std::string> ScenegraphView(std::string directory){
    std::optional<std::string> selectedModel;
    for (auto& entry : std::filesystem::directory_iterator(directory)){
        if (entry.is_directory()){
            if (ImGui::TreeNode(entry.path().filename().string().c_str())){
                auto model = ScenegraphView(entry.path());
                if (model.has_value()){
                  selectedModel = model;
                }
                ImGui::TreePop();
            }
        }
        else{   
            auto fileType = getFileType(entry.path().filename().string());
            auto isModel = fileType == MODEL_EXTENSION;
            if (isModel){
              if(ImGui::Selectable(entry.path().filename().string().c_str())){
                selectedModel = entry.path().string();
              }
            }
        }
    }
    return selectedModel;
}

void renderModelPanel(bool includePanel, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Model Panel");
  }

  if (sceneId.has_value()){
    auto selectedModel = ScenegraphView("../gameresources/build/");
    if (selectedModel.has_value()){
      std::cout << "renderModelPanel: " << print(selectedModel) << std::endl;
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
      GameobjAttributes attr { .attr = {
        {"mesh", selectedModel.value()}
      }};
      mainApi -> makeObjectAttr(sceneId.value(), std::string("mesh-") + uniqueNameSuffix(), attr, submodelAttributes);
    }    
  }


  if (includePanel){
    ImGui::End();
  } 
}

void renderParticlePanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Particle Panel");
  }
/*
      DockButtonConfig {
        .buttonText = "Emit One",
        .onClick = []() -> void {
          dockConfigApi.emitParticleViewerParticle();
        },
      },
      DockCheckboxConfig {
        .label = "Emit Particles",
        .isChecked = []() -> bool {
          return dockConfigApi.getParticlesViewerShouldEmit();
        },
        .onChecked = [](bool isChecked) -> void { 
          dockConfigApi.setParticlesViewerShouldEmit(isChecked);
        },
      },
      DockTextboxNumeric {
        .label = "rate",
        .value = floatParticleGetValue("rate"),
        .onEdit = floatParticleSetValue("rate"),
      },
      DockTextboxNumeric {
        .label = "duration",
        .value = floatParticleGetValue("duration"),
        .onEdit = floatParticleSetValue("duration"),
      },
      DockTextboxNumeric {
        .label = "limit",
        .value = floatParticleGetValue("limit"),
        .onEdit = floatParticleSetValue("limit"),
      },
      DockGroup {
        .groupName = "Base Particle",
        .onClick = createCollapsableOnClick("particle-base"),
        .collapse = createShouldBeCollapse("particle-base"),
        .configFields = {
          DockCheckboxConfig {
            .label = "enable physics",
            .isChecked = floatParticleGetValueBool("+physics", "enabled", "disabled"),
            .onChecked = floatParticleSetValueBool("+physics", "enabled", "disabled"),
          },    
          DockCheckboxConfig {
            .label = "enable collision",
            .isChecked = floatParticleGetValueBool("+physics_collision", "collide", "nocollide"),
            .onChecked = floatParticleSetValueBool("+physics_collision", "collide", "nocollide"),
          },    
          DockColorPickerConfig {
            .label = "tint",
            .getColor = []() -> glm::vec4 { 
              auto attr = dockConfigApi.getParticleAttribute("+tint");
              if (!attr.has_value()){
                return glm::vec4(0.f, 0.f, 0.f, 0.f);
              }
              auto vec4Value = std::get_if<glm::vec4>(&attr.value());
              modassert(vec4Value, "has tint but not a vec4");
              return *vec4Value;
            },
            .onColor = [](glm::vec4 color) -> void {
              dockConfigApi.setParticleAttribute("+tint", color);
            },
          },
          DockImageConfig {
            .label =  "texture",
            .onImageSelect = [](std::string texture) -> void {
              dockConfigApi.setParticleAttribute("+texture", texture);
            }
          },
          DockTextboxNumeric {
            .label = "gravity-x",
            .value = floatParticleGetValueVec3("+physics_gravity", 0),
            .onEdit = floatParticleSetValueVec3("+physics_gravity", 0),
          },
          DockTextboxNumeric {
            .label = "gravity-y",
            .value = floatParticleGetValueVec3("+physics_gravity", 1),
            .onEdit = floatParticleSetValueVec3("+physics_gravity", 1),
          },
          DockTextboxNumeric {
            .label = "gravity-z",
            .value = floatParticleGetValueVec3("+physics_gravity", 2),
            .onEdit = floatParticleSetValueVec3("+physics_gravity", 2),
          },
          DockTextboxNumeric {
            .label = "velocity-x",
            .value = floatParticleGetValueVec3("+physics_velocity", 0),
            .onEdit = floatParticleSetValueVec3("+physics_velocity", 0),
          },
          DockTextboxNumeric {
            .label = "velocity-y",
            .value = floatParticleGetValueVec3("+physics_velocity", 1),
            .onEdit = floatParticleSetValueVec3("+physics_velocity", 1),
          },
          DockTextboxNumeric {
            .label = "velocity-z",
            .value = floatParticleGetValueVec3("+physics_velocity", 2),
            .onEdit = floatParticleSetValueVec3("+physics_velocity", 2),
          },
          DockTextboxNumeric {
            .label = "scale-x",
            .value = floatParticleGetValueVec3("+scale", 0),
            .onEdit = floatParticleSetValueVec3("+scale", 0),
          },
          DockTextboxNumeric {
            .label = "scale-y",
            .value = floatParticleGetValueVec3("+scale", 1),
            .onEdit = floatParticleSetValueVec3("+scale", 1),
          },
          DockTextboxNumeric {
            .label = "scale-z",
            .value = floatParticleGetValueVec3("+scale", 2),
            .onEdit = floatParticleSetValueVec3("+scale", 2),
          },
        },
      },
      DockGroup {
        .groupName = "Particle Values",
        .onClick = createCollapsableOnClick("particle-values"),
        .collapse = createShouldBeCollapse("particle-values"),
        .configFields = {
          DockTextboxNumeric {
            .label = "position-x",
            .value = floatParticleGetValueVec3("!position", 0),
            .onEdit = floatParticleSetValueVec3("!position", 0),
          },
          DockTextboxNumeric {
            .label = "position-y",
            .value = floatParticleGetValueVec3("!position", 1),
            .onEdit = floatParticleSetValueVec3("!position", 1),
          },
          DockTextboxNumeric {
            .label = "position-z",
            .value = floatParticleGetValueVec3("!position", 2),
            .onEdit = floatParticleSetValueVec3("!position", 2),
          },
          DockTextboxNumeric {
            .label = "scale-x",
            .value = floatParticleGetValueVec3("!scale", 0),
            .onEdit = floatParticleSetValueVec3("!scale", 0),
          },
          DockTextboxNumeric {
            .label = "scale-y",
            .value = floatParticleGetValueVec3("!scale", 1),
            .onEdit = floatParticleSetValueVec3("!scale", 1),
          },
          DockTextboxNumeric {
            .label = "scale-z",
            .value = floatParticleGetValueVec3("!scale", 2),
            .onEdit = floatParticleSetValueVec3("!scale", 2),
          },
        }
      }, 
      DockGroup {
        .groupName = "Particle Variance",
        .onClick = createCollapsableOnClick("particle-variance"),
        .collapse = createShouldBeCollapse("particle-variance"),
        .configFields = {
          DockTextboxNumeric {
            .label = "position",
            .value = []() -> std::string{ return "1.0"; },
            .onEdit = [](float, std::string&) -> void { },
          },
          DockTextboxNumeric {
            .label = "scale",
            .value = []() -> std::string{ return "1.0"; },
            .onEdit = [](float, std::string&) -> void { },
          },
        }
      }
    }
    */
  if (includePanel){
    ImGui::End();
  }  
}


///// these are game specific, so should be moved, theyre just mocked here for now

void renderBallGameplay(bool includePanel){
  if (includePanel){
    ImGui::Begin("Ball Gameplay");
  }
  static bool doThing = false;

  static float speed = 0.f;
  ImGui::DragFloat("jump", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("magnitude", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("torque", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("jump-magnitude", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("mass", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("friction", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("restitution", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("gravity", &speed, 0.0f, 10.0f);

  if (includePanel){
    ImGui::End();
  }
}
void renderMovementPanel(bool includePanel){

  /*
      createSimpleTextboxNumeric("traits", "Speed", "speed", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Speed Air", "speed-air", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Jump Height", "jump-height", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Gravity", "gravity", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Mass", "mass", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Friction", "friction", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Restitution", "restitution", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleCheckbox("traits", "Crouch", "crouch", []() -> SqlFilter { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleCheckbox("traits", "Move Vertical", "move-vertical", []() -> SqlFilter { return SqlFilter { .column = "profile", .value = "default" }; }),
  */

  if (includePanel){
    ImGui::Begin("Movement Gameplay");
  }

  static float speed = 0.f;
  ImGui::DragFloat("Speed", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Speed Air", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Jump Height", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Gravity", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Mass", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Friction", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Restitution", &speed, 0.0f, 10.0f);

  bool enabled = false;
  ImGui::Checkbox("Crouch", &enabled);
  ImGui::Checkbox("Move Vertical", &enabled);

  if (includePanel){
    ImGui::End();
  } 
}

void renderWeaponsPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Weapons Gameplay");
  }

/*
    .title = "WEAPONS",
    .configFields = {
      DockSelectConfig {
        .selectOptions = SelectOptions {
          .getOptions = []() -> std::vector<std::string>& {
            static std::vector<std::string> options = listGuns();
            return options;
          },
          .toggleExpanded = [](bool expanded) -> void {
            weaponsExpanded = expanded;
          },
          .onSelect = [](int index, std::string& gun) -> void {
            weaponSelectIndex = index;
            weaponsExpanded = false;
            selectedGun = gun;
            modlog("editor gun", std::string("selected gun: ") + gun);
          },
          .currentSelection = []() -> int { return weaponSelectIndex; },
          .isExpanded = []() -> bool { return weaponsExpanded; },
        }
      },
      createSimpleGunCheckbox("Ironsight", "ironsight"),
      createSimpleGunCheckbox("Raycast", "raycast"),
      createSimpleGunCheckbox("Hold", "hold"),
      createSimpleTextboxNumeric("guns","Bloom", "bloom"),
      createSimpleTextboxNumeric("guns","Min Bloom", "minbloom"),
      createSimpleTextboxNumeric("guns","Bloom Length", "bloom-length"),
      createSimpleTextboxNumeric("guns","Horizontal Sway", "bloom-length"),
      createSimpleTextboxNumeric("guns","Vertical Sway", "bloom-length"),
    }
  },
  */

  {
    if (ImGui::Button("Rename")){
      ImGui::OpenPopup("Rename");
    }
  
    if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
        std::string name = "";
        ImGui::InputText("Name", &name);
        if (ImGui::Button("OK"))
        {
            std::cout << "create weapon: " << name << std::endl;
    
            ImGui::CloseCurrentPopup();
        }
    
        ImGui::SameLine();
    
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
    
        ImGui::EndPopup();
    }
  }
  {
    if (ImGui::Button("Delete Weapon")){
      ImGui::OpenPopup("Confirm Delete Weapon");
    }
    if (ImGui::BeginPopupModal("Confirm Delete Weapon", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
      ImGui::Text("Are you sure?");
      if (ImGui::Button("OK")){
          ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")){
          ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  std::vector<std::string> weapons {
    "weapon_one",
    "weapon_two",
  };

  int selectedWeapon = 0;

  if (ImGui::BeginCombo("Weapon", weapons.at(selectedWeapon).c_str())){
      for (int i = 0; i < weapons.size(); i++){
          bool selected = (selectedWeapon == i);
          if (ImGui::Selectable(weapons.at(i).c_str(), selected)){
             selectedWeapon = i;
          }
          if (selected){
            ImGui::SetItemDefaultFocus();
          }
      }
      ImGui::EndCombo();
  }

  bool enabled = false;

  ImGui::Checkbox("Ironsight", &enabled);
  ImGui::Checkbox("Raycast", &enabled);
  ImGui::Checkbox("Hold", &enabled);

  static float speed = 0.f;
  ImGui::DragFloat("Bloom", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Min Bloom", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Bloom Length", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Horizontal Sway", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Vertical Sway", &speed, 0.0f, 10.0f);


  if (includePanel){
    ImGui::End();
  } 
}

void renderSpawnPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Spawn");
  }

/*
     DockButtonConfig {
        .buttonText = "Create Spawnpoint",
        .onClick = []() -> void {
          std::string spawnpointFile("../afterworld/scenes/prefabs/gameplay/spawnpoint.rawscene");
          std::unordered_map<std::string, AttributeValue> attrs;
          attrs["+spawnpoint|spawn"] = std::string("|") + enemyTypes.at(0);
          dockConfigApi.createPrefab(spawnpointFile, attrs);
        },
      },
      DockOptionConfig {
        .options = enemyTypes,
        .onClick = [](std::string&, int index) -> void {
          dockConfigApi.setObjAttr("+spawnpoint|spawn", std::string("|") + enemyTypes.at(index));
        },
        .getSelectedIndex = [](void) -> int {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawn");
          if (!attr.has_value()){
            return -1;
          }
          auto spawnStr = std::get_if<std::string>(&attr.value());
          modassert(spawnStr, "invalid type for spawnStr");

          for (int i = 0; i < enemyTypes.size(); i++){
            if (enemyTypes.at(i) == spawnStr -> substr(1, spawnStr -> size())){
              return i;
            }
          }
          return -1;
        }
      },
      DockCheckboxConfig {
        .label = "Spawn On Load",
        .isChecked = []() -> bool {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return false;
          }
          auto value = std::get_if<std::string>(&attr.value());
          if (value == NULL){
            return false;
          }
          return *value == "|onload";
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", "|onload");
          }else{
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", DeleteAttribute{});
          }
        },
      },
      DockCheckboxConfig {
        .label = "Enable Spawn Tag",
        .isChecked = []() -> bool {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return false;
          }
          auto strValue = std::get_if<std::string>(&attr.value());
          modassert(strValue, "enable spawn tag wrong type");
          auto values = split(strValue -> substr(1, strValue -> size()), ',');
          for (auto &value : values){
            if (value != "onload"){
              return true;
            }
          }
          return false;
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", "|default");
          }else{
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", DeleteAttribute{});
          }
        },
      },
      DockTextboxConfig {
        .label = "Spawn Tag",
        .text = []() -> std::string {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return "[disabled]";
          }
          auto strValue = std::get_if<std::string>(&attr.value());
          modassert(strValue, "invalid type for spawn tag");
          auto body = strValue -> substr(1, strValue -> size());
          if (body == "onload"){
            return "[disabled]";
          }
          return body; 
        },
        .onEdit = [](std::string value) -> void {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return;
          }
          auto strValue = std::get_if<std::string>(&attr.value());
          modassert(strValue, "invalid type for spawn tag");
          auto body = strValue -> substr(1, strValue -> size());
          if (body == "onload"){
            return;
          }
          dockConfigApi.setObjAttr("+spawnpoint|spawntags", std::string("|") + value);
        }
      },
      */
  if (includePanel){
    ImGui::End();
  }   
}

void renderTriggerPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Trigger");
  }

/*
      DockButtonConfig {
        .buttonText = "Create Trigger",
        .onClick = []() -> void {
          std::unordered_map<std::string, AttributeValue> attrs;
          std::string triggerFile("../afterworld/scenes/prefabs/gameplay/trigger.rawscene");
          dockConfigApi.createPrefab(triggerFile, attrs);
        },
      },
      DockCheckboxConfig {
        .label = "Oneshot",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-remove");
          if (!value.has_value()){
            return false;
          }
          auto strValue = std::get_if<std::string>(&value.value());
          if (strValue == NULL){
            return false;
          }
          return *strValue == "|enter";
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-remove", "|enter");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-remove", DeleteAttribute{});
          }
        },
      },
      DockCheckboxConfig {
        .label = "On Enter",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-enter");
          return value.has_value();
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-enter", "|enter");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-enter", DeleteAttribute{});
          }   
        },
      },
      DockTextboxConfig {
        .label = "On Enter Key",
        .text = []() -> std::string {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-enter");
          if (!value.has_value()){
            return "[disabled]";
          }
          auto attrValue = value.value();
          auto strValue = std::get_if<std::string>(&attrValue);
          modassert(strValue, "invalid type onEnterKey");
          return strValue -> substr(1, strValue -> size());
        },
        .onEdit = [](std::string value) -> void {
          auto enterValue = dockConfigApi.getObjAttr("+trigger|switch-enter");
          if (!enterValue.has_value()){
            return;
          }
          dockConfigApi.setObjAttr("+trigger|switch-enter", std::string("|") + value);
        }
      },
      DockCheckboxConfig {
        .label = "On Exit",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-exit");
          return value.has_value();
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-exit", "|exit");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-exit", DeleteAttribute{});
          }   
        },
      },
      DockTextboxConfig {
        .label = "On Exit Key",
        .text = []() -> std::string {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-exit");
          if (!value.has_value()){
            return "[disabled]";
          }
          auto attrValue = value.value();
          auto strValue = std::get_if<std::string>(&attrValue);
          modassert(strValue, "invalid type onEnterKey");
          return strValue -> substr(1, strValue -> size());
        },
        .onEdit = [](std::string value) -> void {
          auto enterValue = dockConfigApi.getObjAttr("+trigger|switch-exit");
          if (!enterValue.has_value()){
            return;
          }
          dockConfigApi.setObjAttr("+trigger|switch-exit", std::string("|") + value);
        }
      },
    }
    */
  if (includePanel){
    ImGui::End();
  }   
}