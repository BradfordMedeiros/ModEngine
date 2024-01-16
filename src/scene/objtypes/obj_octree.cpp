#include "./obj_octree.h"

std::vector<AutoSerialize> octreeAutoserializer {
 
};

struct OctreeDivision {
  // -x +y -z 
  // +x +y -z
  // -x +y +z
  // +x +y +z
  // -x -y -z 
  // +x -y -z
  // -x -y +z
  // +x -y +z
  bool filled;
  std::vector<OctreeDivision> divisions; 
};

struct Octree {
  double size;
  OctreeDivision rootNode;
};


Octree unsubdividedOctree {
  .size = 5.f,
  .rootNode = OctreeDivision {
    .filled = false,
    .divisions = {},
  },
};

Octree subdividedOne {
  .size = 5.f,
  .rootNode = OctreeDivision {
    .filled = true,
    .divisions = {
      OctreeDivision { .filled = true },
      OctreeDivision { .filled = false },
      OctreeDivision { 
        .filled = true,
        .divisions = {
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
        },

      },
      OctreeDivision { .filled = false },

      OctreeDivision { .filled = true },
      OctreeDivision { .filled = false },
      OctreeDivision { .filled = true },
      OctreeDivision { .filled = true },
    },
  },
};

Octree testOctree = subdividedOne;



/*Mesh createNavmeshFromPointList(std::vector<glm::vec3> points, ){

}*/

Vertex createVertex2(glm::vec3 position, glm::vec2 texCoords){
  Vertex vertex {
    .position = position,
    .normal = glm::vec3(0.f, 0.f, 0.f),  // TODO - calculate normal properly
    .texCoords = texCoords,
  };
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    vertex.boneIndexes[i] = 0;
    vertex.boneWeights[i] = 0;
  }
  return vertex;
}

void addCubePoints(std::vector<glm::vec3>& points, float size, glm::vec3 offset){
  //bottom plane
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  points.push_back(glm::vec3(size, 0.f, -size) + offset);
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);

  // top plane
  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(size, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);

  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(size, size, 0.f) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);

  // left plane
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  // right plane
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(size, 0.f, -size) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);

  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);
  points.push_back(glm::vec3(size, size, 0.f) + offset);


  // front plane
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  points.push_back(glm::vec3(size, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);


  // back plane
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(size, 0.f, -size) + offset);

  points.push_back(glm::vec3(size, 0.f, -size) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);
}

Mesh createOctreeMesh(ObjectTypeUtil& util){
  std::vector<Vertex> vertices;
  std::vector<glm::vec3> points = {};

  float rootWidth = testOctree.size;
  if (testOctree.rootNode.divisions.size() == 0){
    if (testOctree.rootNode.filled){
      addCubePoints(points, rootWidth, glm::vec3(0.f, 0.f, 0.f));
    }
  }else{
    modassert(testOctree.rootNode.divisions.size() == 8, "subdivisions should be 0 or 8");
    float size = rootWidth * 0.5f;
  
    // -x +y -z 
    // +x +y -z
    // -x +y +z
    // +x +y +z
    // -x -y -z 
    // +x -y -z
    // -x -y +z
    // +x -y +z

    if (testOctree.rootNode.divisions.at(0).filled){
      addCubePoints(points, size, glm::vec3(-0.5f * size, 0.5f * size, -0.5f * size));
    }
    if (testOctree.rootNode.divisions.at(1).filled){
      addCubePoints(points, size, glm::vec3(0.5f * size, 0.5f * size, -0.5f * size));
    }
    if (testOctree.rootNode.divisions.at(2).filled){
      addCubePoints(points, size, glm::vec3(-0.5f * size, 0.5f * size, 0.5f * size));
    }
    if (testOctree.rootNode.divisions.at(3).filled){
      addCubePoints(points, size, glm::vec3(0.5f * size, 0.5f * size, 0.5f * size));
    } 

    if (testOctree.rootNode.divisions.at(4).filled){
      addCubePoints(points, size, glm::vec3(-0.5f * size, -0.5f * size, -0.5f * size));
    }
    if (testOctree.rootNode.divisions.at(5).filled){
      addCubePoints(points, size, glm::vec3(0.5f * size, -0.5f * size, -0.5f * size));
    }
    if (testOctree.rootNode.divisions.at(6).filled){
      addCubePoints(points, size, glm::vec3(-0.5f * size, -0.5f * size, 0.5f * size));
    }
    if (testOctree.rootNode.divisions.at(7).filled){
      addCubePoints(points, size, glm::vec3(0.5f * size, -0.5f * size, 0.5f * size));
    } 

  }


  if (points.size() == 0){ // just hack for now 
    addCubePoints(points, 0.0001f, glm::vec3(0.f, 0.f, 0.f));
  }

  for (int i = 0; i < points.size(); i++){
    int triangleIndex = i % 3;
    if (triangleIndex == 0){
      vertices.push_back(createVertex2(points.at(i), glm::vec2(0.f, 0.f)));  // maybe the tex coords should just be calculated as a ratio to a fix texture
      continue;
    }else if (triangleIndex == 1){
      vertices.push_back(createVertex2(points.at(i), glm::vec2(1.f, 0.f)));
      continue;
    }else if (triangleIndex == 2){
      vertices.push_back(createVertex2(points.at(i), glm::vec2(0.f, 1.f)));
      continue;
    }
    modassert(false, "invalid triangleIndex");
  }
  std::vector<unsigned int> indices;
  for (int i = 0; i < vertices.size(); i++){ // should be able to optimize this better...
    indices.push_back(i);
  }
  MeshData meshData {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "./res/textures/grid.png",
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
    .boundInfo = getBounds(vertices),
    .bones = {},
  };
  return util.loadMesh(meshData);
}

Mesh* getOctreeMesh(GameObjectOctree& octree){
  return &octree.mesh;
}

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectOctree obj {};

  obj.mesh = createOctreeMesh(util);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
//  auto serializedData = serializeVoxelState(obj.voxel, util.textureName);
//
//  pairs.push_back(std::pair<std::string, std::string>("from", serializedData.voxelState));
//  if (serializedData.textureState != ""){
//    pairs.push_back(std::pair<std::string, std::string>("fromtextures", serializedData.textureState));
//  }
  return pairs;
}  //


