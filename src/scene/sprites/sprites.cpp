#include "./sprites.h"

struct FontParamInfo {
  std::map<unsigned int, FontParams> fontmeshes;
  float lineSpacing;
};

FontParamInfo loadModFontMeshes(font& fontToLoad){
  std::map<unsigned int, FontParams> fontmeshes;
  for (const auto &[ascii, font]: fontToLoad.chars) {
    assert(fontmeshes.find(ascii) == fontmeshes.end());
    std::cout << "loaded font mesh: " << ascii << " (" << ((char)ascii) << ")" << std::endl;
    // this is (-1 to 1, -1 to 1)
    fontmeshes[ascii] = FontParams {
      .mesh = loadSpriteMeshSubimage(fontToLoad.image, font.x, font.y, font.width, font.height, loadTexture, true),
      .advance = 1.f,
      .size = glm::vec2(1.f, 1.f),
      .bearing = glm::vec2(0.f, 1.f),
    };
  }
  return FontParamInfo{
    .fontmeshes = fontmeshes,
    .lineSpacing = 1.f,
  };
}


FT_Library freeTypeInstance;
FT_Library* initFreeType(){
  static bool freeTypeInited = false;
  if (!freeTypeInited){
    if (FT_Init_FreeType(&freeTypeInstance)){
        std::cout << "Error - Could not init freeType" << std::endl;
        assert(false);
    }
    freeTypeInited = true;
  }
  return &freeTypeInstance;
}

FontParamInfo loadTtfFontMeshes(std::string filepath, ttfFont& fontToLoad, Texture& nullTexture){
  std::map<unsigned int, FontParams> fontmeshes;
  FT_Library* freeType = initFreeType();
  FT_Face face;
  if (FT_New_Face(*freeType, filepath.c_str(), 0, &face)){
    modassert(false, "Error - FreeType - failed loading font");
  }
  FT_Set_Pixel_Sizes(face, 128, 128); // what size to set this?

  modassert(!FT_HAS_VERTICAL(face), "font rendering does not support vertical");

  float pixelToNDIScaling = 100.f; 
  for (int i = 0; i < 128; i++){
    auto error = FT_Load_Char(face, i, FT_LOAD_RENDER);
    modassert(!error, "ERROR - loadTtfFontMeshes - could not load char: " + std::to_string(i));


    unsigned int width = face -> glyph -> bitmap.width;
    unsigned int height = face -> glyph -> bitmap.rows;

    if (face -> glyph -> bitmap.width == 0){
      std::cout << "warning 0 size, character: " << i << std::endl;
    }

    float glyphAdvance = face -> glyph -> advance.x / 64;
    glm::vec2 glyphSize(face -> glyph -> bitmap.width, face -> glyph -> bitmap.rows); 
    glm::vec2 glyphBearing(face -> glyph -> bitmap_left, face -> glyph -> bitmap_top);
    //glm::vec2 glyphBearing(0.f, 0.f);
    std::cout << "FONT: loading: " << (char) i << " size = " << print(glyphSize) << ", bearing = " << print(glyphBearing) << ", advance = " << glyphAdvance <<  std::endl;

    // this should be smarter.  1 NDI text covers -1 to 1
    // since I'm doing all ndi, maybe I should normalize the values or something?

    fontmeshes[i] = FontParams {
      .mesh = loadSpriteMesh("./res/textures/wood.jpg", [&face, i, width, height, &nullTexture](std::string _) -> Texture {
        char value = (char)i;
        std::cout << "Loading character: (" << i << ") - " << value << std::endl;
        auto hasBuffer = face -> glyph -> bitmap.buffer != NULL;
        if (hasBuffer){
          return loadTextureDataRed(face -> glyph -> bitmap.buffer, width, height);
        }
        return nullTexture;
      }),
      .advance = glyphAdvance / pixelToNDIScaling,
      .size = glyphSize / pixelToNDIScaling,
      .bearing = glyphBearing / pixelToNDIScaling,
    };
  }

  int ymax = FT_MulFix(face -> bbox.yMax, face -> size -> metrics.y_scale) / 64;
  int ymin = FT_MulFix(face -> bbox.yMin, face -> size -> metrics.y_scale) / 64;
  int height = ymax - ymin;
  auto faceHeight = height / pixelToNDIScaling;

  FT_Done_Face(face);
  return FontParamInfo{
    .fontmeshes = fontmeshes,
    .lineSpacing = faceHeight * 0.8f, // 1.f is "correct" but seems too big on avg
  };;
}

FontParamInfo loadFontMesh(std::string filepath, fontType fontInfo, Texture& nullTexture){
  auto fontToLoadPtr = std::get_if<font>(&fontInfo);
  if (fontToLoadPtr != NULL){
    return loadModFontMeshes(*fontToLoadPtr);
  }
  auto ttfFontToLoadPtr = std::get_if<ttfFont>(&fontInfo);
  if (ttfFontToLoadPtr != NULL){
    return loadTtfFontMeshes(filepath, *ttfFontToLoadPtr, nullTexture);
  }
  modassert(fontToLoadPtr != NULL, "invalid font type - NULL");
  return FontParamInfo { .fontmeshes = {}, .lineSpacing = 1.f };
}

std::vector<FontFamily> loadFontMeshes(std::vector<FontToLoad> fontInfos, Texture& nullTexture){
  std::vector<FontFamily> fontParams;
  for (auto &fontInfo : fontInfos){
    auto loadedFont = loadFontMesh(fontInfo.name, fontInfo.type, nullTexture);
    fontParams.push_back(
      FontFamily {
        .name = fontInfo.name,
        .lineSpacing = loadedFont.lineSpacing,
        .asciToMesh = loadedFont.fontmeshes,
      }
    );
  }
  FT_Done_FreeType(*initFreeType());
  return fontParams;
}


void drawSpriteZBias(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height, glm::mat4 model, float zbias){
  auto scaleMatrix = glm::scale(glm::mat4(1.f), glm::vec3(width * 1.0f, height * 1.0f, 1.0f));
  auto translateMatrix = glm::translate(glm::mat4(1.f), glm::vec3(left, top, zbias));
  glm::mat4 modelMatrix = model * translateMatrix * scaleMatrix; 
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  drawMesh(mesh, shaderProgram, -1, -1, false);
}

/// these two functions are now equivalent since drawing things non-centered has not been useful
void drawSprite(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height, glm::mat4 model){
  drawSpriteZBias(shaderProgram, mesh, left, top, width, height, model, 0.f);
}
void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height){
  drawSprite(shaderProgram, mesh, centerX, centerY, width, height, glm::mat4(1.f));
}
///////////////////////////////////////////////////////////////////////////////////////

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

struct ImmediateDrawingInfo {
  Mesh* mesh;
  glm::vec2 pos;
  glm::vec2 size;
};

float calcLeftOffset(ImmediateDrawingInfo& info){
  return info.pos.x - info.size.x;
}
float calcRightOffset(ImmediateDrawingInfo& info){
  return info.pos.x + info.size.x;
}
float calcBottomOffset(ImmediateDrawingInfo& info){
  return info.pos.y - info.size.y;
}
float calcTopOffset(ImmediateDrawingInfo& info){
  return info.pos.y + info.size.y;
}

struct TextDrawingInfo {
  glm::vec2 size;
  glm::vec2 centerOffset;
};

TextDrawingInfo calcDrawInfo(std::vector<ImmediateDrawingInfo>& drawingInfo){
  if (drawingInfo.size() == 0){
    return TextDrawingInfo{ .size = glm::vec2(0.f, 0.f), .centerOffset = glm::vec2(0.f, 0.f) };
  }
  float maxLeft = calcLeftOffset(drawingInfo.at(0));
  float maxRight = calcRightOffset(drawingInfo.at(0));
  float maxBottom = calcBottomOffset(drawingInfo.at(0));
  float maxTop = calcTopOffset(drawingInfo.at(0));
  for (int i = 1; i < drawingInfo.size(); i++){
    ImmediateDrawingInfo& info = drawingInfo.at(i);
    float left = calcLeftOffset(info);
    float right = calcRightOffset(info);
    float top = calcTopOffset(info);
    float bottom = calcBottomOffset(info);
    if (left < maxLeft){
      maxLeft = left;
    }
    if (right > maxRight){
      maxRight = right;
    }
    if (bottom < maxBottom){
      maxBottom = bottom;
    }
    if (top > maxTop){
      maxTop = top;
    }
  }

  float width = maxRight - maxLeft;
  float height = maxTop - maxBottom;
  float halfWidth = 0.5f * width;
  float halfHeight = 0.5f * height;
  return TextDrawingInfo {
    .size = glm::vec2(width, height),
    .centerOffset = glm::vec2(-1 * maxLeft - halfWidth, -1 * maxBottom - halfHeight),
  };
}

glm::vec2 calcAlignOffset(std::vector<ImmediateDrawingInfo>& drawingInfo, AlignType align, float left, float top){
  auto drawingDimensions = calcDrawInfo(drawingInfo);
  auto offsetToCenter = drawingDimensions.centerOffset + glm::vec2(left, top);
  if (align == CENTER_ALIGN){
    std::cout << "center align" << std::endl;
    return offsetToCenter;
  }else if (align == POSITIVE_ALIGN){
    std::cout << "pos align" << std::endl;
    return offsetToCenter + glm::vec2(drawingDimensions.size.x * 0.5f, drawingDimensions.size.y * 0.5f);
  }else if (align == NEGATIVE_ALIGN){
    std::cout << "negative align" << std::endl;
    return offsetToCenter - glm::vec2(drawingDimensions.size.x * 0.5f, drawingDimensions.size.y * 0.5f);
  }
  modassert(false, "calc align offset invalid align type");
  return glm::vec2(0.f, 0.f);
}

int drawWordsRelative(GLint shaderProgram, FontFamily& fontFamily, glm::mat4 model, std::string word, float left, float top, unsigned int fontSize, float spacing, AlignType align, TextWrap wrap, TextVirtualization virtualization, int cursorIndex, bool cursorIndexLeft, int highlightLength){
  std::map<unsigned int, FontParams>& fontMeshes = fontFamily.asciToMesh;
  float fontSizeNdi = convertFontSizeToNdi(fontSize);
  float offsetDelta = 2.f * fontSizeNdi;
  //std::cout << "Fontsizendi: " << fontSizeNdi << std::endl;

  auto largestLineBreakSize = findLineBreakSize(word, wrap, virtualization);
  float originalleftAlign = 0.f;
  float leftAlign = originalleftAlign;
  int numTriangles = 0;

  int numCharactersOnLine = 0;
  int lineNumber = 0;
  
  int i = glm::max(0, virtualization.offset);
  
  int additionaCursorIndex = highlightLength + cursorIndex;

  //std::cout << "rendering word: (" << word << " - " << word.size() << ") " << std::endl;
  //std::cout << "letters: " << std::endl;

  std::vector<ImmediateDrawingInfo> cursors;
  std::vector<ImmediateDrawingInfo> drawingInfo;

  float lastCharacterAdvance = 0.f;
  glm::vec2 cursorSizing(0.3f, 1.2f);
  glm::vec2 cursorCenteringOffset(cursorSizing.x, cursorSizing.y);
  for (; i < word.size(); i++){
    char& character = word.at(i);
    //std::cout << "[" << (character == '\n' ? '@' : character) << "] ";
    if (character == 'q' || (wrap.type == WRAP_CHARACTERS && numCharactersOnLine >= wrap.wrapamount)) {
      leftAlign = originalleftAlign;
      numCharactersOnLine = 0;
      lineNumber++;
      //std::cout << "not drawing this, resetting left align to: " << leftAlign << std::endl;
    }
    if (character == 'q'){
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
    float topAlign = (lineNumber - virtualization.offsety) * -1 * offsetDelta * fontFamily.lineSpacing;

    float characterAdvance = 1.f;
    glm::vec2 characterSizing(1.f, 1.f);
    glm::vec2 characterBearing(0.f, 0.f);
    if (fontMeshes.find((int)(character)) != fontMeshes.end()){
      characterAdvance = fontMeshes.at((int)(character)).advance;
      characterSizing = fontMeshes.at((int)(character)).size;
      characterBearing = fontMeshes.at((int)(character)).bearing * offsetDelta;
      Mesh& fontMesh = fontMeshes.at((int)character).mesh;
      //std::cout << "char = " << character << ", " << "advance = " << characterAdvance << ", size = " << print(characterSizing) << ", bearing = " << print(characterBearing) << std::endl;
      //std::cout << "draw sprite: " << left << std::endl;
      /* add 1 not 0.5 since size 1 => 2 ndi */

      glm::vec2 centeringOffset(characterSizing.x, -1.f * characterSizing.y);
      ImmediateDrawingInfo info {
        .mesh = &fontMesh,
        .pos = glm::vec2(
          leftAlign + characterBearing.x + centeringOffset.x,
          top + topAlign + characterBearing.y + centeringOffset.y
        ),
        .size = glm::vec2(fontSizeNdi * characterSizing.x, fontSizeNdi * characterSizing.y),
      };
      drawingInfo.push_back(info);
    }else{
      std::cout << "missed character: " << (int)character << std::endl;
      modassert(false, "draw sprite font mesh not found");
    }

    //std::cout << "linespacing: " << fontFamily.lineSpacing << std::endl;
    //std::cout << "char sizing: " << print(characterSizing) << std::endl;
    //std::cout << std::endl;

    lastCharacterAdvance = offsetDelta * characterAdvance;
    
    if (cursorIndex == i || additionaCursorIndex == i){
      float additionalCursorOffset = cursorIndexLeft ? 0 : lastCharacterAdvance;
      ImmediateDrawingInfo cursor {
        .mesh = &fontMeshes.at('|').mesh,
        .pos = glm::vec2(
          leftAlign + additionalCursorOffset, 
          top + topAlign + cursorSizing.y  //+ cursorCenteringOffset.y + 2.f
        ),
        .size = glm::vec2(fontSizeNdi * cursorSizing.x, fontSizeNdi * cursorSizing.y), 
      };
      cursors.push_back(cursor);
    }
    //std::cout << "offset delta: " << offsetDelta << std::endl;
    leftAlign += lastCharacterAdvance;  // @todo this spacing is hardcoded for a fix set of font size.  This needs to be proportional to fontsize.
  }
  //std::cout << std::endl;
  if (cursorIndex == i || additionaCursorIndex == i){
      float topAlign = (lineNumber - virtualization.offsety) * -1 * offsetDelta * fontFamily.lineSpacing;
      float additionalCursorOffset = cursorIndexLeft ? 0 : lastCharacterAdvance;
      ImmediateDrawingInfo cursor {
        .mesh = &fontMeshes.at('|').mesh,
        .pos = glm::vec2(leftAlign  + additionalCursorOffset, top + topAlign + cursorSizing.y),
        .size = glm::vec2(fontSizeNdi * cursorSizing.x, fontSizeNdi * cursorSizing.y), 
      };
      cursors.push_back(cursor);
  }

  auto offsetToCenter = calcAlignOffset(drawingInfo, align, left, top);

  //auto offsetToCenter = drawingDimensions.centerOffset;
  for (auto &info : drawingInfo){
      //std::cout << "offset center: " << print(offsetToCenter) << std::endl;
      //std::cout << "info.pos.x = " << info.pos.x << ", info.size.x = " << info.size.x << std::endl;
      drawSprite(shaderProgram, *info.mesh, info.pos.x + offsetToCenter.x, info.pos.y + offsetToCenter.y, info.size.x, info.size.y, model);
      numTriangles += info.mesh -> numTriangles;
  }
  for (auto &cursor : cursors){
    drawSpriteZBias(shaderProgram, *cursor.mesh, cursor.pos.x + offsetToCenter.x, cursor.pos.y + offsetToCenter.y, cursor.size.x, cursor.size.y, model, -0.1f);
    numTriangles += cursor.mesh -> numTriangles; 
  }
  return numTriangles;
}

void drawWords(GLint shaderProgram, FontFamily& fontFamily, std::string word, float left, float top, unsigned int fontSize){
  drawWordsRelative(shaderProgram, fontFamily, glm::mat4(1.f), word, left, top, fontSize, 14, POSITIVE_ALIGN, TextWrap { .type = WRAP_NONE, .wrapamount = 0.f }, TextVirtualization { .maxheight = -1, .offsetx = 0, .offsety = 0 }, -1);
}

float calculateLeftAlign(float left, int numLetters, float offsetDelta, AlignType align){
  bool center = align == CENTER_ALIGN;
  // To center, move it back by half of the totals offsets.  If it's even, add an additional half an offset delta
  float leftAlign = !center ? left : ((left  - ((numLetters / 2) * offsetDelta) + ((numLetters % 2) ? 0.f : (0.5f * offsetDelta))));
  return leftAlign;
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