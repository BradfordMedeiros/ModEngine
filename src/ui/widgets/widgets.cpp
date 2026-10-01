#include "./widgets.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

extern CustomApiBindings* mainApi;
extern DefaultResources defaultResources;
extern Stats statistics;
extern engineState state;

std::vector<std::string> getAllShaders();
double timeSeconds(bool realtime);
glm::vec3 createLocation();


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
      auto customTexture = getGameObjectCustomTexture(id);
      auto opacityTexture = getGameObjectOpacityTexture(id);
      static int textureTarget = 0;
      const char* textureTargets[] = {
        "Main texture",
        "Custom shader texture",
        "Opacity texture",
      };

      ImGui::Text("Assign texture as");
      ImGui::SetNextItemWidth(-1);
      ImGui::Combo("##texture-target", &textureTarget, textureTargets, IM_ARRAYSIZE(textureTargets));
      if (textureTarget == 1){
        ImGui::TextWrapped("Current: %s", customTexture.c_str());
      }else if (textureTarget == 2){
        ImGui::TextWrapped("Current: %s", opacityTexture.c_str());
      }

      float thumbnailSize = 64.0f;
      float spacing = ImGui::GetStyle().ItemSpacing.x;
      float panelWidth = ImGui::GetContentRegionAvail().x;
      int columns = (panelWidth + spacing) / (thumbnailSize + spacing);
      columns = std::max(columns, 1);

      for (int i = 0; i < textures.size(); i++){
        auto& texture = textures.at(i);
        if (ImGui::ImageButton(texture.name, texture.textureId, ImVec2(64, 64))){
          if (textureTarget == 0){
            setGameObjectTexture(id, texture.name);
          }else if (textureTarget == 1){
            setGameObjectCustomTexture(id, texture.name);
            customTexture = texture.name;
          }else{
            setGameObjectOpacityTexture(id, texture.name);
            opacityTexture = texture.name;
          }
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


void updateFont(int symbol, std::string path, float fontSize){
  for (auto& font : imguiFonts){
    if (font.symbol == symbol){
      font.path = path;
      font.fontSize = fontSize;
      break;
    }
  }

  ImGuiIO& io = ImGui::GetIO();

  io.Fonts -> Clear();

  for (auto& font : imguiFonts){
    font.font = io.Fonts -> AddFontFromFileTTF(
      font.path.c_str(),
      font.fontSize
    );
  }

  io.Fonts -> Build();

  // Your ImGui renderer/backend also needs to recreate its font texture
  // here, depending on how you've initialized ImGui.
}

std::vector<std::string> allFonts {

};
std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions);


void renderFontWidget(bool includePanel){
  static bool doOnce = true;
  if (doOnce){
    doOnce = false;
    allFonts = listFilesWithExtensionsFromPackage("./res/fonts/", { "ttf", "otf" });
  }
  if (includePanel){
    ImGui::Begin("Font Panel");
  }

  for (int i = 0; i < imguiFonts.size(); i++){
    auto& font = imguiFonts.at(i);
    ImGui::PushID(i);

    ImGui::Text(nameForSymbol(font.symbol).c_str());
    if (ImGui::BeginCombo("##path", font.path.c_str())){
      for (int j = 0; j < allFonts.size(); j++){
        auto& newFont = allFonts.at(j);
        if (ImGui::Selectable(newFont.c_str(), false)){
          updateFont(font.symbol, newFont, font.fontSize);
        }      
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("-")){
      font.fontSize -= 1.f;
      updateFont(font.symbol, font.path, font.fontSize);
    }
    ImGui::SameLine();
    if (ImGui::Button("+")){
      font.fontSize += 1.f;
      updateFont(font.symbol, font.path, font.fontSize);
    }
    ImGui::Text("%.0f", font.fontSize);

    ImGui::Dummy(ImVec2(0.f, 10.f));

    ImGui::PopID();
  }

  if (ImGui::Button("Save")){
    saveUiData("../afterworld/data/config/ui.json");
  }

  if (includePanel){
    ImGui::End();
  }  
}

void renderFontBindingWidget(bool includePanel){
  if (includePanel){
    ImGui::Begin("Font Binding Panel");
  }

  if (imguiFontBindings.size() == 0){
    ImGui::Text("No font bindings");
    if (includePanel){
      ImGui::End();
      return;
    }  
  }

  static int selectedBinding = 0;
  auto& imguiFontBinding = imguiFontBindings.at(selectedBinding);
  auto name = nameForSymbol(imguiFontBinding.fontBinding);

  if (ImGui::BeginCombo("Font Binding", name.c_str())){
    for (int i = 0; i < imguiFontBindings.size(); i++){
      auto& imguiFontBinding = imguiFontBindings.at(i);
      auto name = nameForSymbol(imguiFontBinding.fontBinding);
      if(ImGui::Selectable(name.c_str())){
        selectedBinding = i;
      }
    }
    ImGui::EndCombo();
  }

  auto selectedFontName = nameForSymbol(imguiFontBinding.fontSymbol);
  if (ImGui::BeginCombo("##path", selectedFontName.c_str())){
    for (int i = 0; i < imguiFonts.size(); i++){
      auto& font = imguiFonts.at(i);
      auto fontName = nameForSymbol(font.symbol);
      if (ImGui::Selectable(fontName.c_str(), false)){
        imguiFontBinding.fontSymbol = font.symbol;
      }      
    }
    ImGui::EndCombo();
  }

  if (ImGui::Button("Save")){
    saveUiData("../afterworld/data/config/ui.json");
  }

  if (includePanel){
    ImGui::End();
  }  
}

void renderColorWidget(bool includePanel){
  if (includePanel){
    ImGui::Begin("Color Panel");
  }

  for (int i = 0; i < imguiColors.size(); i++){
    auto& color = imguiColors.at(i);
    ImGui::PushID(i);

    ImGui::Text(nameForSymbol(color.symbol).c_str());
    ImGui::Text("%s", print(color.color).c_str());
    ImGui::Dummy(ImVec2(0.f, 10.f));

    auto tint = color.color;
    float colorValues[4] = {tint.r, tint.g, tint.b, tint.a};
    if (ImGui::ColorEdit4("Tint", colorValues)){
      color.color = glm::vec4(colorValues[0], colorValues[1], colorValues[2], colorValues[3]);
    }

    ImGui::PopID();
  }

  if (ImGui::Button("Save")){
    saveUiData("../afterworld/data/config/ui.json");
  }

  if (includePanel){
    ImGui::End();
  }  
}

const std::string pointEditorMeshName = "point-editor-generated-mesh";
const std::string pointEditorMeshObjectName = "mesh-point-editor-generated";

std::optional<objid> generateMeshFromPoints(const std::vector<glm::vec3>& points){
  int sides = 6;
  float radius = 0.25f;
  float pi = 3.14159265358979323846f;
  std::vector<glm::vec3> face;
  face.reserve(sides * 3);
  for (int side = 0; side < sides; side++){
    float angle = (2.f * pi * side) / sides;
    float nextAngle = (2.f * pi * (side + 1)) / sides;
    face.push_back(glm::vec3(0.f, 0.f, 0.f));
    face.push_back(glm::vec3(radius * std::cos(angle), radius * std::sin(angle), 0.f));
    face.push_back(glm::vec3(radius * std::cos(nextAngle), radius * std::sin(nextAngle), 0.f));
  }

  std::string meshName = pointEditorMeshName + uniqueNameSuffix();
  mainApi -> generateMesh(face, points, meshName);

  GameobjAttributes attributes;
  attributes.attr["mesh"] = meshName;
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  return mainApi -> makeObjectAttr(
    0,
    pointEditorMeshObjectName,
    attributes,
    submodelAttributes
  );
}


PointConfig loadPointConfig(std::string filepath){
  auto fileContent = readFileOrPackage(filepath);
  rapidjson::Document doc;
  doc.Parse(fileContent.c_str());
  modassert(!doc.HasParseError() && doc.IsObject(), "invalid point config json");

  PointConfig pointConfig;
  auto& positions = doc["position"];
  auto& rotations = doc["rotations"];
  auto& connections = doc["connections"];
  auto& names = doc["names"];

  for (rapidjson::SizeType index = 0; index < positions.Size(); index++){
    auto& position = positions[index];
    modassert(position.IsArray() && position.Size() == 3 && position[0].IsNumber() && position[1].IsNumber() && position[2].IsNumber(), "point config position must be a three-component vector");  
    pointConfig.position.emplace_back(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());

    auto& rotation = rotations[index];
    modassert(rotation.IsArray() && rotation.Size() == 4 && rotation[0].IsNumber() && rotation[1].IsNumber() && rotation[2].IsNumber() && rotation[3].IsNumber(), "point config rotation must be a four-component scene rotation");
    auto rotationQuat = parseQuat(glm::vec4(rotation[0].GetFloat(), rotation[1].GetFloat(), rotation[2].GetFloat(), rotation[3].GetFloat()));
    pointConfig.rotations.push_back(rotationQuat);

    modassert(connections[index].IsInt(), "point config connection must be an integer");
    pointConfig.connections.push_back(connections[index].GetInt());

    if (names[index].IsString()){
      pointConfig.names.push_back(names[index].GetString());
    }else if (names[index].IsNull()){
      pointConfig.names.push_back(std::nullopt);
    }else{
      modassert(false, "invalid type for name");
    }


    if (doc.HasMember("orbui")){
      auto& orbUis = doc["orbui"];
      if (orbUis[index].IsString()){
        pointConfig.orbUis.push_back(orbUis[index].GetString());
      }else if (orbUis[index].IsNull()){
        pointConfig.orbUis.push_back(std::nullopt);
      }else{
        modassert(false, "invalid type for orbui");
      }
    }
    if (doc.HasMember("levels")){
      auto& levels = doc["levels"];
      if (levels[index].IsString()){
        pointConfig.levels.push_back(levels[index].GetString());
      }else if (levels[index].IsNull()){
        pointConfig.levels.push_back(std::nullopt);
      }else{
        modassert(false, "invalid type for levels");
      }
    }
  }
  return pointConfig;
}
void savePointConfig(std::string filepath, PointConfig pointConfig){
  modassert(pointConfig.position.size() == pointConfig.rotations.size(), std::string("rotation expected size: ") + std::to_string(pointConfig.position.size()));
  modassert(pointConfig.position.size() == pointConfig.connections.size(), std::string("connections expected size: ") + std::to_string(pointConfig.position.size()));
  modassert(pointConfig.position.size() == pointConfig.names.size(), std::string("names expected size: ") + std::to_string(pointConfig.position.size()));
  modassert(pointConfig.position.size() == pointConfig.orbUis.size() || pointConfig.orbUis.size() == 0, std::string("orbUis expected size: ") + std::to_string(pointConfig.position.size()));
  modassert(pointConfig.position.size() == pointConfig.levels.size() || pointConfig.levels.size() == 0, std::string("levels expected size: ") + std::to_string(pointConfig.position.size()));


  rapidjson::Document doc;
  doc.SetObject();
  auto& allocator = doc.GetAllocator();
  rapidjson::Value positions(rapidjson::kArrayType);
  rapidjson::Value rotations(rapidjson::kArrayType);
  rapidjson::Value connections(rapidjson::kArrayType);
  rapidjson::Value names(rapidjson::kArrayType);
  rapidjson::Value orbUis(rapidjson::kArrayType);
  rapidjson::Value levels(rapidjson::kArrayType);

  for (int i = 0; i < pointConfig.position.size(); i++){
    auto& position = pointConfig.position.at(i);
    rapidjson::Value jsonPosition(rapidjson::kArrayType);
    jsonPosition.PushBack(position.x, allocator);
    jsonPosition.PushBack(position.y, allocator);
    jsonPosition.PushBack(position.z, allocator);
    positions.PushBack(jsonPosition, allocator);

    auto rotation = serializeQuatToVec4(pointConfig.rotations.at(i));
    rapidjson::Value jsonRotation(rapidjson::kArrayType);
    jsonRotation.PushBack(rotation.x, allocator);
    jsonRotation.PushBack(rotation.y, allocator);
    jsonRotation.PushBack(rotation.z, allocator);
    jsonRotation.PushBack(rotation.w, allocator);
    rotations.PushBack(jsonRotation, allocator);

    connections.PushBack(pointConfig.connections.at(i), allocator);
    if (pointConfig.names.at(i).has_value()){
      names.PushBack(rapidjson::Value(pointConfig.names.at(i).value().c_str(), allocator), allocator);
    }else{
      names.PushBack(rapidjson::Value() /* null */, allocator);
    }

    if (pointConfig.orbUis.size() > 0){
      if (pointConfig.orbUis.at(i).has_value()){
        orbUis.PushBack(rapidjson::Value(pointConfig.orbUis.at(i).value().c_str(), allocator), allocator);
      }else{
        orbUis.PushBack(rapidjson::Value() /* null */, allocator);
      }      
    }

    if (pointConfig.levels.size() > 0){
      if (pointConfig.levels.at(i).has_value()){
        levels.PushBack(rapidjson::Value(pointConfig.levels.at(i).value().c_str(), allocator), allocator);
      }else{
        levels.PushBack(rapidjson::Value() /* null */, allocator);
      }
    }
  }

  doc.AddMember("position", positions, allocator);
  doc.AddMember("rotations", rotations, allocator);
  doc.AddMember("connections", connections, allocator);
  doc.AddMember("names", names, allocator);
  doc.AddMember("orbUis", orbUis, allocator);
  doc.AddMember("levels", levels, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  realfiles::saveFile(filepath, buffer.GetString());
}

std::string print(PointConfig pointConfig){
  std::string values;
  values += "position: [";
  values += print(pointConfig.position);
  values += "]\n";

  values += "rotations: [";
  values += print(pointConfig.rotations);
  values += "]\n";

  values += "connections: [";
  values += print(pointConfig.connections);
  values += "]\n";

  values += "names: [";
  values += print(pointConfig.names);
  values += "]\n";

  values += "orbUis: [";
  values += print(pointConfig.orbUis);
  values += "]\n";

  values += "levels: [";
  values += print(pointConfig.levels);
  values += "]\n";


  return values;
}


struct OrbPointConfig {
  std::string orbUi;
  std::string level;
  std::optional<objid> connectionPointId;
};

void generateOrbsFromPoints(std::vector<glm::vec3>& points, std::vector<objid>& pointIds, std::vector<OrbPointConfig>& orbConfigs){
  PointConfig pointConfig {};

  for (int index = 0; index < points.size(); index++){
    pointConfig.position.push_back(points.at(index));

    auto rotation = mainApi -> getGameObjectRotation(pointIds.at(index), true, "[ui] - point editor generateOrbsFromPoints");
    pointConfig.rotations.push_back(rotation);

    auto connectionPointId = orbConfigs.at(index).connectionPointId;
    int connection = -1;
    if (connectionPointId.has_value()){
      for (int i = 0; i < pointIds.size(); i++){
        if (pointIds.at(i) == connectionPointId.value()){
          connection = i;
        }
      }      
    }
    pointConfig.connections.push_back(connection);

    std::string name = std::string("point-orb-") + std::to_string(index);
    pointConfig.names.push_back(name);

    pointConfig.orbUis.push_back(orbConfigs.at(index).orbUi);
    pointConfig.levels.push_back(orbConfigs.at(index).level);

  }

  savePointConfig("../afterworld/scenes/levels/worlds/w1/w1-2/test.json", pointConfig);


}

OrbPointConfig getPointMarkerOrbConfig(objid pointId){
  OrbPointConfig config;
  auto orbUi = getObjectAttribute(pointId, "point-editor-orb-ui");
  auto level = getObjectAttribute(pointId, "point-editor-orb-level");
  auto connection = getObjectAttribute(pointId, "point-editor-orb-connection");
  if (orbUi.has_value()){
    if (auto value = std::get_if<std::string>(&orbUi.value())) config.orbUi = *value;
  }
  if (level.has_value()){
    if (auto value = std::get_if<std::string>(&level.value())) config.level = *value;
  }
  if (connection.has_value()){
    if (auto value = std::get_if<std::string>(&connection.value())){
      auto first = value->data();
      auto last = first + value->size();
      objid connectionPointId;
      auto [parsedEnd, error] = std::from_chars(first, last, connectionPointId);
      if (error == std::errc() && parsedEnd == last){
        config.connectionPointId = connectionPointId;
      }
    }
  }
  return config;
}

void savePointMarkerOrbConfig(objid pointId, const OrbPointConfig& config){
  mainApi -> setSingleGameObjectAttr(pointId, "point-editor-orb-ui", config.orbUi);
  mainApi -> setSingleGameObjectAttr(pointId, "point-editor-orb-level", config.level);
  mainApi -> setSingleGameObjectAttr(
    pointId,
    "point-editor-orb-connection",
    config.connectionPointId.has_value() ? std::to_string(config.connectionPointId.value()) : ""
  );
}

std::optional<objid> createPointMarker(glm::vec3 position){
  GameobjAttributes attributes;
  attributes.attr["mesh"] = "./res/models/ui/node.obj";
  attributes.attr["point-editor-marker"] = "true";

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  auto pointId = mainApi -> makeObjectAttr(
    0,
    std::string("mesh-point-marker-") + uniqueNameSuffix(),
    attributes,
    submodelAttributes
  );
  if (pointId.has_value()){
    mainApi -> setGameObjectPosition(pointId.value(), position, true, Hint { .hint = "[ui] - point editor create point" });
  }
  return pointId;
}

struct PointEditorCore {
  std::vector<glm::vec3> points {
    glm::vec3(-1.f, 0.f, 0.f),
    glm::vec3(0.f, 0.f, 0.f),
    glm::vec3(1.f, 0.f, 0.f),
  };
  std::vector<objid> pointIds;
  std::string error;
  bool showCoordinates = false;
  int selectedPointIndex = -1;
};


void createPoint(PointEditorCore& editor){
  editor.error.clear();
  auto position = createLocation();
  editor.points.push_back(position);
  if (!editor.pointIds.empty()){
    auto pointId = createPointMarker(position);
    if (!pointId.has_value()){
      editor.points.pop_back();
      editor.error = "Failed to create point marker";
    }else{
      editor.pointIds.push_back(pointId.value());
    }
  }
}
void sponsorPoints(PointEditorCore& editor){
  if (editor.pointIds.empty()){
    for (auto point : editor.points){
      auto pointId = createPointMarker(point);
      if (!pointId.has_value()){
        std::cout << "point editor failed to sponsor point" << std::endl;
        for (auto createdPointId : editor.pointIds){
          mainApi -> removeObjectById(createdPointId);
        }
        editor.pointIds.clear();
        break;
      }
      editor.pointIds.push_back(pointId.value());
    }
  }
}
void readBackPoints(PointEditorCore& editor){
  auto markerIds = mainApi -> getObjectsByAttr("point-editor-marker", std::nullopt, 0);
  if (editor.pointIds.empty()){
    editor.points.clear();
  }
  for (auto markerId : markerIds){
    bool isTracked = false;
    for (auto pointId : editor.pointIds){
      if (pointId == markerId){
        isTracked = true;
        break;
      }
    }
    if (!isTracked){
      editor.pointIds.push_back(markerId);
    }
  }
  std::vector<glm::vec3> updatedPoints;
  std::vector<objid> validPointIds;
  for (auto pointId : editor.pointIds){
    if (!mainApi -> gameobjExists(pointId)){
      continue;
    }
    updatedPoints.push_back(mainApi -> getGameObjectPos(pointId, true, "[ui] - point editor read point"));
    validPointIds.push_back(pointId);
  }
  editor.points = updatedPoints;
  editor.pointIds = validPointIds;
}
void removeSponsoredPoints(PointEditorCore& editor){
  for (auto pointId : editor.pointIds){
    if (mainApi -> gameobjExists(pointId)){
      mainApi -> removeObjectById(pointId);
    }
  }
  editor.pointIds.clear();
}
void clearPoints(PointEditorCore& editor){
  removeSponsoredPoints(editor);
  editor.points = {};
  editor.pointIds = {};
  editor.error = "";
  editor.selectedPointIndex = -1;
}

std::optional<std::string> renderPointEditorControls(PointEditorCore& editor, std::optional<std::string> pointFile, bool* _changed){
  std::optional<std::string> currentFile = pointFile;
  if (ImGui::BeginCombo("Point File", pointFile.has_value() ? pointFile.value().c_str() : "no value")){
    if (ImGui::Selectable("None", false)){
      currentFile = std::nullopt;
    }
    if (ImGui::Selectable("./res/data/test_points.json", false)){
      currentFile = "./res/data/test_points.json";
    }
    if (ImGui::Selectable("./res/data/test_points2.json", false)){
      currentFile = "./res/data/test_points2.json";
    }
    ImGui::EndCombo();
  }

  bool changedFile = false;
  if (currentFile.has_value() && pointFile.has_value() && currentFile.value() != pointFile.value()){
    changedFile = true;
  }else if (currentFile.has_value() != pointFile.has_value()){
    changedFile = true;
  }else if (!currentFile.has_value()){
    changedFile = true;
  }
  *_changed = changedFile;
  if (changedFile){
    clearPoints(editor);
  }

  if (ImGui::Button("Create Point")){
    createPoint(editor);
  }

  ImGui::SameLine();
  if (ImGui::Button("Sponsor Points")){
    sponsorPoints(editor);
  }

  ImGui::SameLine();
  if (ImGui::Button("Read Back Points")){
    readBackPoints(editor);
  }

  ImGui::SameLine();
  if (ImGui::Button("Remove Sponsored Points")){
    removeSponsoredPoints(editor);
  }

  if (!editor.error.empty()){
    ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "%s", editor.error.c_str());
  }

  ImGui::Text("Points: %zu", editor.points.size());
  ImGui::Text("Sponsored objects: %zu", editor.pointIds.size());
  auto selectedIds = mainApi -> selected();
  editor.selectedPointIndex = -1;
  for (int index = 0; index < editor.pointIds.size(); index++){
    for (auto selectedId : selectedIds){
      if (selectedId == editor.pointIds.at(index)){
        editor.selectedPointIndex = index;
        break;
      }
    }
    if (editor.selectedPointIndex >= 0){
      break;
    }
  }
  if (editor.selectedPointIndex >= 0){
    auto selectedId = editor.pointIds.at(editor.selectedPointIndex);
    auto position = mainApi -> getGameObjectPos(selectedId, true, "[ui] - point editor selected point");
    ImGui::Text("Selected point %d: %.2f, %.2f, %.2f", editor.selectedPointIndex, position.x, position.y, position.z);
    if (ImGui::Button("Order -") && editor.selectedPointIndex > 0){
      std::swap(editor.points.at(editor.selectedPointIndex), editor.points.at(editor.selectedPointIndex - 1));
      std::swap(editor.pointIds.at(editor.selectedPointIndex), editor.pointIds.at(editor.selectedPointIndex - 1));
    }
    ImGui::SameLine();
    if (ImGui::Button("Order +") && editor.selectedPointIndex + 1 < editor.pointIds.size()){
      std::swap(editor.points.at(editor.selectedPointIndex), editor.points.at(editor.selectedPointIndex + 1));
      std::swap(editor.pointIds.at(editor.selectedPointIndex), editor.pointIds.at(editor.selectedPointIndex + 1));
    }
  }
  ImGui::Checkbox("Show point coordinates", &editor.showCoordinates);
  if (editor.showCoordinates){
    if (ImGui::BeginChild("PointCoordinates", ImVec2(0.f, 200.f), true)){
      for (int index = 0; index < editor.points.size(); index++){
        auto& point = editor.points.at(index);
        ImGui::Text("%d: %.2f, %.2f, %.2f", index, point.x, point.y, point.z);
      }
    }
    ImGui::EndChild();
  }


  if (!currentFile.has_value()){
    ImGui::Text("No point file selected");
  }
  return currentFile;
}

PointEditorCore pointEditorCore;

void renderMeshPointEditorWidget(bool includePanel){
  if (includePanel){
    ImGui::Begin("Point Mesh Generator");
  }

  ImGui::PushID("PointMeshGenerator");

  static std::optional<std::string> pointFile = "./res/data/test_points.json";

  bool changed = false;
  pointFile = renderPointEditorControls(pointEditorCore, pointFile, &changed);

  static std::string error;
  static std::optional<std::vector<glm::vec3>> pendingPoints;
  static int framesUntilCreation = 0;
    
  if (changed){
    pendingPoints = {};
    std::cout << "changed points" << std::endl;
  }

  if (pointFile.has_value()){
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Generate Mesh Part")){
      if (pendingPoints.has_value()){
        if (framesUntilCreation > 0){
          framesUntilCreation--;
          ImGui::TextDisabled("Replacing generated mesh...");
        }else{
          auto existingId = mainApi -> getGameObjectByName(pointEditorMeshObjectName, 0);
          if (existingId.has_value()){
            if (mainApi -> gameobjExists(existingId.value())){
              mainApi -> removeObjectById(existingId.value());
            }
            framesUntilCreation = 1;
            ImGui::TextDisabled("Replacing generated mesh...");
          }else{
            std::cout << "pending points: " << pendingPoints.value().size() << std::endl;
            auto meshId = generateMeshFromPoints(pendingPoints.value());
            pendingPoints = std::nullopt;
            modassert(meshId.has_value(), "Failed to create generated mesh object");
          }
        }
      }
      if (ImGui::Button("Generate Mesh") && !pendingPoints.has_value()){
        error.clear();
        if (pointEditorCore.points.size() < 2){
          error = "Generate Mesh requires at least two points";
        }else{
          auto existingId = mainApi -> getGameObjectByName(pointEditorMeshObjectName, 0);
          if (existingId.has_value()){
            mainApi -> removeObjectById(existingId.value());
          }

          pendingPoints = pointEditorCore.points;
          framesUntilCreation = 1;
        }
      }
      if (!error.empty()){
        ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "%s", error.c_str());
      }
    }
  }

  ImGui::PopID();
  if (includePanel){
    ImGui::End();
  }
}

void renderOrbUiPointEditorWidget(bool includePanel){
  if (includePanel){
    ImGui::Begin("Point Orb UI Generator");
  }

  ImGui::PushID("PointOrbUiGenerator");

  static std::optional<std::string> pointFile;

  bool changed = false;
  pointFile = renderPointEditorControls(pointEditorCore, pointFile, &changed);

  static std::unordered_map<objid, OrbPointConfig> configs;
  static std::string error;

  if (pointFile.has_value()){
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Generate Orbs Part")){
      if (pointEditorCore.selectedPointIndex >= 0){
        auto selectedId = pointEditorCore.pointIds.at(pointEditorCore.selectedPointIndex);
        auto configIt = configs.find(selectedId);
        if (configIt == configs.end()){
          configIt = configs.emplace(selectedId, getPointMarkerOrbConfig(selectedId)).first;
        }
        auto& orbConfig = configIt->second;
        ImGui::Text("Orb configuration for point %d", pointEditorCore.selectedPointIndex);
        if (ImGui::InputText("Orb UI", &orbConfig.orbUi)){
          savePointMarkerOrbConfig(selectedId, orbConfig);
        }
        if (ImGui::InputText("Orb level", &orbConfig.level)){
          savePointMarkerOrbConfig(selectedId, orbConfig);
        }
        auto connectionPointIndex = -1;
        if (orbConfig.connectionPointId.has_value()){
          auto connection = std::find(pointEditorCore.pointIds.begin(), pointEditorCore.pointIds.end(), orbConfig.connectionPointId.value());
          if (connection != pointEditorCore.pointIds.end()){
            connectionPointIndex = std::distance(pointEditorCore.pointIds.begin(), connection);
          }
        }
        auto connectionLabel = connectionPointIndex >= 0 ? "Point " + std::to_string(connectionPointIndex) : "None";
        if (ImGui::BeginCombo("Connects to", connectionLabel.c_str())){
          if (ImGui::Selectable("None", !orbConfig.connectionPointId.has_value())){
            orbConfig.connectionPointId = std::nullopt;
            savePointMarkerOrbConfig(selectedId, orbConfig);
          }
          for (int pointIndex = 0; pointIndex < pointEditorCore.points.size(); pointIndex++){
            if (pointIndex == pointEditorCore.selectedPointIndex){
              continue;
            }
            auto pointLabel = "Point " + std::to_string(pointIndex);
            if (ImGui::Selectable(pointLabel.c_str(), connectionPointIndex == pointIndex)){
              orbConfig.connectionPointId = pointEditorCore.pointIds.at(pointIndex);
              savePointMarkerOrbConfig(selectedId, orbConfig);
            }
          }
          ImGui::EndCombo();
        }
        ImGui::Separator();
      }else{
        ImGui::TextDisabled("Select a sponsored point to configure its orb");
      }
      if (ImGui::Button("Generate Orbs")){
        error.clear();
        if (pointEditorCore.points.empty()){
          error = "Generate Orbs requires at least one point";
        }else if (pointEditorCore.points.size() != pointEditorCore.pointIds.size()){
          error = "Generate Orbs requires sponsored points";
        }else{
          std::vector<OrbPointConfig> pointConfigs;
          pointConfigs.reserve(pointEditorCore.pointIds.size());
          for (auto pointId : pointEditorCore.pointIds){
              auto configIt = configs.find(pointId);
              if (configIt == configs.end()){
                configIt = configs.emplace(pointId, getPointMarkerOrbConfig(pointId)).first;
              }
              pointConfigs.push_back(configIt->second);
          }
          generateOrbsFromPoints(pointEditorCore.points, pointEditorCore.pointIds, pointConfigs);
  
        }
      }
      if (!error.empty()){
        ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "%s", error.c_str());
      }
    }
  } 

  ImGui::PopID();
  if (includePanel){
    ImGui::End();
  }
}