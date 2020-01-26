#ifndef MOD_UTIL
#define MOD_UTIL

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>
#include <sstream>
#include <glm/gtc/quaternion.hpp>

std::string loadFile(std::string filepath);
std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);

enum ManipulatorMode { NONE, ROTATE, TRANSLATE, SCALE };
enum ManipulatorAxis { NOAXIS, XAXIS, YAXIS, ZAXIS };

std::string print(glm::vec3 vec);
std::string print(glm::quat quat);

float maxvalue(float x, float y, float z);
int maxvalue(int x, int y, int z);

#endif