#ifndef MOD_SPRITES
#define MOD_SPRITES

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "./readfont.h"
#include "../common/mesh.h"

#include <ft2build.h>
#include FT_FREETYPE_H  

struct FontParams {
	Mesh mesh;
	float advance;
	glm::vec2 size;
	glm::vec2 bearing;
};

struct FontFamily {
	std::string name;
	float lineSpacing;
	std::map<unsigned int, FontParams> asciToMesh;
};

std::vector<FontFamily> loadFontMeshes(std::vector<FontToLoad> fontInfos, Texture& nullTexture);

void drawSpriteAround(GLint shaderProgram, Mesh mesh, float centerX, float centerY, float width, float height);
int drawWordsRelative(GLint shaderProgram, FontFamily& fontfamily, glm::mat4 model, std::string word, float left, float top, unsigned int fontSize, AlignType align, TextWrap wrap, TextVirtualization virtualization, int cursorIndex, bool cursorIndexLeft, int highlightLength, bool drawBoundingOnly);
void drawWords(GLint shaderProgram, FontFamily& fontfamily, std::string word, float left, float top, unsigned int fontSize, std::optional<float> maxWidth);
BoundInfo boundInfoForCenteredText(FontFamily& fontFamily, std::string word, float left, float top, unsigned int fontSize, AlignType align, TextWrap wrap, TextVirtualization virtualization, int cursorIndex, bool cursorIndexLeft, int highlightLength, glm::vec3* _offset);

#endif
