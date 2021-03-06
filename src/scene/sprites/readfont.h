#ifndef MOD_FONT
#define MOD_FONT

#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include "../../common/util.h"

// This is all ndi so [0, 1] relative to texture
struct fontInfo {
	float x;	
	float y;
	float width;
	float height;
};

struct font {
	std::string fontname;
	std::string image;
	std::map<unsigned int, fontInfo> chars;
};

font readFont(std::string filepath);

#endif