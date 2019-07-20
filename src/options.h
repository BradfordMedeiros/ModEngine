#ifndef MOD_OPTIONS
#define MOD_OPTIONS
#include <iostream>

struct options {
    std::string shaderFolderPath;
    std::string texturePath;
    std::string framebufferShaderPath;
};

options loadOptions(int argc, char* argv[]);

#endif
