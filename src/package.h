#ifndef MOD_PACKAGE
#define MOD_PACKAGE

#include <vector>
#include <string>
#include <cstring>
#include <iostream>


#include "./common/util.h"

struct PackageHeader {
	uint32_t packageIdentifier = 109111;
	uint32_t version = 0;
	uint32_t numberOfFiles;
};

struct FileMetadata {
	uint32_t offsetBytes;
	uint32_t sizeBytes;
	uint32_t hashname;
	char name[256];
};

struct Package {
	PackageHeader header;
	std::vector<FileMetadata> fileMetadata;
};

void packageDirectory(const char* path, const char* output);
void unpackageDirectory(const char* path, const char* output);
Package loadPackage(const char* path);

Package loadPackage(const char* path);
void closePackage(Package& package);
void readFile(Package& package, const char* _data, int* _sizeBytes);

std::vector<std::string> ls(Package& package, const char* path);
std::string cat(Package& package, const char* path);

void loopPackageShell(Package& package);

#endif