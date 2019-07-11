#ifndef MOD_OPTIONS
#define MOD_OPTIONS
#include <iostream>

struct options {
    std::string shaderFolderPath;
};

options loadOptions(std::string argv);

#endif
