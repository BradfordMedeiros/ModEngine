#include "./obj.h"
#include "../../main_api.h"
#include "../../package.h"

extern CustomApiBindings* mainApi;

std::vector<std::string> listSoundFiles();
std::vector<std::string> listParticlesFiles();
std::optional<std::string> ScenegraphView(std::string directory, FILE_EXTENSION_TYPE type);

void renderPrefabPanel(bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Prefab");
  }

  auto prefabFiles = listFilesWithExtensionsFromPackage("../afterworld/scenes/prefabs", { "rawscene" });
  static std::string prefabPath;
  if (prefabPath.empty() && !prefabFiles.empty()){
    prefabPath = prefabFiles.at(0);
  }

  ImGui::Text("Create Prefab");
  if (prefabFiles.empty()){
    ImGui::TextDisabled("No .rawscene prefabs found");
  }else if (ImGui::BeginCombo("Source", prefabPath.c_str())){
    for (auto& prefabFile : prefabFiles){
      bool selected = prefabPath == prefabFile;
      if (ImGui::Selectable(prefabFile.c_str(), selected)){
        prefabPath = prefabFile;
      }
      if (selected){
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  if (ImGui::Button("Create at Camera")){
    if (sceneId.has_value() && !prefabPath.empty()){
      GameobjAttributes attr {
        .attr = {
          { "scene", prefabPath },
          { "position", createLocation() },
        },
      };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
      mainApi -> makeObjectAttr(
        sceneId.value(),
        std::string("[prefab-instance-") + uniqueNameSuffix(),
        attr,
        submodelAttributes
      );
    }
  }
  if (!sceneId.has_value()){
    ImGui::TextDisabled("No active scene");
  }

  ImGui::Separator();
  ImGui::Text("Selected Prefab");
  if (!objectToDetail.has_value()){
    ImGui::TextDisabled("Select a prefab");
  }else{
    auto selectedId = objectToDetail.value();
    auto prefabId = mainApi -> prefabId(selectedId);
    auto rootId = prefabId.value_or(selectedId);
    auto prefabPathValue = getObjectAttribute(rootId, "scene");
    auto prefabPathValueString = prefabPathValue.has_value() ? std::get_if<std::string>(&prefabPathValue.value()) : nullptr;

    if (prefabPathValueString == nullptr){
      ImGui::TextDisabled("Selected object is not a prefab instance");
    }else{
      ImGui::Text("Root ID: %d", rootId);
      ImGui::TextWrapped("Source: %s", prefabPathValueString -> c_str());

      auto children = mainApi -> getChildrenIdsAndParent(rootId);
      ImGui::Separator();
      ImGui::Text("Objects: %zu", children.size() + 1);

      bool showPrefabObjects = ImGui::BeginChild("PrefabObjects", ImVec2(0.f, 0.f), true);
      if (showPrefabObjects){
        auto rootName = mainApi -> getGameObjNameForId(rootId);
        if (rootName.has_value()){
          ImGui::BulletText("%s", rootName.value().c_str());
        }

        for (auto childId : children){
          auto childName = mainApi -> getGameObjNameForId(childId);
          if (childName.has_value()){
            ImGui::BulletText("%s", childName.value().c_str());
          }
        }
      }
      ImGui::EndChild();
    }
  }

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

void renderSoundPanel(bool includePanel, std::optional<objid> objectToDetail){
  if (includePanel){
    ImGui::Begin("Sound");
  }

  if (objectToDetail.has_value() && getObjectType(objectToDetail.value()) == OBJ_SOUND){
    auto id = objectToDetail.value();

    bool isAutoplay = isGameObjectAutoplay(id);
    ImGui::Checkbox("Autoplay", &isAutoplay);
    setGameObjectAutoplay(id, isAutoplay);

    bool isLoop = isGameObjectLoop(id);
    ImGui::Checkbox("Loop", &isLoop);
    setGameObjectLoop(id, isLoop);

    bool isCenter = isGameObjectCenter(id);
    ImGui::Checkbox("Center", &isCenter);
    setGameObjectCenter(id, isCenter);

    float volume = getGameObjectSoundVolume(id);
    ImGui::SliderFloat("Volume", &volume, 0.f, 1.f);
    setGameObjectSoundVolume(id, volume);
    
    std::vector<std::string> clips = listSoundFiles();

    auto clip = getGameObjectSoundClip(id);

    if (ImGui::BeginCombo("Clip", clip.c_str())){
        for (int i = 0; i < clips.size(); i++){
            bool selected = clip == clips.at(i);
            if (ImGui::Selectable(clips.at(i).c_str(), selected)){
              setGameObjectSoundClip(id, clips.at(i));
            }
            if (selected){
              ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }


    if(ImGui::Button("Play Sound")){
      mainApi -> playOneshot(id, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, id);
    }
  /*
  AutoSerializeCustom {
    .structOffset = 0,
    .field = "clip",
    .fieldType = ATTRIBUTE_STRING,
    .deserialize = [](void* offset, void* fieldValue) -> void {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      std::string* clip = static_cast<std::string*>(fieldValue);
      if (fieldValue == NULL){
        modassert(false, "clip must not be unspecified");
      }else{
        obj -> clip = *clip;
        obj -> source = loadSoundState(obj -> clip);     
      }
    },
    .setAttributes = [](void* offset, void* fieldValue) -> void {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      std::string* clip = static_cast<std::string*>(fieldValue);
      if (fieldValue != NULL){
        unloadSoundState(obj -> source, obj -> clip); 
        obj -> clip = *clip;
        obj -> source = loadSoundState(obj -> clip);     
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      GameObjectSound* obj = static_cast<GameObjectSound*>(offset);
      return obj -> clip;
    },
  },*/
  }else{
    ImGui::Text("Select a sound object");
  }

  if (includePanel){
    ImGui::End();
  } 
}

void renderLightPanel(bool includePanel, std::optional<objid> objectToDetail){
	if (includePanel){
	  ImGui::Begin("Light");
	}

  auto type = objectToDetail.has_value() ? getObjectType(objectToDetail.value()) : OBJ_INVALID;
  if (type == OBJ_LIGHT){
    auto id = objectToDetail.value();
    auto lightType = getGameObjectLightType(objectToDetail.value());

    auto isPoint = lightType == LIGHT_POINT;
    auto wasPoint = isPoint;
    auto isSpotlight = lightType == LIGHT_SPOTLIGHT;
    auto wasSpotlight = isSpotlight;
    auto isDirectional = lightType == LIGHT_DIRECTIONAL;
    auto wasDirectional = isDirectional;

    static bool showOption = false;
    ImGui::Text("Type");

    ImGui::Checkbox("Point", &isPoint);
    ImGui::SameLine();
    ImGui::Checkbox("Spotlight", &isSpotlight);
    ImGui::SameLine();
    ImGui::Checkbox("Directional", &isDirectional);

    if (isPoint && !wasPoint){
      setGameObjectLightType(id, LIGHT_POINT);
    }else if (isSpotlight && !wasSpotlight){
      setGameObjectLightType(id, LIGHT_SPOTLIGHT);
    }else if (isDirectional && !wasDirectional){
      setGameObjectLightType(id, LIGHT_DIRECTIONAL);
    }

    {
      auto tint = getGameObjectLightColor(id);
      float color[3] = {tint.r, tint.g, tint.b};
      if (ImGui::ColorEdit4("Tint", color)){
        setGameObjectLightColor(id, glm::vec3(color[0], color[1], color[2]));
      }
    }

    {
      auto attenuation = getGameObjectLightAttenutation(id);
      ImGui::DragFloat("atten-x", &attenuation.x, 0.1f);
      ImGui::DragFloat("atten-y", &attenuation.y, 0.1f);
      ImGui::DragFloat("atten-z", &attenuation.z, 0.1f);
      setGameObjectLightAttenutation(id, attenuation);

    }

  }

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

    static std::string cubemap;
    static objid cubemapObject = 0;
    if (cubemapObject != id){
      cubemapObject = id;
      cubemap = getGameObjectCubemap(id);
    }
    ImGui::Text("Cubemap");
    bool applyCubemap = ImGui::InputText("##cubemap", &cubemap, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if (ImGui::Button("Apply")){
      applyCubemap = true;
    }
    if (applyCubemap){
      setGameObjectCubemap(id, cubemap);
    }
    ImGui::TextDisabled("Directory containing the six .jpg cubemap faces");
    glm::vec3 cubemapReflection = getGameObjectCubemapReflection(id);
    if (ImGui::SliderFloat("Cubemap Strength", &cubemapReflection.x, 0.f, 3.f) ||
        ImGui::SliderFloat("Fresnel Power", &cubemapReflection.y, 0.1f, 10.f) ||
        ImGui::SliderFloat("Fresnel Base", &cubemapReflection.z, 0.f, 1.f)){
      setGameObjectCubemapReflection(id, cubemapReflection);
    }
  }

  if (includePanel){
    ImGui::End();
  }
}

void renderParticlePanel(bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId){
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

  std::string particleType = "effekseer";

  std::vector<std::string> particles = listParticlesFiles();

  if (objectToDetail.has_value()){
    auto id = objectToDetail.value();
    auto effectName = getEmitterEffect(id);
    bool isEffekseer = effectName != "";

    auto objType = getObjectType(id);
    if (objType == OBJ_EMITTER){
      if (isEffekseer){
        if (ImGui::BeginCombo("File", effectName.c_str())){
          for (auto& particle : particles){
            bool selected = particle == effectName;
            if (ImGui::Selectable(particle.c_str(), selected)){
                std::cout << "set effekseer: " << particle << std::endl;
                mainApi -> setSingleGameObjectAttr(objectToDetail.value(), "effekseer", particle);
            }
            if (selected){
              ImGui::SetItemDefaultFocus();
            }  
          }
          ImGui::EndCombo();
        }    
      }else{
        ImGui::Text("Cannot configure non-effekseer");
      }
    }


    {
      auto tintValue = getEmitterEffectTint(id);
      auto tint = tintValue.has_value() ? tintValue.value() : glm::vec4(1.f, 1.f, 1.f, 1.f);
      float color[4] = {tint.r, tint.g, tint.b, tint.a};
      if (ImGui::ColorEdit4("Tint", color)){
        setEmitterEffectTint(id, glm::vec4(color[0], color[1], color[2], color[3]));
      }
    }

  }

  if (includePanel){
    ImGui::End();
  }  
}

void renderTextPanel(bool includePanel, std::optional<objid> objectToDetail){
  if (includePanel){
    ImGui::Begin("Mesh");
  }

  ImGui::Text("Text");
  if (objectToDetail.has_value()){
    auto id = objectToDetail.value();

    auto text = getGameObjectText(id);
    ImGui::Text(text.c_str());
    ImGui::InputText("Rename Object", &text);
    setGameObjectText(id, text);


    {
      auto tint = getGameObjectTextTint(id);
      float color[4] = {tint.r, tint.g, tint.b, tint.a};
      if (ImGui::ColorEdit4("Tint", color)){
        setGameObjectTextTint(id, glm::vec4(color[0], color[1], color[2], color[3]));
      }
    }

    {
      auto textWrap = getGameObjectTextWrap(id);

      bool wrapNoneEnabled = textWrap.type == WRAP_NONE;
      bool oldWrapNoneEnabled = wrapNoneEnabled;
      bool wrapCharEnabled = textWrap.type == WRAP_CHARACTERS;
      bool oldWrapCharEnabled = wrapCharEnabled;

      ImGui::Checkbox("None", &wrapNoneEnabled);
      ImGui::SameLine();
      ImGui::Checkbox("Char", &wrapCharEnabled);

      if (wrapNoneEnabled && (wrapNoneEnabled != oldWrapNoneEnabled)){
        textWrap.type = WRAP_NONE;
      }else if (wrapCharEnabled && (wrapCharEnabled != oldWrapCharEnabled)){
        textWrap.type = WRAP_CHARACTERS;
      }else if (!wrapNoneEnabled && !wrapCharEnabled){
        textWrap.type = WRAP_NONE;
      }

      ImGui::SliderFloat("Wrap Amount", &textWrap.wrapamount, 0.f, 10.f);

      setGameObjectTextWrap(id, textWrap);
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

void renderObjPanel(bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Object Type Panel");
  }

  if (objectToDetail.has_value()){
    auto id = objectToDetail.value();
    auto type = getObjectType(id);
    if (type == OBJ_MESH){
      renderMeshPanel(false, objectToDetail);
      // enum ObjectType {, , , , , , ,  };
    }else if (type == OBJ_CAMERA){
      renderCameraPanel(false);
    }else if (type == OBJ_PORTAL){
      renderUnknownObjPanel(false);
    }else if (type == OBJ_SOUND){
      renderSoundPanel(false, objectToDetail);
    }else if (type == OBJ_LIGHT){
      renderLightPanel(false, objectToDetail);
    }else if (type == OBJ_OCTREE){
      renderUnknownObjPanel(false);
    }else if (type == OBJ_EMITTER){
      renderParticlePanel(false, objectToDetail, sceneId);
    }else if (type == OBJ_NAVMESH){
      renderUnknownObjPanel(false);
    }else if (type == OBJ_TEXT){
      renderTextPanel(false, objectToDetail);
    }else if (type == OBJ_PREFAB){
      renderPrefabPanel(false, objectToDetail, sceneId);
    }else if (type == OBJ_VIDEO){
      renderUnknownObjPanel(false);
    }else{
      renderUnknownObjPanel(false);
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
  static std::string renameError;
  if (objectId != id){
    objectId = id;
    testname = name;
    renameError.clear();
  }

  ImGui::InputText("Rename Object", &testname);
  if (ImGui::Button("Rename")){
    renameError.clear();
    if (testname.empty()){
      renameError = "Object name cannot be empty";
    }else{
      auto sceneId = mainApi -> listSceneId(id);
      auto existingId = mainApi -> getGameObjectByName(testname, sceneId);
      if (existingId.has_value() && existingId.value() != id){
        renameError = "An object with that name already exists";
      }else{
        mainApi -> renameGameObject(id, testname);
      }
    }
  }
  if (!renameError.empty()){
    ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "%s", renameError.c_str());
  }
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


glm::vec3 createLocation(){
    auto cameraTransform = mainApi -> getCameraTransform(0);
    auto location = cameraTransform.position + (cameraTransform.rotation * glm::vec3(0.f, 0.f, -1.f));
    return location;
}

void renderCreateObj(bool includePanel, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Create Object");
  }

  if (sceneId.has_value()){
    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {} };



    std::optional<objid> createdId;
    if(ImGui::Button("Create Mesh")){
      createdId = mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string("mesh-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
    if(ImGui::Button("Create Camera")){
      createdId = mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string(">camera-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
    if(ImGui::Button("Create Light")){
      createdId = mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string("!light-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }
    if(ImGui::Button("Create Text")){
      GameobjAttributes attr { .attr = { {"value", "default text" }, {  "wraptype", "char" }} };
      createdId = mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string(")text-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }

    if(ImGui::Button("Create Sound")){
      GameobjAttributes attr { .attr = { { "clip", "../gameresources/sound/q009/jumppad.ogg"} }};
      createdId = mainApi -> makeObjectAttr(
        sceneId.value(), 
        std::string("&sound-") + uniqueNameSuffix(), 
        attr, 
        submodelAttributes
      );
    }

    if(ImGui::Button("Create Particle")){
      GameobjAttributes emitterAttr { 
          .attr = {
            { "effekseer", "./res/particles/spirit-white.efkefc" },
            { "state", "enabled" },
          } 
      };
      std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
      createdId = mainApi -> makeObjectAttr(sceneId.value(), std::string("+particle"), emitterAttr, submodelAttributesEmitter);     
    }

    if (createdId.has_value()){
      mainApi -> setGameObjectPosition(createdId.value(), createLocation(), true, Hint { .hint = "[ui] - create obj set pos" });
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

void renderModelPanel(bool includePanel, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Model Panel");
  }

  if (sceneId.has_value()){
    auto selectedModel = ScenegraphView("../gameresources/build/", MODEL_EXTENSION);
    if (selectedModel.has_value()){
      std::cout << "renderModelPanel: " << print(selectedModel) << std::endl;
      std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
      GameobjAttributes attr { .attr = {
        {"mesh", selectedModel.value()}
      }};
      auto createdId = mainApi -> makeObjectAttr(sceneId.value(), std::string("mesh-") + uniqueNameSuffix(), attr, submodelAttributes);
      if (createdId.has_value()){
        mainApi -> setGameObjectPosition(createdId.value(), createLocation(), true, Hint { .hint = "[ui] - renderModelPanel set pos" });
      }
    }    
  }


  if (includePanel){
    ImGui::End();
  } 
}