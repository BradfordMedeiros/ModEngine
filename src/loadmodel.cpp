#include "./loadmodel.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>

void loadModel(std::string modelPath){
   Assimp::Importer import;
   const aiScene* scene = import.ReadFile(modelPath, aiProcess_Triangulate);
   if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
      std::cerr << "error loading model" << std::endl;
      throw std::runtime_error("error loading stuff");
   } 
  

   std::cout << "found file" << std::endl;
}
