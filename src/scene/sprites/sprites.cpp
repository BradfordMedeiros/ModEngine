#include "./sprites.h"

std::map<unsigned int, Mesh> loadFontMeshes(font fontToLoad){
    std::map<unsigned int, Mesh> fontmeshes;
    for ( const auto &[ascii, font]: fontToLoad.chars ) {
        fontmeshes[ascii] = load2DMeshTexCoords(fontToLoad.image, font.x, font.y, font.width, font.height);
    }
    return fontmeshes;
}
void drawSprite(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height){
  glm::mat4 modelMatrix = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(left, top, 0)), glm::vec3(width * 1.0f, height * 1.0f, 1.0f)); 
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
  drawMesh(mesh);
}
void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height){
  drawSprite(shaderProgram, mesh, centerX - width/2, centerY - height/2, width, height);
}
void drawWords(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::string word, float left, float top, unsigned int fontSize){
  float leftAlign = left;

  for (char& character : word){
    drawSprite(shaderProgram, fontMeshes[(int)character], left + leftAlign, top, fontSize, fontSize);
    leftAlign += 14;  // @todo this spacing is hardcoded for a fix set of font size.  This needs to be proportional to fontsize.
  }
}
