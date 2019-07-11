#include "./options.h"
#include <iostream>

options loadOptions(std::string args){
   options opts = {
     .shaderFolderPath = args ,
   };
   return opts;
}
