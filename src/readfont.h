#ifndef MOD_FONT
#define MOD_FONT

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>

struct fontInfo {
	long unsigned int id;
	long unsigned int x;
	long unsigned int y;

};
struct font {
	std::string fontname;
	std::string image;
	std::map<unsigned int, fontInfo> chars;
};

font readFont(std::string filepath);

#endif