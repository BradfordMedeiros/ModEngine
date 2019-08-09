#include <fstream>
#include "./readfont.h"
#include "./util.h"

font readFont(std::string fontFolderpath){
	std::string metadata = fontFolderpath + "/info.sfl";
	std::ifstream file(metadata);

	font info;

	if (!file.is_open()) {
		throw std::runtime_error("Could not open file" + fontFolderpath);
	}
    
    std::string line;
    getline(file, line);		// first line is font name
    info.fontname = line;

    getline(file, line);		// skip second line, dont care
    getline(file, line);		// third line is image name, if not image.png throw exception for now

    if(line != "image.png"){
    	throw std::runtime_error("image must be named image.png for font");
    }

    info.image = fontFolderpath + "/image.png";

    std::getline(file, line);
    unsigned int numchars = std::stoul(line);

	for (unsigned int i = 0; i < numchars, getline(file,line); i++) {
        std::stringstream singleFontLine(line);
		std::string content;

		fontInfo character;

        std::getline(singleFontLine, content, ' ');
        character.id = std::stoul(content),

        std::getline(singleFontLine, content, ' ');
        character.x = std::stoul(content);

        std::getline(singleFontLine, content, ' ');
        character.y = std::stoul(content);

        info.chars[character.id] = character;
    }

    file.close();	
    return info;
}

// font, imagewidth, imageheight -> get ndi for texture coordinates
// then draw similar to 2d mesh, but with texture coords mapped differently 











