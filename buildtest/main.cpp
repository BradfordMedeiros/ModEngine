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

//https://freetype.org/freetype2/docs/tutorial/step1.html
// https://freetype.org/freetype2/docs/reference/ft2-base_interface.html#ft_render_glyph char encoding
#ifdef INCLUDE_FREETYPE
  #include <ft2build.h>
  #include FT_FREETYPE_H  

  void freetypeTest(){
    FT_Library ft;
    if (FT_Init_FreeType(&ft)){
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        assert(false);
    }
    FT_Face face;
    if (FT_New_Face(ft, "res/fonts/Walby-Regular.ttf", 0, &face)){
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
        assert(false);
    }

    //FT_Set_Char_Size(face, 0, 16*64, 300, 300);   
    FT_Set_Pixel_Sizes(face, 64, 64);
    
    std::cout << "num glyphs: " << face -> num_glyphs << std::endl;
    std::cout << "height: " << face -> height << std::endl;

    if (FT_Load_Char(face, 'B', FT_LOAD_RENDER)){
      std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;  
      assert(false);
    }

    std::cout << "glyph bitmap width: " << face -> glyph -> bitmap.width << std::endl;
    std::cout << "glyph bitmap rows: " << face -> glyph -> bitmap.rows << std::endl;
    std::cout << "glyph advance: " << face -> glyph -> advance.x << std::endl;
    std::cout << "glypm bitmap: " << (face->glyph->bitmap.buffer ) << std::endl;

    testpass("FreeType");
  }
#endif

#ifdef INCLUDE_VIDEO
  #include <webm/webm_parser.h>
  #include <webm/file_reader.h>

  #include <vpx/vpx_decoder.h>
  #include <vpx/vp8dx.h>

  void videoTest(){
    // Open the WebM file

    FILE* file = std::fopen("../gameresources/video/bigbuck.webm", "rb");
    if (!file) {
      std::cerr << "File cannot be opened\n";
      return;
    }
    std::cout << "opened the file" << std::endl;

    webm::FileReader reader(file);
    webm::WebmParser parser;

    vpx_codec_ctx_t codec;
    vpx_codec_dec_cfg_t cfg = {0};
    vpx_codec_iface_t *iface = vpx_codec_vp9_dx();
    if (vpx_codec_dec_init(&codec, iface, &cfg, 0)) {
      fprintf(stderr, "Failed to initialize decoder\n");
    }

    testpass("webm");
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

  #ifdef INCLUDE_FREETYPE
    freetypeTest();
  #endif

  #ifdef INCLUDE_VIDEO
    videoTest();
  #endif 

	std::cout << "Build test runs (" << numDepsTested << " dependencies tested)" << std::endl;
}

