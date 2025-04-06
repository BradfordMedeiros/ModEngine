#ifndef MOD_PACKAGE
#define MOD_PACKAGE

#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#include "./common/files.h"

void loopPackageShell();

void mountPackage(const char* path);
void unmountPackage();

std::string readFileOrPackage(std::string filepath);
std::vector<std::string> listFilesWithExtensionsFromPackage(std::string folder, std::vector<std::string> extensions);
bool fileExistsFromPackage(std::string filepath);

#endif