#include "./modlayer_test.h"

struct ModLayerPathTest {
	std::string file;
	std::string modpath;
	std::string expectedpath;
};

void modlayerPathTest(){
	std::vector<ModLayerPathTest> tests = {
		ModLayerPathTest {
			.file = "./res/shaders/blur/vertex.glsl",
			.modpath = "./res/modlayers/",
			.expectedpath = "/home/brad/gamedev/mosttrusted/ModEngine/res/modlayers/shaders/blur/vertex.glsl", // path should be absolute, but the test should check something else
		},
		ModLayerPathTest {
			.file = "./res/textures/default.jpg",
			.modpath = "./res/modlayers/",
			.expectedpath = "/home/brad/gamedev/mosttrusted/ModEngine/res/modlayers/textures/default.jpg",
		},
		ModLayerPathTest {
			.file = "/home/brad/gamedev/mosttrusted/ModEngine/res/textures/wood.jpg ",
			.modpath = "./res/modlayers/",
			.expectedpath = "/home/brad/gamedev/mosttrusted/ModEngine/res/modlayers/textures/wood.jpg",
		},
		ModLayerPathTest {
			.file = "./res/textures/skyboxs/desert",
			.modpath = "./res/modlayers/",
			.expectedpath = "/home/brad/gamedev/mosttrusted/ModEngine/res/modlayers/textures/skyboxs/desert",
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

