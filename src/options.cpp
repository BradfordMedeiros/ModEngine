#include <stdexcept>
#include "./options.h"

options loadOptions(int argc, char* argv[]){
   if (argc < 4){
      throw std::runtime_error("please provide: shaderFolderPath, texturePath, framebuffershader");
   }
   options opts = {
     .shaderFolderPath = argv[1],
     .texturePath = argv[2],
     .framebufferShaderPath = argv[3],
   };

   return opts;
}
