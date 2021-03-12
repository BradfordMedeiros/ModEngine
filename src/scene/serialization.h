#ifndef MOD_SERIALIZATION
#define MOD_SERIALIZATION

#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "./common/util/types.h"
#include "../common/util.h"
#include "./serialobject.h"

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
};

struct Field {
  char prefix;
  std::string type;
};

std::vector<Token> parseFormat(std::string content);
GameobjAttributes fieldsToAttributes(std::map<std::string, std::string> fields);
std::map<std::string, GameobjAttributes> deserializeSceneTokens(std::vector<Token> tokens);
std::string serializeObj(objid id, objid groupId, GameObject& gameobject, std::vector<std::string> children, bool includeIds, std::vector<std::pair<std::string, std::string>> additionalFields, std::string name = "");
bool isIdentityVec(glm::vec3 scale);

#endif