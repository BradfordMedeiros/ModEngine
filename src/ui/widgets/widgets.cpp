#include "./widgets.h"

extern CustomApiBindings* mainApi;
extern DefaultResources defaultResources;
extern Stats statistics;
extern engineState state;

std::vector<std::string> getAllShaders();
double timeSeconds(bool realtime);


std::optional<std::string> ScenegraphView(std::string directory, FILE_EXTENSION_TYPE type){
    std::optional<std::string> selectedModel;
    for (auto& entry : std::filesystem::directory_iterator(directory)){
        if (entry.is_directory()){
            if (ImGui::TreeNode(entry.path().filename().string().c_str())){
                auto model = ScenegraphView(entry.path(), type);
                if (model.has_value()){
                  selectedModel = model;
                }
                ImGui::TreePop();
            }
        }
        else{   
            auto fileType = getFileType(entry.path().filename().string());
            auto isModel = fileType == type;
            if (isModel){
              if(ImGui::Selectable(entry.path().filename().string().c_str())){
                selectedModel = entry.path().string();
              }
            }
        }
    }
    return selectedModel;
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


///// these are game specific, so should be moved, theyre just mocked here for now


enum DisplayRenderType { DEFAULT_RENDER, TEST_RENDER, DEPTH_RENDER };
DisplayRenderType renderType = DEFAULT_RENDER;

void renderDisplayBinding(bool includePanel){
  if (includePanel){
    ImGui::Begin("Display Panel");
  }

  bool wasDefault = renderType == DEFAULT_RENDER;
  bool isDefault = wasDefault;
  ImGui::Checkbox("Default", &isDefault);

  bool wasTestRender = renderType == TEST_RENDER;
  bool isTestRender = wasTestRender;
  ImGui::Checkbox("Test", &isTestRender);

  bool wasDepthRender = renderType == DEPTH_RENDER;
  bool isDepthRender = wasDepthRender;
  ImGui::Checkbox("Depth", &isDepthRender);
  

  if (wasDefault != isDefault && isDefault){
    renderType = DEFAULT_RENDER;
    mainApi -> createViewport(0, 0.f, 0.f, 1.f, 1.f, DefaultBindingOption{}, {});
    mainApi -> removeViewport(1);
    mainApi -> removeViewport(2);

  } 
  if (wasTestRender != isTestRender && isTestRender){
    renderType = TEST_RENDER;
    mainApi -> createViewport(0, 0.f, 0.5f, 0.5f, 0.5f, DefaultBindingOption{}, {});
    mainApi -> createViewport(1, 0.5f, 0.5f, 0.5f, 0.5f, DepthBindingOption{}, {});
    mainApi -> createViewport(2, 0.0f, 0.0f, 0.5f, 0.5f, BloomBindingOption{}, {});

  }


  if (includePanel){
    ImGui::End();
  }  
}



