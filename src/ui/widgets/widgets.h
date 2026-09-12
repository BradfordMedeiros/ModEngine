#ifndef MOD_GUI_WIDGETS
#define MOD_GUI_WIDGETS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "../../cscript/cscript_binding.h"
#include "../../object_util.h"
#include "../../main_api.h"


void renderRenderPanel(bool includePanel);
void renderTransformPanel(bool includePanel);
void renderTextures(bool includePanel, std::optional<objid> objectToDetail);
void renderDisplayBinding(bool includePanel);
void renderFontWidget(bool includePanel);
void renderColorWidget(bool includePanel);

struct ImGuiLoadedFont {
  ImFont* font;
  int symbol;
  float fontSize;
  std::string path;
};
extern std::vector<ImGuiLoadedFont> imguiFonts;
void loadImGuiFont(int symbol, std::string path, float fontSize);
std::optional<ImFont*> getImGuiFont(int symbol);
void updateFont(int symbol, std::string path, float fontSize);


struct ImGuiColor {
	int symbol;
	glm::vec4 color;
};
extern std::vector<ImGuiColor> imguiColors;
ImVec4 getImGuiColor(int symbol, glm::vec4 defaultColor);

#endif