#ifndef MOD_FONT
#define MOD_FONT

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>

struct fontInfo {
	long unsigned int x;	// maybe this should be ndi
	long unsigned int y;
};
struct font {
	std::string fontname;
	std::string image;
	std::map<unsigned int, fontInfo> chars;
	unsigned int imageWidth;
	unsigned int imageHeight;
};

struct convertedCoords {
	float offsetxndi;
	float offsetyndi;
	float widthndi;
	float heightndi;
};

font readFont(std::string filepath);

#endif