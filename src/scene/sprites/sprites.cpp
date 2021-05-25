#include "./sprites.h"

extern World world;

std::map<unsigned int, Mesh> loadFontMeshes(font fontToLoad){
    std::map<unsigned int, Mesh> fontmeshes;
    for ( const auto &[ascii, font]: fontToLoad.chars ) {
      assert(fontmeshes.find(ascii) == fontmeshes.end());
      std::cout << "loaded font mesh: " << ascii << " (" << ((char)ascii) << ")" << std::endl;
      fontmeshes[ascii] = loadSpriteMeshSubimage(fontToLoad.image, font.x, font.y, font.width, font.height, loadTexture);
    }
    return fontmeshes;
}
void drawSprite(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height, glm::mat4 model){
  glm::mat4 modelMatrix = glm::scale(glm::translate(model, glm::vec3(left, top, 0)), glm::vec3(width * 1.0f, height * 1.0f, 1.0f)); 
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  drawMesh(mesh, shaderProgram, -1, -1, false);
}
void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height){
  drawSprite(shaderProgram, mesh, centerX - width/2, centerY - height/2, width, height, glm::mat4(1.f));
}

float calculateLeftAlign(float left, int numWords, bool center, float offsetDelta){
  // To center, move it back by half of the totals offsets.  If it's even, add an additional half an offset delta
  float leftAlign = !center ? left : (left  - ((numWords / 2) * offsetDelta) + ((numWords % 2) ? 0.f : (0.5f * offsetDelta)));
  return leftAlign;
}

void drawWordsRelative(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, glm::mat4 model, std::string word, float left, float top, unsigned int fontSize, bool center, float offsetDelta){
  float leftAlign = calculateLeftAlign(left, word.size(), center, offsetDelta);

  for (char& character : word){
    if (fontMeshes.find((int)(character)) != fontMeshes.end()){
      drawSprite(shaderProgram, fontMeshes.at((int)character), leftAlign, top, fontSize, fontSize, model);
    }
    leftAlign += offsetDelta;  // @todo this spacing is hardcoded for a fix set of font size.  This needs to be proportional to fontsize.
  }
}

void drawWords(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::string word, float left, float top, unsigned int fontSize){
  drawWordsRelative(shaderProgram, fontMeshes, glm::mat4(1.f), word, left, top, fontSize, false, 14);
}

BoundInfo boundInfoForCenteredText(std::string word, unsigned int fontSize, float offsetDelta){
  float leftAlign = calculateLeftAlign(0, word.size(), false, offsetDelta);
  float right = leftAlign + (offsetDelta * word.size());

  BoundInfo info {
    .xMin = 0, .xMax = right - leftAlign,
    .yMin = 0, .yMax = 2,
    .zMin = 0, .zMax = 0.1,
    .isNotCentered = false,
  };
  return info;
}