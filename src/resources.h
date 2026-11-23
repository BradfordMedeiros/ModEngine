#ifndef MOD_RESOURCES
#define MOD_RESOURCES 

#include <vector>

namespace resources {

  #define DEFINE_RESOURCE(name, value) extern const char* name;
  #define DEFINE_FILE_RESOURCE(name, value) extern const char* name;
  #define DEFINE_DIRECTORY_RESOURCE(name, value) extern const char* name;
    #include "./resource_defs.h"
  #undef DEFINE_RESOURCE
  #undef DEFINE_FILE_RESOURCE
  #undef DEFINE_DIRECTORY_RESOURCE
  
  extern std::vector<const char*> coreResources;
  extern std::vector<const char*> coreDirectories;

}

#endif