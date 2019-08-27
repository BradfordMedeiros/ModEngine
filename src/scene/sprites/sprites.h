#ifndef MOD_SPRITES
#define MOD_SPRITES

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "./readfont.h"
#include "../common/mesh.h"

std::map<unsigned int, Mesh> loadFontMeshes(font fontToLoad);
void drawSprite(GLint shaderProgram, Mesh mesh, float left, float top, float width, float height);
void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height);
void drawWords(GLint shaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::string word, float left, float top, unsigned int fontSize);

#endif
