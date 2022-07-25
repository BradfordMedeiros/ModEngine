#include "./sprites.h"

std::map<unsigned int, Mesh> loadFontMeshes(font fontToLoad){
    std::map<unsigned int, Mesh> fontmeshes;
    for ( const auto &[ascii, font]: fontToLoad.chars ) {
      assert(fontmeshes.find(ascii) == fontmeshes.end());
      std::cout << "loaded font mesh: " << ascii << " (" << ((char)ascii) << ")" << std::endl;
      // this is (-1 to 1, -1 to 1)
      fontmeshes[ascii] = loadSpriteMeshSubimage(fontToLoad.image, font.x, font.y, font.width, font.height, loadTexture, true);
    }
    return fontmeshes;
}

void drawSpriteZBias(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height, glm::mat4 model, float zbias){
  auto translateMatrix = glm::translate(glm::mat4(1.f), glm::vec3(left, top, zbias));
  auto scaleMatrix = glm::scale(glm::mat4(1.f), glm::vec3(width * 1.0f, height * 1.0f, 1.0f));
  glm::mat4 modelMatrix = model * translateMatrix * scaleMatrix; 
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  drawMesh(mesh, shaderProgram, -1, -1, false);
}
void drawSprite(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height, glm::mat4 model){
  drawSpriteZBias(shaderProgram, mesh, left, top, width, height, model, 0.f);
}

void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height){
  drawSprite(shaderProgram, mesh, centerX - width/2, centerY - height/2, width, height, glm::mat4(1.f));
}

float calculateLeftAlign(float left, int numLetters, float offsetDelta, AlignType align){
  bool center = align == CENTER_ALIGN;
  // To center, move it back by half of the totals offsets.  If it's even, add an additional half an offset delta
  float leftAlign = !center ? left : ((left  - ((numLetters / 2) * offsetDelta) + ((numLetters % 2) ? 0.f : (0.5f * offsetDelta))));
  return leftAlign;
}

int findLineBreakSize(std::string& word, TextWrap wrap, TextVirtualization virtualization){
  int biggestSize = 0;
  int currentSize = 0;
  for (int i = glm::max(0, virtualization.offset); i < word.size(); i++){
    if (word.at(i) == '\n' && wrap.type != WRAP_NONE){
      if (currentSize > biggestSize){
        biggestSize = currentSize;
      }
      currentSize = 0;
    }
    currentSize++;
    if (wrap.type == WRAP_CHARACTERS && currentSize >= wrap.wrapamount){
      if (currentSize > biggestSize){
        biggestSize = currentSize;
      }
      currentSize = 0;
    }
  }
  if (currentSize > biggestSize){
    biggestSize = currentSize;
  }
  return biggestSize;
}


// So -1 to 1 covers the whole range for both x, y
// Apparently 1pt font is 1.333 pixels
// For now, making fontsize means % of screen, where 1pt = 0.1% of screen 
float convertFontSizeToNdi(float fontsize){
  return fontsize / 1000.f;
}

int drawWordsRelative(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, glm::mat4 model, std::string word, float left, float top, unsigned int fontSize, float spacing, AlignType align, TextWrap wrap, TextVirtualization virtualization, int cursorIndex, bool cursorIndexLeft, int highlightLength){
  float fontSizeNdi = convertFontSizeToNdi(fontSize);
  float offsetDelta = 2.f * fontSizeNdi;
  //std::cout << "Fontsizendi: " << fontSizeNdi << std::endl;

  auto largestLineBreakSize = findLineBreakSize(word, wrap, virtualization);
  float originalleftAlign = calculateLeftAlign(left, largestLineBreakSize, offsetDelta, align);
  float leftAlign = originalleftAlign;
  int numTriangles = 0;

  int numCharactersOnLine = 0;
  int lineNumber = 0;
  
  int i = glm::max(0, virtualization.offset);
  float additionalCursorOffset = cursorIndexLeft ? 0 : offsetDelta;

  int additionaCursorIndex = highlightLength + cursorIndex;

  //std::cout << "letters: " << std::endl;
  for (; i < word.size(); i++){
    char& character = word.at(i);
    if (character == '\n' || (wrap.type == WRAP_CHARACTERS && numCharactersOnLine >= wrap.wrapamount)) {
      leftAlign = originalleftAlign;
      numCharactersOnLine = 0;
      lineNumber++;
    }
    if (wrap.type != WRAP_NONE && character == '\n'){
      continue;
    }

    numCharactersOnLine++;
    if (numCharactersOnLine <= virtualization.offsetx){
      continue;
    }
    if (lineNumber < virtualization.offsety){
      continue;
    }
    if (virtualization.maxheight >= 0 && (lineNumber - virtualization.offsety >= virtualization.maxheight)){
      break;
    }
    float topAlign = (lineNumber - virtualization.offsety) * -1 * offsetDelta;

    if (fontMeshes.find((int)(character)) != fontMeshes.end()){
      Mesh& fontMesh = fontMeshes.at((int)character);
      //Mesh& fontMesh = fontMeshes.at('5');
      //drawSprite(shaderProgram, fontMesh, 0, 0, fontSize, fontSize, model);
      //std::cout << character << " fontsize_ndi = " << fontSizeNdi << " left = " << left << " left_align = " << leftAlign << " top = " << top <<  " top_align = " << topAlign << std::endl;
      drawSprite(shaderProgram, fontMesh, leftAlign, top + topAlign, fontSizeNdi, fontSizeNdi, model);
      //drawSprite(shaderProgram, fontMesh, 0.75, 0, fontSizeNdi, fontSizeNdi, model);
      //drawSprite(shaderProgram, fontMesh, leftAlign, top + topAlign, fontSizeNdi, fontSizeNdi, model);
      //drawSprite(shaderProgram, fontMesh, 0, 0, fontSize, fontSize, model);
      //drawSprite(shaderProgram, fontMesh, leftAlign, top + topAlign, 1.f, 1.f, model);
      numTriangles += fontMesh.numTriangles;
    }
    

    if (cursorIndex == i || additionaCursorIndex == i){
      Mesh& fontMesh = fontMeshes.at('|');
      drawSpriteZBias(shaderProgram, fontMesh, leftAlign - offsetDelta * 0.5f + additionalCursorOffset, top + topAlign, fontSize * 0.3f, fontSize * 2.f, model, -0.1f);
      numTriangles += fontMesh.numTriangles;      
    }
    leftAlign += offsetDelta;  // @todo this spacing is hardcoded for a fix set of font size.  This needs to be proportional to fontsize.
  }
  //std::cout << std::endl;

  if (cursorIndex == i || additionaCursorIndex == i){
      float topAlign = (lineNumber - virtualization.offsety) * -1 * offsetDelta;
      Mesh& fontMesh = fontMeshes.at('|');
      drawSpriteZBias(shaderProgram, fontMesh, leftAlign - offsetDelta * 0.5f + additionalCursorOffset, top + topAlign, fontSize * 0.3f, fontSize * 2.f, model, -0.1f);
      numTriangles += fontMesh.numTriangles;      
  }

  return numTriangles;
}

void drawWords(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::string word, float left, float top, unsigned int fontSize){
  drawWordsRelative(shaderProgram, fontMeshes, glm::mat4(1.f), word, left, top, fontSize, 14, NEGATIVE_ALIGN, TextWrap { .type = WRAP_NONE, .wrapamount = 0.f }, TextVirtualization { .maxheight = -1, .offsetx = 0, .offsety = 0 }, -1);
}

BoundInfo boundInfoForCenteredText(std::string word, unsigned int fontSize, float spacing, AlignType align, TextWrap wrap, TextVirtualization virtualization, glm::vec3 *_offset){
  //assert(type == CENTER_ALIGN);
  float offsetDelta = 2.f;
  auto largestLineBreakSize = findLineBreakSize(word, wrap, virtualization);
  float leftAlign = calculateLeftAlign(0, largestLineBreakSize, offsetDelta, align);
  float right = leftAlign + (offsetDelta * largestLineBreakSize);

  /*std::cout << "calculate bound info text" << std::endl;
  std::cout << "font size: " << fontSize << std::endl;
  std::cout << "offset delta: " << offsetDelta << std::endl;
  std::cout << "left align: " << leftAlign << std::endl;
  std::cout << "right: " << right << std::endl;*/

  auto halfWidth = (right - leftAlign) / 2.f;

  *_offset = glm::vec3(halfWidth - 0.5f * offsetDelta, 0.f, 0.f);

  BoundInfo info {
    .xMin = -1 * halfWidth, .xMax = halfWidth,
    .yMin = -1, .yMax = 1,
    .zMin = 0, .zMax = 0.1,
  };
  return info;
}