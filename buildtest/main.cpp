#include <iostream>
#include <assert.h>
#include <string>

int numDepsTested = 0;
void testpass(std::string target){
  std::cout << target << " Verified" << std::endl;
  numDepsTested++;
}
#ifdef INCLUDE_GLFW
  #include <GLFW/glfw3.h>

  void glfwTest(){
  	numUnitsTested++;
    glfwInit();
  	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   	//auto window = glfwCreateWindow(1000, 1000, "Build Test", NULL, NULL); // Uncomment to show window
    //glfwMakeContextCurrent(window);
    /*while(!glfwWindowShouldClose(window)){
       glfwPollEvents();
	   glfwSwapBuffers(window);
    }*/
    glfwTerminate();
  	testpass("GLFW");
  }
#endif

#ifdef INCLUDE_GLAD
  #include <glad/glad.h>
  void gladTest(){
   	GLint x = 2;
   	GLint y = 4;
   	assert(x + y == 6);
   	testpass("GLAD");
  }
#endif

#ifdef INCLUDE_GLM
  #include <glm/glm.hpp>
  void glmTest(){
  	glm::vec3 one(1.f, 2.f, 3.f);
  	glm::vec3 two(2.4f, 5.3, 3.5);
  	assert((one + two).x > 2); 
  	testpass("GLM");
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
  	testpass("Stb Image");
  }
#endif

#ifdef INCLUDE_OPENAL
  #include <AL/alut.h>
  #include <cstring>

  void openAlTest(){
    alutInit(NULL, NULL);
    ALuint soundBuffer = alutCreateBufferFromFile("./res/sounds/sample.wav");
  	ALenum error = alutGetError();
  	assert(error == ALUT_ERROR_NO_ERROR);
   	ALuint soundSource;
  	alGenSources(1, &soundSource);
  	alSourcei(soundSource, AL_BUFFER, soundBuffer);  
  	//while(true){					// uncomment to play sound
  	//	alSourcePlay(soundSource);
  	//}
    alutExit();
    testpass("Alut");
  }
#endif

#ifdef INCLUDE_GUILE
 #include <libguile.h>

  void guileTest(){
  	int listSize = 3;
  	SCM list = scm_make_list(scm_from_unsigned_integer(listSize), scm_from_unsigned_integer(0));
  	for (int i = 0; i < listSize; i++){
      scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_int32(i * 100)); 
  	}
  	auto numElements = scm_to_int32(scm_length(list));
  	assert(numElements == listSize);
  	for (int i = 0; i < numElements; i++){
      auto value = scm_to_int32(scm_list_ref(list, scm_from_unsigned_integer(i)));
   	  assert(value == i * 100);
   	}
  	testpass("Guile");
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
   	testpass("CXXOpts");
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
   	testpass("Assimp");
  }
#endif

#ifdef INCLUDE_BULLET
  #include <btBulletDynamicsCommon.h>
  void bulletTest(){
    auto colConfig = new btDefaultCollisionConfiguration();  
    auto dispatcher = new btCollisionDispatcher(colConfig);  
    auto broadphase = new btDbvtBroadphase();
    auto constraintSolver = new btSequentialImpulseConstraintSolver();
    auto dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, constraintSolver, colConfig);
    dynamicsWorld -> stepSimulation(100, 0);  
    testpass("Bullet");
  }
#endif

#ifdef INCLUDE_FREETYPE
  void freetypeTest(){
    assert(false);
    testpass("FreeType");
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

	#ifdef INCLUDE_GUILE
		guileTest();
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

  #ifdef INCLUDE_FREETYPE
    freetypeTest();
  #endif

	std::cout << "Build test runs (" << numDepsTested << " dependencies tested)" << std::endl;
}