#ifndef MOD_OPTIONS
#define MOD_OPTIONS
#include <iostream>

struct options {
    std::string shaderFolderPath;
    std::string texturePath;
};

options loadOptions(char* argv[]);

#endif
