#include <iostream>
#include <assert.h>

#ifdef INCLUDE_GLFW
  #include <GLFW/glfw3.h>

  void glfwTest(){
  	std::cout << "placeholder glfw test" << std::endl;
  }
#endif

#ifdef INCLUDE_GLAD
  void gladTest(){

  }
#endif

#ifdef INCLUDE_GLM
  #include <glm/glm.hpp>
  void glmTest(){
  	glm::vec3 one(1.f, 2.f, 3.f);
  	glm::vec3 two(2.4f, 5.3, 3.5);
  	assert((one + two).x > 2); 
  	std::cout << "GLM Verified" << std::endl;
  } 
#endif

#ifdef INCLUDE_STB
  #include <stb_image.h>
  #include <stb_image_write.h>
  void stbImageTest(){

  }
#endif

#ifdef INCLUDE_OPENAL
  void openAlTest(){

  }
#endif

#ifdef INCLUDE_CXXOPTS
  #include <cxxopts.hpp>
  void cxxoptsTest(int argc, char* argv[]){
  	cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  	cxxoption.add_options()
   	  ("q,testarg", "Hide the window of the game engine", cxxopts::value<bool>()->default_value("true"))
    ;        
  	const auto result = cxxoption.parse(argc, argv);
  	auto testArg = result["testarg"].as<bool>();
  	assert(testArg);
   	std::cout << "CXXOpts Verified" << std::endl;
  }
#endif

#ifdef INCLUDE_ASSIMP
  void assimpTest(){

  }
#endif

#ifdef INCLUDE_BULLET
  void bulletTest(){
  	
  }
#endif

int main(int argc, char* argv[]){
	#ifdef INCLUDE_GLFW
		glfwTest();
	#endif

	#ifdef INCLUDE_GLAD
		gladTest();
	#endif

	#ifdef INCLUDE_GLM
		glmTest();
	#endif

	#ifdef INCLUDE_STB
		stbImageTest();
	#endif

	#ifdef INCLUDE_OPENAL
		openAlTest();
	#endif

	#ifdef INCLUDE_CXXOPTS
		cxxoptsTest(argc, argv);
	#endif

	#ifdef INCLUDE_ASSIMP
		assimpTest();
	#endif

	#ifdef INCLUDE_BULLET
		bulletTest();
	#endif


	std::cout << "Build test runs!" << std::endl;
}