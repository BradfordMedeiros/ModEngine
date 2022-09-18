#ifndef MODPLUGIN_SQLUTIL
#define MODPLUGIN_SQLUTIL

#include <variant>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <filesystem>

namespace sql{
std::vector<std::string> listAllCsvFilesStems(std::filesystem::path path);
std::string join(std::vector<std::string> values, char delimeter);
void saveFile(std::string filepath, std::string content);
std::string loadFile(std::string filepath);
std::vector<std::string> split(std::string strToSplit, char delimeter);
std::vector<std::string> filterWhitespace(std::vector<std::string> values);
void appendFile(std::string filepath, std::string content);
}
#endif