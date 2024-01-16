#include "./obj_octree.h"

std::vector<AutoSerialize> octreeAutoserializer {
 
};

struct OctreeDivision {
  // topleft, topright, bottomleft, bottomright
  std::vector<OctreeDivision> divisions; 
};

struct Octree {
  double size;
  OctreeDivision rootNode;
};


Octree testOctree {
  .size = 5.f,
  .rootNode = OctreeDivision {
    .divisions = {
      OctreeDivision {},
      OctreeDivision {
        .divisions = {
          OctreeDivision {},
          OctreeDivision {},
          OctreeDivision {},
          OctreeDivision {},
        }
      },
      OctreeDivision {},
      OctreeDivision {},
    },
  },
};

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

void addCubePoints(std::vector<glm::vec3>& points){
  //bottom plane
  points.push_back(glm::vec3(0.f, 0.f, -1.f));
  points.push_back(glm::vec3(1.f, 0.f, 0.f));
  points.push_back(glm::vec3(0.f, 0.f, 0.f));

  points.push_back(glm::vec3(1.f, 0.f, -1.f));
  points.push_back(glm::vec3(1.f, 0.f, 0.f));
  points.push_back(glm::vec3(0.f, 0.f, -1.f));

  // top plane
  points.push_back(glm::vec3(0.f, 1.f, 0.f));
  points.push_back(glm::vec3(1.f, 1.f, 0.f));
  points.push_back(glm::vec3(0.f, 1.f, -1.f));

  points.push_back(glm::vec3(0.f, 1.f, -1.f));
  points.push_back(glm::vec3(1.f, 1.f, 0.f));
  points.push_back(glm::vec3(1.f, 1.f, -1.f));

  // left plane
  points.push_back(glm::vec3(0.f, 1.f, -1.f));
  points.push_back(glm::vec3(0.f, 0.f, -1.f));
  points.push_back(glm::vec3(0.f, 0.f, 0.f));

  points.push_back(glm::vec3(0.f, 1.f, 0.f));
  points.push_back(glm::vec3(0.f, 1.f, -1.f));
  points.push_back(glm::vec3(0.f, 0.f, 0.f));

  // right plane
  points.push_back(glm::vec3(1.f, 0.f, 0.f));
  points.push_back(glm::vec3(1.f, 0.f, -1.f));
  points.push_back(glm::vec3(1.f, 1.f, -1.f));

  points.push_back(glm::vec3(1.f, 0.f, 0.f));
  points.push_back(glm::vec3(1.f, 1.f, -1.f));
  points.push_back(glm::vec3(1.f, 1.f, 0.f));


  // front plane
  points.push_back(glm::vec3(1.f, 0.f, 0.f));
  points.push_back(glm::vec3(0.f, 1.f, 0.f));
  points.push_back(glm::vec3(0.f, 0.f, 0.f));

  points.push_back(glm::vec3(1.f, 1.f, 0.f));
  points.push_back(glm::vec3(0.f, 1.f, 0.f));
  points.push_back(glm::vec3(1.f, 0.f, 0.f));


  // back plane
  points.push_back(glm::vec3(0.f, 0.f, -1.f));
  points.push_back(glm::vec3(0.f, 1.f, -1.f));
  points.push_back(glm::vec3(1.f, 0.f, -1.f));

  points.push_back(glm::vec3(1.f, 0.f, -1.f));
  points.push_back(glm::vec3(0.f, 1.f, -1.f));
  points.push_back(glm::vec3(1.f, 1.f, -1.f));
}

Mesh createOctreeMesh(ObjectTypeUtil& util){
  std::vector<Vertex> vertices;

  std::vector<glm::vec3> points = {};
  for (int i = 0; i < testOctree.rootNode.divisions.size(); i++){
    addCubePoints(points);
    break;
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


