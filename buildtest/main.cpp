#include <iostream>
#include <assert.h>

#ifdef INCLUDE_GLFW
  #include <GLFW/glfw3.h>

  void glfwTest(){
    glfwInit();
  	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   	//auto window = glfwCreateWindow(1000, 1000, "Build Test", NULL, NULL);
    //glfwMakeContextCurrent(window);
    /*while(!glfwWindowShouldClose(window)){
       glfwPollEvents();
	   glfwSwapBuffers(window);
    }*/
    glfwTerminate();
  	std::cout << "GLFW Verified" << std::endl;
  }
#endif

#ifdef INCLUDE_GLAD
  #include <glad/glad.h>
  void gladTest(){
   	GLint x = 2;
   	GLint y = 4;
   	assert(x + y == 6);
   	std::cout << "GLAD Verified" << std::endl;
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
    int textureWidth = 0, textureHeight = 0, numChannels = 0;
    auto texturePath = "./res/textures/wood.jpg";
  	unsigned char* data = stbi_load(texturePath, &textureWidth, &textureHeight, &numChannels, 0); 
  	if (!data){
      throw std::runtime_error(std::string("failed loading texture ") + texturePath + ", reason: " + stbi_failure_reason());
  	}
  	assert(textureWidth != 0);
  	assert(textureHeight != 0);
  	assert(numChannels != 0);
  	//std::cout << "texture: width = " << textureWidth << " height = " << textureHeight << " numChannels = " << numChannels << std::endl;
	stbi_image_free(data);
  	std::cout << "Stb Image Verified" << std::endl;
  }
#endif

#ifdef INCLUDE_OPENAL
  void openAlTest(){
   	std::cout << "OpenAl Test Not Implemented" << std::endl;
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
  #include <assimp/Importer.hpp>
  #include <assimp/scene.h>
  #include <assimp/postprocess.h>

  void assimpTest(){
   	Assimp::Importer import;
   	const aiScene* scene = import.ReadFile("./res/models/unit_rect/unit_rect.obj", aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
   	assert(scene != NULL);
   	assert(scene -> mNumMeshes > 0);
   	assert(scene -> mRootNode != NULL);
   	std::cout << "Assimp Verified" << std::endl;
  }
#endif

#ifdef INCLUDE_BULLET
  void bulletTest(){
   	std::cout << "Bullet Test Not Implemented" << std::endl;
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