#include <iostream>

#ifdef INCLUDE_GLFW
  #include <GLFW/glfw3.h>

  void glfwTest(){
  	std::cout << "placeholder glfw test" << std::endl;
  }
#endif

#ifdef INCLUDE_GLAD
#endif

#ifdef INCLUDE_GLM
  #include <glm/glm.hpp>
  void glmTest(){
  	glm::vec3 one(1.f, 2.f, 3.f);
  	glm::vec3 two(2.4f, 5.3, 3.5);
  	assert((one + two).x > 2); 
  	std::cout << "Some GLM Includes Verified" << std::endl;
  } 
#endif

#ifdef INCLUDE_STB

#endif

#ifdef INCLUDE_OPENAL
#endif

#ifdef INCLUDE_CXXOPTS

#endif

#ifdef INCLUDE_ASSIMP

#endif

#ifdef INCLUDE_BULLET

#endif

int main(){
	#ifdef INCLUDE_GLFW
		glfwTest();
	#endif

	#ifdef INCLUDE_GLM
		glmTest();
	#endif


	std::cout << "Build test runs!" << std::endl;
}