#include "./widgets.h"

extern CustomApiBindings* mainApi;
std::vector<std::string> getAllShaders();

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

void renderActiveScene(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Active Scene");
	}

  ImGui::Text("Active Id = [19323939]");
  ImGui::Button("Save Scene");
  ImGui::Button("Reset Scene");

  if (includePanel){
	  ImGui::End();
  }
}

void renderCreateObj(bool includePanel){
  if (includePanel){
    ImGui::Begin("Create Object");
  }

  ImGui::Button("Create Mesh");
  ImGui::Button("Create Camera");
  ImGui::Button("Create Light");
  ImGui::Button("Create Text");


  if (includePanel){
    ImGui::End();
  }
}

void renderCameraPanel(bool includePanel){
	if (includePanel){
	  ImGui::Begin("Cameras");
	}

  ImGui::Button("Create Camera");

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

  ImGui::Button("Create Light");

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