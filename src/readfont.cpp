#include <fstream>
#include "./readfont.h"
#include "./util.h"

fontInfo getFont(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int imageWidth, unsigned int imageHeight){
    fontInfo character;

    character.x = x/(float)imageWidth;
    character.y = y/(float)imageHeight;
    character.width = width/(float)imageWidth;
    character.height = height/(float)imageHeight;
    
    return character;
}

font readFont(std::string fontFolderpath){
	std::string metadata = fontFolderpath + "/info.modfont";
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

    getline(file, line);        
    unsigned int imageWidth = std::stoul(line);
    getline(file, line);       
    unsigned int imageHeight = std::stoul(line);

	for (unsigned int i = 0; i < numchars, getline(file,line); i++) {
        std::stringstream singleFontLine(line);
		std::string content;

        std::getline(singleFontLine, content, ' ');
        unsigned int ascii = std::stoul(content);

        std::getline(singleFontLine, content, ' ');
        unsigned int x = std::stoul(content);

        std::getline(singleFontLine, content, ' ');
        unsigned int y = std::stoul(content);

        std::getline(singleFontLine, content, ' ');
        unsigned int width = std::stoul(content);

        std::getline(singleFontLine, content, ' ');
        unsigned int height = std::stoul(content);

        info.chars[ascii] = getFont(x, y, width, height, imageWidth, imageHeight);
    }

    file.close();	
    return info;
}

// font, imagewidth, imageheight -> get ndi for texture coordinates
// then draw similar to 2d mesh, but with texture coords mapped differently 











