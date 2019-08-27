#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>

// idea is to have a union of the types we want + active type in struct, and then a data structure which will map ids (objects that we add to the scene)
// to the proper types 
// this way we can decouple, for example, the camera type, or the concept of a mesh, from our scene graph 

void testObjectTypes(){
	std::cout << "test object types here!!" << std::endl;
}

#endif 