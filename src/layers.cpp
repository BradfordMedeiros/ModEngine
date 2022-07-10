#include "./layers.h"

RenderUniforms uniformInfoFromTokens(){
  RenderUniforms uniforms {
    .intUniforms = {
      RenderDataInt {
        .uniformName = "enableLighting",
        .value = true,
      }
    },
    .floatUniforms = {},
    .floatArrUniforms = {},
    .vec3Uniforms = {},
    .builtInUniforms = {},
  };
  return uniforms;
}

std::map<std::string, RenderUniforms> allUniformsInfos(std::vector<Token>& uniformTokens){
  std::map<std::string, RenderUniforms> uniforms;
  uniforms["default"] = uniformInfoFromTokens();
  return uniforms;
}

struct LayerTokensSplit {
  std::vector<Token> normalTokens;
  std::vector<Token> uniformTokens;
};

// TODO actually implemement
LayerTokensSplit splitLayerTokens(std::vector<Token>& tokens){
  std::vector<Token> normalTokens;
  for (auto &token : tokens){
    if (!(token.attribute == "?enableLighting")){  // obviously this is placeholder
      normalTokens.push_back(token);
    }
  }
  return LayerTokensSplit {
    .normalTokens = normalTokens,
    .uniformTokens = {},
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


std::vector<LayerInfo> parseLayerInfo(std::string file, std::function<std::string(std::string)> readFile){
  std::cout << "parse layer info: " << file << std::endl;
  std::map<std::string, LayerInfo> layers2; 
  auto unsplitTokens = parseFormat(readFile(file));
  auto splitTokens = splitLayerTokens(unsplitTokens);
  auto allUniforms = allUniformsInfos(splitTokens.uniformTokens);
  // create map[token.target] => uniform 

  for (auto token : splitTokens.normalTokens){
    std::cout << "(" << token.target << ", " << token.attribute << ", " << token.payload << ")" << std::endl; 
    if (layers2.find(token.target) == layers2.end()){
      layers2[token.target] = LayerInfo {
        .name = token.target, 
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
          .builtInUniforms = {},
        },
      };
    }
    setLayerOption(layers2.at(token.target), token.attribute, token.payload);
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
        .builtInUniforms = {},
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
  std::cout << "layer not found: " << layername << std::endl;
  assert(false);
  return NULL;
}
void setLayerOptions(std::vector<LayerInfo>& layers, std::vector<StrValues>& values){
  for (auto &value : values){
    LayerInfo& layer = *findLayerByName(layers, value.target);
    setLayerOption(layer, value.attribute, value.payload);
  }
}