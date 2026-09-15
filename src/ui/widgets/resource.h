#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_internal.h"
#include "../../cscript/cscript_binding.h"
#include "../../object_util.h"
#include "../../main_api.h"

struct ImGuiColor {
	int symbol;
	glm::vec4 color;
};
extern std::vector<ImGuiColor> imguiColors;
ImVec4 getImGuiColor(int symbol, glm::vec4 defaultColor);

struct ImGuiLoadedFont {
  ImFont* font;
  int symbol;
  float fontSize;
  std::string path;
};
extern std::vector<ImGuiLoadedFont> imguiFonts;
void loadImGuiFont(int symbol, std::string path, float fontSize);
ImGuiLoadedFont& getImGuiFontType(int symbol);
ImFont* getImGuiFont(int symbol);
void updateFont(int symbol, std::string path, float fontSize);

struct ImGuiFontBinding {
	int fontBinding;
	int fontSymbol;
};
extern std::vector<ImGuiFontBinding> imguiFontBindings;
ImFont* getFontByBinding(int symbol);

bool loadUiData(std::string filepath);
void saveUiData(std::string filepath);