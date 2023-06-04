#ifndef MOD_MAINUTIL
#define MOD_MAINUTIL

#include "./main_api.h"

struct ParsedLoadScene {
  std::string sceneToLoad;
  std::optional<std::string> sceneFileName;
  std::vector<std::string> tags;
};

objid createManipulator();
ManipulatorSelection onManipulatorSelected();
glm::mat4 projectionFromLayer(LayerInfo& layer);
LayerInfo layerByName(std::string layername);
LayerInfo getLayerForId(objid id);
std::vector<ParsedLoadScene> parseSceneArgs(std::vector<std::string>& rawScenes);
std::optional<unsigned int> getTextureId(std::string& texture);


#endif