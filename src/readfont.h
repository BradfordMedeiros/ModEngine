#ifndef MOD_FONT
#define MOD_FONT

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>

struct fontInfo {
	float x;	// maybe this should be ndi
	float y;
};

struct font {
	std::string fontname;
	std::string image;
	std::map<unsigned int, fontInfo> chars;
};

font readFont(std::string filepath);

#endif