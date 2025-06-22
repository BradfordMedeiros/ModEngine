#include "./octree_serialization.h"

extern std::optional<AtlasDimensions> atlasDimensions;

// Serialization
////////////////////////////////////////////////////////////////////////////////
/*
5
1 [0 0 1 1 [0 0 0 1 0 0 0 0] 0 0 1] 1 1 1 1 1 1]
*/

// This could easily be optimized by saving this in binary form instead
// human readable, at least for now, seems nice
std::string serializeOctreeDivision(OctreeDivision& octreeDivision, std::vector<FaceTexture>& textures, std::vector<OctreeShape*>& shapeData, std::vector<OctreeMaterial>& materials){
  if (octreeDivision.divisions.size() != 0){
    std::string str = "[ ";
    modassert(octreeDivision.divisions.size() == 8, "serialization - unexpected # of octree divisions");
    //modassert(octreeDivision.fill == FILL_MIXED, "octree divisions, but not mixed filled");
    for (int i = 0; i < octreeDivision.divisions.size(); i++){
      auto value = serializeOctreeDivision(octreeDivision.divisions.at(i), textures, shapeData, materials);
      str += value + " ";
    }
    str += "]";
    return str;
  }

  //modassert(octreeDivision.faces.size() == 6, "serializeOctreeDivision unexpected number of octree faces");
  for (auto &face : octreeDivision.faces){
    textures.push_back(face); 
  }
  shapeData.push_back(&octreeDivision.shape);
  materials.push_back(octreeDivision.material);

  return octreeDivision.fill == FILL_FULL ? "1" : "0";
}

std::string serializeTexCoord(FaceTexture& faceTexture){
  std::string value = "";
  value += std::to_string(faceTexture.textureIndex) + "|";
  value += serializeVec(faceTexture.texCoordsTopLeft) + "|";
  value += serializeVec(faceTexture.texCoordsTopRight) + "|";
  value += serializeVec(faceTexture.texCoordsBottomLeft) + "|";
  value += serializeVec(faceTexture.texCoordsBottomRight);
  return value;
}

std::string serializeOctree(Octree& octree){
  // 2
  // [ 1 1 1 1 [ 1 1 1 1 1 0 1 0 ] [ 1 1 1 1 0 1 0 1 ] [ 1 1 1 1 1 0 1 0 ] [ 1 1 1 1 0 1 0 1 ] ]
  // 0.3 0.3|0.4 0.2|0.3 0.4|0.1 0.1,2,1,2,1,2;1,2,3,1,1,1;1,1,1,1,1,1
  std::vector<FaceTexture> textures;
  std::vector<OctreeShape*> shapeData;
  std::vector<OctreeMaterial> materials;
  
  std::string str = std::to_string(1.f) + "\n";
  str += serializeOctreeDivision(octree.rootNode, textures, shapeData, materials) + "\n";

  std::string textureString = "";
  for (int i = 0; i < textures.size(); i+=6){
    textureString += serializeTexCoord(textures.at(i)) + ",";  // front
    textureString += serializeTexCoord(textures.at(i + 1)) + ",";  // back
    textureString += serializeTexCoord(textures.at(i + 2)) + ",";  // left
    textureString += serializeTexCoord(textures.at(i + 3)) + ",";  // right
    textureString += serializeTexCoord(textures.at(i + 4)) + ",";  // top
    textureString += serializeTexCoord(textures.at(i + 5));  // down

    if ((i + 5) != textures.size() -1){
      textureString += ";";
    }
  }

  std::cout << "num texture coords: " << textures.size() << std::endl;
  str += textureString + "\n";

  {
    std::string shapeString = "";
    for (int i = 0; i < shapeData.size(); i++){
      auto shape = shapeData.at(i);
      auto blockShapePtr = std::get_if<ShapeBlock>(shape);
      auto rampShapePtr = std::get_if<ShapeRamp>(shape);
      modassert(blockShapePtr || rampShapePtr, "invalid shape");
      if (blockShapePtr){
        shapeString += "b";
      }else if (rampShapePtr){
        shapeString += "r|" + std::to_string(rampShapePtr -> startHeight) + "|" + std::to_string(rampShapePtr -> endHeight) + "|" + std::to_string(rampShapePtr -> startDepth) + "|" + std::to_string(rampShapePtr -> endDepth) + "|";
        if (rampShapePtr -> direction == RAMP_FORWARD){
          shapeString += "f";
        }else if (rampShapePtr -> direction == RAMP_BACKWARD){
          shapeString += "b";
        }else if (rampShapePtr -> direction == RAMP_LEFT){
          shapeString += "l";
        }else if (rampShapePtr -> direction == RAMP_RIGHT){
          shapeString += "r";
        }else{
          modassert(false, "invalid shape type");
        }
      }else{
        modassert(false, "invalid shape type");
      }
      if (i != shapeData.size() - 1){
        shapeString += ";";
      }
    }
    str += shapeString + "\n";
  }

  {
    std::string materialString = "";
    for (int i = 0; i < materials.size(); i++){
      auto material = materials.at(i);
      if (material == OCTREE_MATERIAL_DEFAULT){
        materialString += "d";
      }else if (material == OCTREE_MATERIAL_WATER){
        materialString += "w";
      }else{
        modassert(false, "invalid material type");
      }
      if (i != materials.size() - 1){
        materialString += ";";
      }
    }
    str += materialString + "\n";
  }


  std::string textureNames = "";
  for (int i = 0; i < atlasDimensions.value().textureNames.size(); i++){
    textureNames += atlasDimensions.value().textureNames.at(i) + (i == (atlasDimensions.value().textureNames.size() - 1) ? "" : ",");
  }
  str += textureNames;


  return str;
}


// Deserialization
////////////////////////////////////////////////////////////////////////////////

std::vector<std::vector<FaceTexture>> deserializeTextures(std::string& values){
  std::vector<std::vector<FaceTexture>> textures;
  auto valuesStr = split(values, ';'); // denotes each octree division
  for (auto &value : valuesStr){
    auto faces = split(value, ',');
    modassert(faces.size() == 6, "invalid face size for textures");
    std::vector<FaceTexture> faceTextures;
    for (auto &face : faces){
      auto textureCoords = split(face, '|');

      modassert(textureCoords.size() == 5, "invalid texture coords size");
      faceTextures.push_back(FaceTexture {
        .textureIndex = std::atoi(textureCoords.at(0).c_str()),
        .texCoordsTopLeft = parseVec2(textureCoords.at(1)),
        .texCoordsTopRight = parseVec2(textureCoords.at(2)),
        .texCoordsBottomLeft = parseVec2(textureCoords.at(3)),
        .texCoordsBottomRight = parseVec2(textureCoords.at(4)),
      });
    }
    textures.push_back(faceTextures);
  }
  // , denotes each face
  // | denotes each texture 
  return textures;
}

std::vector<OctreeShape> deserializeShapes(std::string& values){
  std::cout << values << std::endl;
  auto valuesStr = split(values, ';');
  std::vector<OctreeShape> octreeShapes;
  for (auto &valueStr : valuesStr){
    if (valueStr.at(0) == 'd' || valueStr.at(0) == 'b'){
      octreeShapes.push_back(ShapeBlock{});
    }else if (valueStr.at(0) == 'r'){
      auto shapeData = split(valueStr, '|');
      auto startHeight = std::atof(shapeData.at(1).c_str());
      auto endHeight = std::atof(shapeData.at(2).c_str());
      auto startDepth = std::atof(shapeData.at(3).c_str());
      auto endDepth = std::atof(shapeData.at(4).c_str());
      auto direction = RAMP_FORWARD;
      if (shapeData.at(5) == "f"){
        // do nothing
      }else if (shapeData.at(5) == "b"){
        direction = RAMP_BACKWARD;
      }else if (shapeData.at(5) == "l"){
        direction = RAMP_LEFT;
      }else if (shapeData.at(5) == "r"){
        direction = RAMP_RIGHT;
      }else{
        modassert(false, "invalid shape type - bad defined ramp");
      }
      octreeShapes.push_back(ShapeRamp {
        .direction = direction,
        .startHeight = startHeight,
        .endHeight = endHeight,
        .startDepth = startDepth,
        .endDepth = endDepth,
      });
    }else{
      modassert(false, std::string("invalid shape type: ") + valueStr.at(0));
    }
  }
  return octreeShapes;
}


std::vector<OctreeMaterial> deserializeMaterials(std::string& values){
  auto valuesStr = split(values, ';');
  std::vector<OctreeMaterial> materials;
  for (auto &valueStr : valuesStr){
    if (valueStr.at(0) == 'd' || valueStr.at(0) == 'b'){
      materials.push_back(OCTREE_MATERIAL_DEFAULT);
    }else if (valueStr.at(0) == 'w'){
      materials.push_back(OCTREE_MATERIAL_WATER);
    }else{
      modassert(false, "invalid material type");
    }
  }
  return materials;
}


bool equalOrdered(std::vector<std::string>& mapTextures, std::vector<std::string>& atlasTextures){
  if (mapTextures.size() != atlasTextures.size()){
    return false;
  }
  for (int i = 0; i < mapTextures.size(); i++){
    if (mapTextures.at(i) != atlasTextures.at(i)){
      return false;
    }
  }
  return true;
}
std::unordered_map<int, int> getTextureMapping(std::vector<std::string>& mapTextures, std::vector<std::string>& atlasTextures, std::vector<std::vector<FaceTexture>>& faceTextures){
  std::unordered_map<int, int> mapToAtlas;
  for (int x = 0; x < mapTextures.size(); x++){
    for (int y = 0; y < atlasTextures.size(); y++){
      if (mapTextures.at(x) == atlasTextures.at(y)){
        mapToAtlas[x] = y;
        break;
      }
    }
  }
  return mapToAtlas;
}

std::vector<std::string> splitBrackets(std::string& value){
  int numBrackets = 0;
  std::vector<std::string> values;
  std::string currString = "";
  for (int i = 0; i < value.size(); i++){
    if (value.at(i) == '['){
      numBrackets++;
    }else if (value.at(i) == ']'){
      numBrackets--;
    }
 
    currString += value.at(i);
    if (numBrackets == 0){
      if (currString != " "){
        values.push_back(currString);
      }
      currString = "";
    }
  }
  if (currString.size() > 0){
    if (currString != " "){
      values.push_back(currString);
    }
  }
  modassert(numBrackets == 0, std::string("invalid balancing to brackets: ") + value + ", " + std::to_string(numBrackets));
  return values;
}

OctreeDivision deserializeOctreeDivision(std::string& value, std::vector<std::vector<FaceTexture>>& textures, int* currentTextureIndex, std::vector<OctreeShape>& octreeShapes, int* currentShapeIndex, std::vector<OctreeMaterial>& materials){
  value = trim(value);
  bool inBrackets = value.size() >= 2 && value.at(0) == '[' && value.at(value.size() -1) == ']';

  if (inBrackets){
    auto withoutBrackets = value.substr(1, value.size() - 2);
    auto splitValues = splitBrackets(withoutBrackets);
    std::vector<OctreeDivision> octreeDivisions;
    for (auto &splitValue : splitValues){
      modassert(splitValue.size() > 0, "split value should not be 0 length");
      octreeDivisions.push_back(deserializeOctreeDivision(splitValue, textures, currentTextureIndex, octreeShapes, currentShapeIndex, materials));
    }
    modassert(octreeDivisions.size() == 8, std::string("invalid division size, got: " + std::to_string(octreeDivisions.size())));
    return OctreeDivision {
      .fill = FILL_MIXED,
      .shape = ShapeBlock{},
      .divisions = octreeDivisions,
    };
  }
  modassert(value.size() >= 1 && (value.at(0) == '0' || value.at(0) == '1'), std::string("invalid value type, got: ") + value + ", size=  " + std::to_string(value.size()));
  auto filled = value.at(0) == '1';
  *currentTextureIndex = *currentTextureIndex + 1;
  *currentShapeIndex = *currentShapeIndex + 1;

  return OctreeDivision {
    .fill = filled ? FILL_FULL : FILL_EMPTY,
    .material = materials.at(*currentShapeIndex),
    .shape = octreeShapes.at(*currentShapeIndex),
    .faces = textures.at(*currentTextureIndex),
    .divisions = {},
  };
}


Octree deserializeOctree(std::string& value){
  auto lines = split(value, '\n');
  modassert(lines.size() == 6, std::string("invalid line size, got: ") + std::to_string(lines.size()));

  auto textures = deserializeTextures(lines.at(2));
  auto shapes = deserializeShapes(lines.at(3));
  modassert(shapes.size() == textures.size(), std::string("texture size and shapes sizes disagree: t, s = ") + std::to_string(textures.size()) + ", " + std::to_string(shapes.size()));

  auto materials = deserializeMaterials(lines.at(4));

  auto textureAtlas = split(lines.at(5), ',');
  //modassert(equalOrdered(textureAtlas, atlasDimensions.value().textureNames), "textures changed");
  auto textureMapping = getTextureMapping(textureAtlas, atlasDimensions.value().textureNames, textures);
  modassert(textureMapping.size() == textureAtlas.size(), std::string("does not have a mapping for all textures, textureMapping.size = ") + std::to_string(textureMapping.size()) + ", textureAtlas.size = " + std::to_string(textureAtlas.size()));
  for (auto &texture : textures){
    for (auto &textureFace : texture){
      textureFace.textureIndex = textureMapping.at(textureFace.textureIndex);
    }
  }

  int currentTextureIndex = -1;
  int currentShapeIndex = -1;
  return Octree  {
    .rootNode = deserializeOctreeDivision(lines.at(1), textures, &currentTextureIndex, shapes, &currentShapeIndex, materials),
  };
}
