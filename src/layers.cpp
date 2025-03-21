#include "./layers.h"

std::unordered_map<std::string, RenderUniforms> allUniformsInfos(std::vector<Token>& uniformTokens){
  std::unordered_map<std::string, RenderUniforms> uniforms;
  auto renderStageValues = parseRenderStages(uniformTokens, 0, 0);  // kind of hackey, should make this common code less coupled to render stages
  
  std::cout << "num render stage values: " << renderStageValues.size() << " num tokens: " << uniformTokens.size() << std::endl;
  for (auto &renderStageValue : renderStageValues){
    modassert(renderStageValue.textures.size() == 0, "layers does not support target textures");
    modassert(renderStageValue.shader == "", "layers shader must be empty");
    modassert(uniforms.find(renderStageValue.name) == uniforms.end(), "layers -> found duplicate render stage");
    uniforms[renderStageValue.name] = RenderUniforms {
      .intUniforms = renderStageValue.intUniforms,
      .floatUniforms = renderStageValue.floatUniforms,
      .floatArrUniforms = renderStageValue.floatArrUniforms,
      .vec3Uniforms = renderStageValue.vec3Uniforms,
    };
  }
  return uniforms;
}

struct LayerTokensSplit {
  std::vector<Token> normalTokens;
  std::vector<Token> uniformTokens;
};


bool isUniformStyleToken(Token& token){
  return (token.attribute == "?enableLighting" || token.attribute == "!enableLighting");
}
// TODO actually implemement
LayerTokensSplit splitLayerTokens(std::vector<Token>& tokens){
  std::vector<Token> normalTokens;
  std::vector<Token> uniformTokens;
  for (auto &token : tokens){
    if (!isRenderStageToken(token)){  // obviously this is placeholder
      normalTokens.push_back(token);
    }else{
      uniformTokens.push_back(token);
    }
  }
  return LayerTokensSplit {
    .normalTokens = normalTokens,
    .uniformTokens = uniformTokens,
  };
}


void setLayerOption(LayerInfo& layer, std::string& attribute, std::string& payload){
  if (attribute == "zindex"){
    layer.zIndex = std::atoi(payload.c_str());
  }else if (attribute == "depth"){
    layer.depthBufferLayer = std::atoi(payload.c_str());
  }else if (attribute == "ortho"){
    if (payload == "true"){
      layer.orthographic = true;
    }else if (payload == "false"){
      layer.orthographic = false;
    }else{
      std::cout << "WARNING: layers: ortho" << payload << " is not a valid option" << std::endl;
      assert(false);
    }
  }else if (attribute == "scale"){
    if (payload == "true"){
      layer.scale = true;
    }else if (payload == "false"){
      layer.scale = false;
    }else{
      std::cout << "WARNING: layers: scale" << payload << " is not a valid option" << std::endl;
      assert(false);
    }
  }else if (attribute == "view"){
    if (payload == "true"){
      layer.disableViewTransform = false;
    }else if (payload == "false"){
      layer.disableViewTransform = true;
    }else{
      std::cout << "WARNING: layers: view" << payload << " is not a valid option" << std::endl;
      assert(false);
    }      
  }else if (attribute == "visible"){
    if (payload == "true"){
      layer.visible = true;
    }else if (payload == "false"){
      layer.visible = false;
    }else{
      std::cout << "WARNING: layers: visible" << payload << " is not a valid option" << std::endl;
      assert(false);
    }      
  }else if (attribute == "near"){
    layer.nearplane = std::atof(payload.c_str());
  }else if (attribute == "far"){
    layer.farplane = std::atof(payload.c_str());
  }else if (attribute == "fov"){
    layer.fov = std::atof(payload.c_str());
  }else if (attribute == "selection"){
    layer.selectIndex = std::atoi(payload.c_str());
  }else{
    std::cout << "WARNING: layers: " << attribute << " is not a valid option" << std::endl;
    assert(false);
  }

  std::cout << "set layer: " << attribute << " - " << payload  << " visible? " << layer.visible <<  std::endl;
}

std::set<std::string> allLayerNames(std::vector<Token>& tokens){
  std::set<std::string> names;
  for (auto &token : tokens){
    names.insert(token.target);
  }
  return names;
}

std::vector<LayerInfo> parseLayerInfo(std::string file, std::function<std::string(std::string)> readFile){
  std::cout << "parse layer info: " << file << std::endl;
  std::unordered_map<std::string, LayerInfo> layers2; 
  auto unsplitTokens = parseFormat(readFile(file));
  auto splitTokens = splitLayerTokens(unsplitTokens);
  auto allUniforms = allUniformsInfos(splitTokens.uniformTokens);
  // create map[token.target] => uniform 
  auto layerNames = allLayerNames(unsplitTokens);

  for (auto layerName : layerNames){
    if (layers2.find(layerName) == layers2.end()){
      layers2[layerName] = LayerInfo {
        .name = layerName, 
        .zIndex = 0,
        .orthographic = false,
        .scale = false,
        .disableViewTransform = false,
        .visible = true,
        .depthBufferLayer = 0,
        .fov = 45.f,
        .nearplane = 0.1f,
        .farplane = 1000.f,
        .selectIndex = 0,
        .uniforms = {
          .intUniforms = {},
          .floatUniforms = {},
          .floatArrUniforms = {},
          .vec3Uniforms = {},
        },
      };
    }
  }
  for (auto &token : splitTokens.normalTokens){
    std::cout << "(" << token.target << ", " << token.attribute << ", " << token.payload << ")" << std::endl; 
    setLayerOption(layers2.at(token.target), token.attribute, token.payload);
  }
  for (auto &[target, uniforms] : allUniforms){
    layers2.at(target).uniforms = uniforms;
  }

  std::vector<LayerInfo> layers;
  if (layers2.find("default") == layers2.end()){
    layers.push_back(LayerInfo{
      .name = "",
      .zIndex = 0,
      .orthographic = false,
      .scale = false,
      .disableViewTransform = false,
      .visible = true,
      .depthBufferLayer = 0,
      .fov = 45.f,
      .nearplane = 0.1f,
      .farplane = 1000.f,
      .selectIndex = 0,
      .uniforms = {
        .intUniforms = {},
        .floatUniforms = {},
        .floatArrUniforms = {},
        .vec3Uniforms = {},
      },
    });
  }else{
    layers2.at("default").name = "";    
  }
  for (auto &[_, layer] : layers2){
    layers.push_back(layer);
  }
  return layers;
}

LayerInfo* findLayerByName(std::vector<LayerInfo>& layers, std::string& layername){
  for (auto &layer : layers){
    if (layer.name == layername){
      std::cout << "layer name: " << layer.name << std::endl;
      return &layer;
    }
  }

  std::vector<std::string> allLayers;
  for (auto &layer : layers){
    allLayers.push_back(layer.name);
  }
  std::cout << "layer not found: " << layername << ", all = " << print(allLayers) << std::endl;
  assert(false);
  return NULL;
}
void setLayerOptions(std::vector<LayerInfo>& layers, std::vector<StrValues>& values){
  for (auto &value : values){
    LayerInfo& layer = *findLayerByName(layers, value.target);
    setLayerOption(layer, value.attribute, value.payload);
  }
}