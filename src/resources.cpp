#include "./resources.h"

namespace resources {

#define DEFINE_RESOURCE(name, value) const char* name = value;
#include "./resource_defs.h"
#undef DEFINE_RESOURCE

std::vector<const char*> coreResources = {
	#define DEFINE_RESOURCE(name, value) name,
		#include "./resource_defs.h"
	#undef DEFINE_RESOURCE
};

}