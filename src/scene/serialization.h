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
#include "./objtypes/obj_util.h"

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
std::string getTokenPayload(std::vector<Token>& tokens, std::string attribute);
std::string serializeSceneTokens(std::vector<Token>& tokens);

struct DividedTokens {
  std::vector<Token> mainTokens;
  std::vector<Token> subelementTokens;
};

std::string mainTargetElement(std::string target);
DividedTokens divideMainAndSubelementTokens(std::vector<Token> tokens);
std::map<std::string, GameobjAttributes> deserializeSceneTokens(std::vector<Token> tokens);
std::string serializeObj(objid id, objid groupId, GameObject& gameobject, std::vector<std::string> children, bool includeIds, std::vector<std::pair<std::string, std::string>> additionalFields, std::string name = "");

void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr);

void setAttribute(GameObject& gameobj, std::string field, AttributeValue attr);
void setAllAttributes(GameObject& gameobj, GameobjAttributes& attr, ObjectSetAttribUtil& util);

#endif