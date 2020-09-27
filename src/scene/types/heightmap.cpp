#include "./heightmap.h"

HeightMapData loadAndAllocateHeightmap(std::string heightmapFilePath, int dim){
  assert(dim > 0);

  std::cout << "INFO: LOADING HEIGHTMAP: " << heightmapFilePath << std::endl;
  
  std::cout << "Event: loading file: " << heightmapFilePath << std::endl;
  int textureWidth, textureHeight, numChannels;

  // https://github.com/nothings/stb/blob/master/stb_image.h
  unsigned char* imageData = stbi_load(heightmapFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, 4);
  if (!imageData ){
    throw std::runtime_error("failed loading texture " + heightmapFilePath + ", reason: " + stbi_failure_reason());
  }

  int dataWidth = textureWidth > dim ? dim : textureWidth;
  int dataHeight = textureHeight > dim ? dim : textureHeight;

  float widthMultiplier = textureWidth / ((float)dim);
  float heightMultiplier = textureHeight / ((float)dim);

  std::cout << "texture width:" << textureWidth << std::endl;
  std::cout << "texture height: " << textureHeight << std::endl;
  std::cout << "w multiplier: " << widthMultiplier << std::endl;
  std::cout << "h multiplier: " << heightMultiplier << std::endl;  


  float* newData = new float[dataWidth * dataHeight];
  assert(numChannels == 3);

  // 4 components = 4 bytes (4 components, 1 byte per component -> RGBA)
  for (int i = 0; i < dataWidth; i++){
    for (int j = 0; j < dataHeight; j++){
      int byteOffset = (int)(((i * dataHeight * heightMultiplier) + j * widthMultiplier) * 4);
      assert(byteOffset < (textureWidth * textureHeight * 4));
      char r = imageData[byteOffset];
      //char g = imageData[byteOffset + 1];
      //char b = imageData[byteOffset + 2];
      //char a = imageData[byteOffset + 3];
      newData[(i * dataHeight) + j] = r;
    }
  }
  stbi_image_free(imageData);

  HeightMapData heightmapData {
    .data = newData,
    .width = dataWidth,
    .height = dataHeight,
  };
  return heightmapData;
}

// heightmap can share extra indicies for connected squares (they dont)
MeshData generateHeightmapMeshdata(HeightMapData& heightmap){
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  int32_t boneIndexes[NUM_BONES_PER_VERTEX]; // hardcoded limit of 4 per vertex
  float boneWeights[NUM_BONES_PER_VERTEX]; 

  for (int h = 0; h < heightmap.height; h++){
    for (int w = 0; w < heightmap.width; w++){
      Vertex vertex1 {
        .position = glm::vec3(w - (heightmap.width / 2.f), 0.f, h - (heightmap.height / 2.f)),
        .normal = glm::vec3(0.f, 0.f, 0.f), // todo 
        .texCoords = glm::vec2(0.f, 0.f),   // todo 
      };  
      Vertex vertex2 {
        .position = glm::vec3(1.f + w - (heightmap.width / 2.f), 0.f, h - (heightmap.height / 2.f)),
        .normal = glm::vec3(0.f, 0.f, 0.f), // todo 
        .texCoords = glm::vec2(0.f, 0.f),   // todo 
      };  
      Vertex vertex3 {
        .position = glm::vec3(1.f + w - (heightmap.width / 2.f), 0.f, 1.f + h - (heightmap.height / 2.f)),
        .normal = glm::vec3(0.f, 0.f, 0.f), // todo 
        .texCoords = glm::vec2(0.f, 0.f),   // todo 
      };  
      Vertex vertex4 {
        .position = glm::vec3(w - (heightmap.width / 2.f), 0.f, 1.f + h - (heightmap.height / 2.f)),
        .normal = glm::vec3(0.f, 0.f, 0.f), // todo 
        .texCoords = glm::vec2(0.f, 0.f),   // todo 
      };  

      vertices.push_back(vertex1);    
      vertices.push_back(vertex2);
      vertices.push_back(vertex3);
      vertices.push_back(vertex4);

      indices.push_back((h * heightmap.width * 4) + (w * 4));   
      indices.push_back((h * heightmap.width * 4) + (w * 4) + 1);
      indices.push_back((h * heightmap.width * 4) + (w * 4) + 2);
      indices.push_back((h * heightmap.width * 4) + (w * 4) + 2);
      indices.push_back((h * heightmap.width * 4) + (w * 4) + 3);
      indices.push_back((h * heightmap.width * 4) + (w * 4));
    }
  }

  BoundInfo boundInfo{};
  std::vector<Bone> bones;
  MeshData data {
    .vertices = vertices,
    .indices = indices,
    .hasDiffuseTexture = false,
    .hasEmissionTexture = false,
    .hasOpacityTexture = false,
    .boundInfo = boundInfo,
    .bones = bones,
  };
  return data;
}