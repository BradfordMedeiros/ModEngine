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
void packageDirectory(const char* output, std::vector<std::string> dirs);

unsigned int openFileOrPackage(std::string filepath);
int closeFileOrPackage(unsigned int handle);
size_t readFileOrPackage(unsigned int handle, void *ptr, size_t size, size_t nmemb);
int seekFileOrPackage(unsigned int handle, int offset, int whence);
size_t tellFileOrPackage(unsigned int handle);

#endif