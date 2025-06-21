#include "./octree_physics.h"

struct PhysicsShapeData {
  OctreeShape* shape;
  std::vector<int> path;
};

int maxSubdivision(std::vector<PhysicsShapeData>& shapeData){
  int maxSize = 0;
  for (auto &shape : shapeData){
    if (shape.path.size() > maxSize){
      maxSize = shape.path.size();
    }
  }
  return maxSize;
}

struct SparseShape {
  OctreeShape* shape;
  std::vector<int> path;
  bool deleted;
  int minX;
  int maxX;
  int minY;
  int maxY;
  int minZ;
  int maxZ;
};

std::string print(SparseShape& sparseShape){
  glm::vec3 minValues(sparseShape.minX, sparseShape.minY, sparseShape.minZ);
  glm::vec3 maxValues(sparseShape.maxX, sparseShape.maxY, sparseShape.maxZ);
  std::string value = print(minValues) + " | " + print(maxValues);
  return value;
}

std::vector<SparseShape> joinSparseShapes(std::vector<SparseShape>& shapes){

  // join along x

  std::vector<SparseShape> joinedShapes;

  for (int iterations = 0; iterations < 2; iterations++){
    for (int i = 0; i < shapes.size(); i++){
      for (int j = i + 1; j < shapes.size(); j++){
        SparseShape& shape1 = shapes.at(i);
        SparseShape& shape2 = shapes.at(j);
        if (shape1.deleted || shape2.deleted){
          continue;
        }
        // if shape2 can fit into shape1 in other y and z dimensions, and is right next ot it on the x
        // or vice versa
        if (
          (shape2.minY == shape1.minY && shape2.maxY == shape1.maxY &&  shape2.minZ == shape1.minZ && shape2.maxZ == shape1.maxZ) || 
          (shape1.minY == shape2.minY && shape1.maxY == shape2.maxY &&  shape1.minZ == shape2.minZ && shape1.maxZ == shape2.maxZ)
        ){
          std::cout << "found a candidate for joining: " << print(shape1) << ", " << print(shape2) << std::endl;
          // if the shape is just in the other...shouldn't ever happen
          if (shape2.minX >= shape1.minX && shape2.maxX <= shape1.maxX){
            shape2.deleted = true;
            modassert(false, "expected blocks being joined not to be completely within the other");
          }
          if (shape2.minX == shape1.maxX){
            shape2.deleted = true;
            shape1.maxX = shape2.maxX;
          }
          if (shape1.minX == shape2.maxX){
            shape1.deleted = true;
            shape2.maxX = shape1.maxX;
          }
        }
      }
    }

    // join over y
    for (int i = 0; i < shapes.size(); i++){
      for (int j = i + 1; j < shapes.size(); j++){
        SparseShape& shape1 = shapes.at(i);
        SparseShape& shape2 = shapes.at(j);
        if (shape1.deleted || shape2.deleted){
          continue;
        }
        // if shape2 can fit into shape1 in other y and z dimensions, and is right next ot it on the x
        // or vice versa
        if (
          (shape2.minX == shape1.minX && shape2.maxX == shape1.maxX &&  shape2.minZ == shape1.minZ && shape2.maxZ == shape1.maxZ) || 
          (shape1.minX == shape2.minX && shape1.maxX == shape2.maxX &&  shape1.minZ == shape2.minZ && shape1.maxZ == shape2.maxZ)
        ){
          std::cout << "found a candidate for joining: " << print(shape1) << ", " << print(shape2) << std::endl;
          // if the shape is just in the other...shouldn't ever happen
          if (shape2.minY >= shape1.minY && shape2.maxY <= shape1.maxY){
            shape2.deleted = true;
            modassert(false, "expected blocks being joined not to be completely within the other");
          }
          if (shape2.minY == shape1.maxY){
            shape2.deleted = true;
            shape1.maxY = shape2.maxY;
          }
          if (shape1.minY == shape2.maxY){
            shape1.deleted = true;
            shape2.maxY = shape1.maxY;
          }
        }
      }
    }

    // join over z
    for (int i = 0; i < shapes.size(); i++){
      for (int j = i + 1; j < shapes.size(); j++){
        SparseShape& shape1 = shapes.at(i);
        SparseShape& shape2 = shapes.at(j);
        if (shape1.deleted || shape2.deleted){
          continue;
        }
        // if shape2 can fit into shape1 in other y and z dimensions, and is right next ot it on the x
        // or vice versa
        if (
          (shape2.minX == shape1.minX && shape2.maxX == shape1.maxX &&  shape2.minY == shape1.minY && shape2.maxY == shape1.maxY) || 
          (shape1.minX == shape2.minX && shape1.maxX == shape2.maxX &&  shape1.minY == shape2.minY && shape1.maxY == shape2.maxY)
        ){
          std::cout << "found a candidate for joining: " << print(shape1) << ", " << print(shape2) << std::endl;
          // if the shape is just in the other...shouldn't ever happen
          if (shape2.minZ >= shape1.minZ && shape2.maxZ <= shape1.maxZ){
            shape2.deleted = true;
            modassert(false, "expected blocks being joined not to be completely within the other");
          }
          if (shape2.minZ == shape1.maxZ){
            shape2.deleted = true;
            shape1.maxZ = shape2.maxZ;
          }
          if (shape1.minZ == shape2.maxZ){
            shape1.deleted = true;
            shape2.maxZ = shape1.maxZ;
          }
        }
      }
    }
  }


  for (auto &shape : shapes){
    if (!shape.deleted){
      joinedShapes.push_back(shape);
    }
  }


  return joinedShapes;
}


void addAllDivisions(std::vector<PhysicsShapeData>& shapeBlocks, OctreeDivision& octreeDivision, std::vector<int> path){
  if (octreeDivision.fill == FILL_FULL){
    shapeBlocks.push_back(PhysicsShapeData {
      .shape = &octreeDivision.shape,
      .path = path,
    });
  }else if (octreeDivision.fill == FILL_MIXED){
    modassert(octreeDivision.divisions.size() == 8, "expected 8 octree division addAllDivisions");
    for (int i = 0; i < octreeDivision.divisions.size(); i++){ 
      std::vector<int> newPath = path;
      newPath.push_back(i);
      addAllDivisions(shapeBlocks, octreeDivision.divisions.at(i), newPath);
    }
  }
}

SparseShape blockToSparseSubdivision(OctreeShape* shape, std::vector<int>& path, int subdivisionSize){
  modassert(path.size() <= subdivisionSize, "expected path.size() <= subdivisionSize");
  auto offsetValue = indexForOctreePath(path);
  modassert(offsetValue.subdivisionLevel == path.size(), "should be equal size");
  auto newOffset = indexForSubdivision(offsetValue.value.x, offsetValue.value.y, offsetValue.value.z, offsetValue.subdivisionLevel, subdivisionSize);
  auto size = glm::pow(2, subdivisionSize - path.size());  
  
  std::cout << "block to sparse subdivision: size = " << size << ", path = " << print(path) << ", max = " << subdivisionSize << ", maxvalue: " << glm::pow(2, subdivisionSize) << std::endl;
  SparseShape sparseShape {
    .shape = shape,
    .path = path,
    .deleted = false,
    .minX = newOffset.x,
    .maxX = newOffset.x + size,
    .minY = newOffset.y,
    .maxY = newOffset.y + size,
    .minZ = newOffset.z,
    .maxZ = newOffset.z + size,
  };
  std::cout << "sparse shape: " << print(sparseShape) << std::endl;

  return sparseShape;
}


glm::vec3 calculatePosition(std::vector<int>& path){
  glm::vec3 offset(0.f, 0.f, 0.f);
  for (int i = 0; i < path.size(); i++){
    offset = offsetForFlatIndex(path.at(i), glm::pow(0.5f, i + 1), offset);
  }
  return offset;
}

struct FinalShapeData {
  OctreeShape* shape;
  glm::vec3 position;
  glm::vec3 shapeSize;
};

std::vector<FinalShapeData> optimizePhysicsShapeData(std::vector<PhysicsShapeData>& shapeData){
  std::vector<FinalShapeData> optimizedShapes;

  auto subdivisionSize = maxSubdivision(shapeData);
  std::vector<SparseShape> sparseShapes;
  for (auto &shape : shapeData){
    ShapeBlock* blockShape = std::get_if<ShapeBlock>(shape.shape);
    ShapeRamp* rampShape = std::get_if<ShapeRamp>(shape.shape);
    modassert(blockShape || rampShape, "invalid shape type");
    if (blockShape){
      auto sparseShape = blockToSparseSubdivision(shape.shape, shape.path, subdivisionSize);
      sparseShapes.push_back(sparseShape);
    }else if (rampShape){
      auto subdivisionLevels = glm::ivec3(shape.path.size());
      float subdivisionSizeX = glm::pow(0.5f, subdivisionLevels.x);
      float subdivisionSizeY = glm::pow(0.5f, subdivisionLevels.y);
      float subdivisionSizeZ = glm::pow(0.5f, subdivisionLevels.z);
      glm::vec3 shapeSize(subdivisionSizeX, subdivisionSizeY, subdivisionSizeZ);

      optimizedShapes.push_back(FinalShapeData {
        .shape = shape.shape,
        .position = calculatePosition(shape.path),
        .shapeSize = shapeSize,
      });
    }
  }

  auto combinedSparseShapes = joinSparseShapes(sparseShapes);
  for (auto &sparseShape : combinedSparseShapes){
    auto subdivisionLevels = glm::ivec3(sparseShape.path.size(), sparseShape.path.size(), sparseShape.path.size());
    float subdivisionSizeX = glm::pow(0.5f, subdivisionLevels.x);
    float subdivisionSizeY = glm::pow(0.5f, subdivisionLevels.y);
    float subdivisionSizeZ = glm::pow(0.5f, subdivisionLevels.z);

    // max - minx is in terms of the smaller subdivisions, but the shapeSize is absolute to 1, so this corrects it
    float multiplierX = (sparseShape.maxX - sparseShape.minX) / glm::pow(2, subdivisionSize - sparseShape.path.size());
    float multiplierY = (sparseShape.maxY - sparseShape.minY) / glm::pow(2, subdivisionSize - sparseShape.path.size());
    float multiplierZ = (sparseShape.maxZ - sparseShape.minZ) / glm::pow(2, subdivisionSize - sparseShape.path.size());

    glm::vec3 adjustedSize(multiplierX * subdivisionSizeX, multiplierY * subdivisionSizeY, multiplierZ * subdivisionSizeZ);
    //std::cout << "span x: " << spanX << ", maxsub = " << subdivisionSize << ", current size = " << sparseShape.path.size() << ", size = " << print(shapeSize) << ", adjusted = " << print(adjustedSize) << std::endl;

    optimizedShapes.push_back(FinalShapeData {
      .shape = sparseShape.shape,
      .position = calculatePosition(sparseShape.path),
      .shapeSize = adjustedSize,
    });
  }

  return optimizedShapes;
}

void createShapeData(std::vector<FinalShapeData>& shapeData, std::vector<PositionAndScale>& _octreeCubes, std::vector<Transformation>& _rampBlocks){
  for (auto &shape : shapeData){
    ShapeBlock* blockShape = std::get_if<ShapeBlock>(shape.shape);
    ShapeRamp* rampShape = std::get_if<ShapeRamp>(shape.shape);
    modassert(blockShape || rampShape, "shape type not supported");
    if (blockShape){
      _octreeCubes.push_back(PositionAndScale {
        .position = shape.position, 
        .size = glm::vec3(shape.shapeSize.x, shape.shapeSize.y, shape.shapeSize.z),
      });
    }else if (rampShape){
      auto heightMultiplier = rampShape -> endHeight - rampShape -> startHeight;
      float depth = rampShape -> endDepth - rampShape -> startDepth;
      float ySize = shape.shapeSize.y * heightMultiplier;
      auto rampPosition = shape.position + glm::vec3(0.f, shape.shapeSize.y * rampShape -> startHeight, 0.f);

      if (rampShape -> direction == RAMP_FORWARD){
        auto extraOffset = glm::vec3(0.f, 0.f, -1 * rampShape -> startDepth * shape.shapeSize.z);
        _rampBlocks.push_back(Transformation {
          .position = rampPosition + extraOffset,
          .scale = glm::vec3(shape.shapeSize.x, ySize, shape.shapeSize.z * depth),
          .rotation = MOD_ORIENTATION_FORWARD,
        });
      }else if (rampShape -> direction == RAMP_BACKWARD){
        auto extraOffset = glm::vec3(0.f, 0.f, -1 * (1.f - rampShape -> endDepth) * shape.shapeSize.z);
        _rampBlocks.push_back(Transformation {
          .position = rampPosition + extraOffset,
          .scale = glm::vec3(shape.shapeSize.x, ySize, shape.shapeSize.z * depth),
          .rotation = MOD_ORIENTATION_BACKWARD,
        });
      }else if (rampShape -> direction == RAMP_LEFT){
        auto extraOffset = glm::vec3(rampShape -> startDepth * shape.shapeSize.z, 0.f, 0.f);
        _rampBlocks.push_back(Transformation {
          .position = rampPosition + extraOffset,
          .scale = glm::vec3(shape.shapeSize.x, ySize, shape.shapeSize.z * depth),
          .rotation = MOD_ORIENTATION_RIGHT,
        });
      }else if (rampShape -> direction == RAMP_RIGHT){
        auto extraOffset = glm::vec3((1.f - rampShape -> endDepth) * shape.shapeSize.z, 0.f, 0.f);
        _rampBlocks.push_back(Transformation {
          .position = rampPosition + extraOffset,
          .scale = glm::vec3(shape.shapeSize.x, ySize, shape.shapeSize.z * depth),
          .rotation = MOD_ORIENTATION_LEFT,
        });
      }
    }
  }
}

PhysicsShapes getPhysicsShapes(Octree& octree){
  PhysicsShapes empty{};
  return empty;
  
  std::vector<PositionAndScale> octreeCubes;


  // ENUMERATE OUT THE REST OF THE VERTS FOR A RAMP
  std::vector<PositionAndScaleVerts> shapes = {
    PositionAndScaleVerts {
      .verts = {
        // left 
        glm::vec3(0.f, 0.5f, -0.5f),
        glm::vec3(0.f, 0.f, -0.5f),
        glm::vec3(0.f, 0.f, 0.f),

        // right 
        glm::vec3(0.5f, 0.5f, -0.5f),
        glm::vec3(0.5f, 0.f, -0.5f),
        glm::vec3(0.5f, 0.f, 0.f),

        // bottom 
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, -0.5f),
        glm::vec3(0.5f, 0.f, 0.f),
        glm::vec3(0.5f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, -0.5f),
        glm::vec3(0.5f, 0.f, -0.5f),

        // back
        glm::vec3(0.f, 0.f, -0.5f),
        glm::vec3(0.f, 0.5f, -0.5f),
        glm::vec3(0.5f, 0.5f, -0.5f),

        glm::vec3(0.5f, 0.5f, -0.5f),
        glm::vec3(0.f, 0.f, -0.5f),
        glm::vec3(0.5f, 0.f, -0.5f),

        // main ramp face
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 0.5f, -0.5f),
        glm::vec3(0.5f, 0.f, 0.f),

        glm::vec3(0.5f, 0.f, 0.f),
        glm::vec3(0.f, 0.5f, -0.5f),
        glm::vec3(0.5f, 0.5f, -0.5f),
      },
      .centeringOffset = glm::vec3(-0.25, -0.25f, 0.25f),
      .specialBlocks = {},
    }
  };

  std::vector<PhysicsShapeData> shapeDatas;
  addAllDivisions(shapeDatas, octree.rootNode, {});
  PhysicsShapes physicsShapes {};

  auto optimizedShapeData = optimizePhysicsShapeData(shapeDatas);
  createShapeData(optimizedShapeData, octreeCubes, shapes.at(0).specialBlocks /* ramps */);

  physicsShapes.blocks = octreeCubes;
  physicsShapes.shapes = shapes;

  int numShapes = 0;
  for (auto &shape : shapes){
    numShapes += shape.specialBlocks.size();
  }

  std::cout << "getPhysicsShapes, shape datas size = " << shapeDatas.size() << std::endl;
  std::cout << "getPhysicsShapes  num shapes: " << numShapes << ", num blocks " << physicsShapes.blocks.size() << std::endl;
  //modassert((physicsShapes.blocks.size() + numShapes) == shapeDatas.size(), "shape datas should be equal to blocks and numShapes");

  return physicsShapes;
}