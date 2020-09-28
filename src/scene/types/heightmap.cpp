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

int indexFromCoords(HeightMapData& heightmap, int vertexX, int vertexY){
  return (vertexY * heightmap.width + vertexX);
}

// heightmap can share extra indicies for connected squares (they dont)
MeshData generateHeightmapMeshdata(HeightMapData& heightmap){
  std::vector<Vertex> vertices;
  std::vector<int> frequency;
  std::vector<unsigned int> indices;

  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  int32_t boneIndexes[NUM_BONES_PER_VERTEX]; // hardcoded limit of 4 per vertex
  float boneWeights[NUM_BONES_PER_VERTEX]; 

  for (int h = 0; h < heightmap.height; h++){
    for (int w = 0; w < heightmap.width; w++){
      auto height = heightmap.data[(h * heightmap.width) + w];
      Vertex vertex {
        .position = glm::vec3(w - ((heightmap.width  - 1)/ 2.f), height, h - ((heightmap.height - 1) / 2.f)),
        .normal = glm::vec3(0.f, 1.f, 0.f), // todo 
        .texCoords = glm::vec2(w * 1.f / heightmap.width, h * 1.f / heightmap.height),   // todo 
      };  
      vertices.push_back(vertex);    
      frequency.push_back(0);
    }
  }

  // Remember CCW winding order (for face culling)  -- see ./misc/hmtriangles.png
  for (int h = 1; h < heightmap.height; h++){
    for (int w = 1; w < heightmap.width; w++){
      // first triangle
      indices.push_back(indexFromCoords(heightmap, w - 1, h));
      indices.push_back(indexFromCoords(heightmap, w, h));      
      indices.push_back(indexFromCoords(heightmap, w, h - 1));      

      // second triangle
      indices.push_back(indexFromCoords(heightmap, w, h - 1));      
      indices.push_back(indexFromCoords(heightmap, w - 1, h - 1));      
      indices.push_back(indexFromCoords(heightmap, w - 1, h));      
    }
  }

  // every 3 indices make a triangle, take those vertices + cross product, and you have a normal 
  // add that normal to a list for that indices in a list
  // then loop over the vertices and divide by how many times that vertex appeared 
  // (could probably derive frequency w/o memory since all interior have same amount and exterior)
  for (int i = 0; i < indices.size(); i+=3){ 
    auto vert0 = indices.at(i);
    auto vert1 = indices.at(i + 1);
    auto vert2 = indices.at(i + 2);
    auto point0 = vertices.at(vert0).position;
    auto point1 = vertices.at(vert1).position;
    auto point2 = vertices.at(vert2).position;
    auto sideOne = point1 - point0; 
    auto sideTwo = point2 - point0;
    auto normal = glm::cross(sideOne, sideTwo);     // right hand rule determines if this is up or down 
    frequency.at(vert0) = frequency.at(vert0) + 1;
    vertices.at(vert0).normal += normal;
    frequency.at(vert1) = frequency.at(vert1) + 1;
    vertices.at(vert1).normal += normal;
    frequency.at(vert2) = frequency.at(vert2) + 1;
    vertices.at(vert2).normal += normal;
  }

  for (int i = 0; i < vertices.size(); i++){
    vertices.at(i).normal /= ((float)frequency.at(i));
  }

  BoundInfo boundInfo{};
  std::vector<Bone> bones;
  MeshData data {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "./res/textures/grass.jpeg",
    .hasDiffuseTexture = true,
    .hasEmissionTexture = false,
    .hasOpacityTexture = false,
    .boundInfo = boundInfo,
    .bones = bones,
  };
  return data;
}