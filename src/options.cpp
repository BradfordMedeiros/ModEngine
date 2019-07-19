#include "./options.h"

options loadOptions(char* argv[]){

   options opts = {
     .shaderFolderPath = argv[1],
     .texturePath = argv[2],
   };

   return opts;
}
