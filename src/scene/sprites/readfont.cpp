#include "./readfont.h"

std::string readFileOrPackage(std::string filepath); // i dont really like directly referencing this here, but...it's ok


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
    auto fileContent = readFileOrPackage(metadata);
	std::istringstream file(fileContent);

	font info;

    std::string line;
    getline(file, line);		// first line is font name
    //info.fontname = line;

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

    return info;
}

ttfFont readTTFFont(std::string filepath){
    return ttfFont{};
}

fontType readFontFile(std::string filepath){
    auto fontExtension = getExtension(filepath);
    std::cout << "FONT: font extension is: " << (fontExtension.has_value() ? fontExtension.value() : "no extension") << std::endl;
    if (fontExtension == "ttf"){
        return readTTFFont(filepath);
    }
    return readFont(filepath);
}

std::vector<FontToLoad> readFontFile(std::vector<std::string> filepaths){
    std::vector<FontToLoad> fonts;
    for (auto filepath : filepaths){
        fonts.push_back(FontToLoad{
           .name = filepath,
           .type = readFontFile(filepath),
        });
    }
    return fonts;
}