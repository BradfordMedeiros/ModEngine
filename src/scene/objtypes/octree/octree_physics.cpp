#include "./octree_physics.h"

int maxSubdivision(std::vector<PhysicsShapeData>& shapeData){
  int maxSize = 0;
  for (auto &shape : shapeData){
    if (shape.path.size() > maxSize){
      maxSize = shape.path.size();
    }
  }
  return maxSize;
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

std::string print(SparseShape& sparseShape){
  glm::vec3 minValues(sparseShape.minX, sparseShape.minY, sparseShape.minZ);
  glm::vec3 maxValues(sparseShape.maxX, sparseShape.maxY, sparseShape.maxZ);
  std::string value = print(minValues) + " | " + print(maxValues);
  return value;
}

ValueAndSubdivision indexForOctreePath(std::vector<int> path){
  glm::ivec3 sumIndex(0, 0, 0);
  int subdivisionLevel = 0;
  for (int index : path){
    sumIndex = sumIndex * 2;
    sumIndex += flatIndexToXYZ(index);;
    subdivisionLevel++;
  }
  return ValueAndSubdivision {
    .value = sumIndex,
    .subdivisionLevel = subdivisionLevel,
  };
}