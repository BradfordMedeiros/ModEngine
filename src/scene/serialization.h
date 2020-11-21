#ifndef MOD_SERIALIZATION
#define MOD_SERIALIZATION

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "./common/util/types.h"
#include "../common/util.h"

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
  std::string layer;
};

struct Field {
  char prefix;
  std::string type;
};

struct LayerInfo {
  std::string name;
  int zIndex;
  bool orthographic;
};
struct ParsedContent {
  std::vector<Token> tokens; 
  std::vector<LayerInfo> layers;
};
ParsedContent parseFormat(std::string content);
GameobjAttributes fieldsToAttributes(std::map<std::string, std::string> fields);
std::map<std::string, GameobjAttributes> deserializeSceneTokens(std::vector<Token> tokens);

#endif