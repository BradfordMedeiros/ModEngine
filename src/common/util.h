#ifndef MOD_UTIL
#define MOD_UTIL

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

std::string loadFile(std::string filepath);
std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);

#endif