#include "./modlayer_test.h"

struct ModLayerPathTest {
	std::string file;
	std::string modpath;
	std::string expectedpath;
};

std::string absPath(std::string path){
	auto filepath = std::filesystem::absolute(std::filesystem::path(path)).lexically_normal();
	return filepath.string();
}

void modlayerPathTest(){
	std::vector<ModLayerPathTest> tests = {
		ModLayerPathTest {
			.file = "./res/shaders/blur/vertex.glsl",
			.modpath = "./res/modlayers/",
			.expectedpath = absPath("./res/modlayers/shaders/blur/vertex.glsl"), // path should be absolute
		},
		ModLayerPathTest {
			.file = "./res/textures/default.jpg",
			.modpath = "./res/modlayers/",
			.expectedpath = absPath("./res/modlayers/textures/default.jpg"),
		},
		ModLayerPathTest {
			.file = "./res/textures/skyboxs/desert",
			.modpath = "./res/modlayers/",
			.expectedpath = absPath("./res/modlayers/textures/skyboxs/desert"),
		},
	};

	for (int i = 0; i < tests.size(); i++){
		auto test = tests.at(i);
		auto newPath = joinModPath(test.modpath, test.file);
		if (newPath != test.expectedpath){
      throw std::logic_error("wrong path: got: (" + newPath + ") but wanted: (" + test.expectedpath + ") - " + std::to_string(i));
		}
	}
}

