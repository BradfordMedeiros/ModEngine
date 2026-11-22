#include "./resources.h"

namespace resources {

#define DEFINE_RESOURCE(name, value) const char* name = value;
#define DEFINE_FILE_RESOURCE(name, value) const char* name = value;
#include "./resource_defs.h"
#undef DEFINE_RESOURCE
#undef DEFINE_FILE_RESOURCE

std::vector<const char*> coreResources = {
	#define DEFINE_RESOURCE(name, value)
	#define DEFINE_FILE_RESOURCE(name, value) name,
		#include "./resource_defs.h"
	#undef DEFINE_RESOURCE
	#undef DEFINE_FILE_RESOURCE
};

}