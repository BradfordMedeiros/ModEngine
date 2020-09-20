#ifndef MOD_UTIL
#define MOD_UTIL

#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <stdlib.h>    

std::string loadFile(std::string filepath);
void saveFile(std::string filepath, std::string content);
std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions);
std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);
std::string join(std::vector<std::string> values, char delimeter);
std::vector<std::string> filterWhitespace(std::vector<std::string> values);
std::vector<std::string> filterComments(std::vector<std::string> values);

enum ManipulatorMode { NONE, ROTATE, TRANSLATE, SCALE };
enum Axis { NOAXIS, XAXIS, YAXIS, ZAXIS };

std::string print(glm::vec3 vec);
std::string print(glm::vec2 vec);
std::string print(glm::quat quat);
glm::vec3 parseVec(std::string positionRaw);
glm::vec2 parseVec2(std::string positionRaw);
glm::quat parseQuat(std::string payload);
glm::vec3 quatToVec(glm::quat quat);
std::string serializeVec(glm::vec3 vec);
std::string serializeVec(glm::vec2 vec);
std::string serializeRotation(glm::quat rotation);


float maxvalue(float x, float y, float z);
int maxvalue(int x, int y, int z);

typedef short objid;
objid getUniqueObjId();
std::string getUniqueObjectName();

#endif