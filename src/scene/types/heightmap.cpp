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

MeshData generateHeightmapMeshdata(HeightMapData& heightmap){
  /*std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
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
  return data;*/
  std::cout << "HEIGHTMAP PLACEHOLDER LOAD MESH! -- just loading box mesh for now!" << std::endl;
  ModelData data = loadModel("./res/models/box/box.obj");
  return data.meshIdToMeshData.begin() -> second;
}