#include "./layers.h"

std::vector<LayerInfo> parseLayerInfo(std::string file){
  std::cout << "parse layer info: " << file << std::endl;
  std::map<std::string, LayerInfo> layers2; 
  auto tokens = parseFormat(loadFile(file));
  for (auto token : tokens){
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
      };
    }

    if (token.attribute == "zindex"){
      layers2[token.target].zIndex = std::atoi(token.payload.c_str());
    }else if (token.attribute == "depth"){
      layers2[token.target].depthBufferLayer = std::atoi(token.payload.c_str());
    }else if (token.attribute == "ortho"){
      if (token.payload == "true"){
        layers2[token.target].orthographic = true;
      }else if (token.payload == "false"){
        layers2[token.target].orthographic = false;
      }else{
        std::cout << "WARNING: layers: ortho" << token.payload << " is not a valid option" << std::endl;
        assert(false);
      }
    }else if (token.attribute == "scale"){
      if (token.payload == "true"){
        layers2[token.target].scale = true;
      }else if (token.payload == "false"){
        layers2[token.target].scale = false;
      }else{
        std::cout << "WARNING: layers: scale" << token.payload << " is not a valid option" << std::endl;
        assert(false);
      }
    }else if (token.attribute == "view"){
      if (token.payload == "true"){
        layers2[token.target].disableViewTransform = false;
      }else if (token.payload == "false"){
        layers2[token.target].disableViewTransform = true;
      }else{
        std::cout << "WARNING: layers: view" << token.payload << " is not a valid option" << std::endl;
        assert(false);
      }      
    }else if (token.attribute == "visible"){
      if (token.payload == "true"){
        layers2[token.target].visible = true;
      }else if (token.payload == "false"){
        layers2[token.target].visible = false;
      }else{
        std::cout << "WARNING: layers: visible" << token.payload << " is not a valid option" << std::endl;
        assert(false);
      }      
    }else if (token.attribute == "near"){
      layers2[token.target].nearplane = std::atof(token.payload.c_str());
    }else if (token.attribute == "far"){
      layers2[token.target].farplane = std::atof(token.payload.c_str());
    }else if (token.attribute == "fov"){
      layers2[token.target].fov = std::atof(token.payload.c_str());
    }else if (token.attribute == "selection"){
      layers2[token.target].selectIndex = std::atoi(token.payload.c_str());
    }else{
      std::cout << "WARNING: layers: " << token.attribute << " is not a valid option" << std::endl;
    }
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
    });
  }else{
    layers2.at("default").name = "";    
  }
  for (auto &[_, layer] : layers2){
    layers.push_back(layer);
  }
  return layers;
}