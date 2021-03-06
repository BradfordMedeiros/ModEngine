#ifndef MOD_SPRITES
#define MOD_SPRITES

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "./readfont.h"
#include "../common/mesh.h"
#include "../scene.h"

std::map<unsigned int, Mesh> loadFontMeshes(font fontToLoad);
void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height);
void drawWordsRelative(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, glm::mat4 model, std::string word, float left, float top, unsigned int fontSize, bool center, float offsetDelta);
void drawWords(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::string word, float left, float top, unsigned int fontSize);
BoundInfo boundInfoForCenteredText(std::string word, unsigned int fontSize, float offsetDelta);

#endif
