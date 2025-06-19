#ifndef MOD_RESOURCES
#define MOD_RESOURCES 

#include <vector>

namespace resources {

  #define DEFINE_RESOURCE(name, value) extern const char* name;
    #include "./resource_defs.h"
  #undef DEFINE_RESOURCE
  
  extern std::vector<const char*> coreResources;

}

#endif